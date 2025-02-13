#include "../include/RoutineControl.h"
#include "../../diagnostic_session_control/include/DiagnosticSessionControl.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/TestUtils.h"
#include "../../utils/include/FileManager.h"
#include "../../utils/include/NegativeResponse.h"
#include "../../authentication/include/SecurityAccess.h"

#include <cstring>
#include <string>
#include <thread>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <gtest/gtest.h>
#include <net/if.h>
#include "Globals.h"

int socket_;
int socket2_;
std::vector<uint8_t> seed;
std::string file_path = std::string(PROJECT_PATH) + "/backend/mcu/mcu_data.txt";

/* Create object for all tests */
struct RoutineControlTest : testing::Test
{
    RoutineControl* routine_control;
    SecurityAccess* r;
    DiagnosticSessionControl* dsc;
    CaptureFrame* capture_frame;
    Logger logger;
    RoutineControlTest()
    {
        routine_control = new RoutineControl(socket2_,logger);
        r = new SecurityAccess(socket2_, logger);
        dsc = new DiagnosticSessionControl(logger, socket2_);
        capture_frame = new CaptureFrame(socket_);
    }
    ~RoutineControlTest()
    {
        delete routine_control;
        delete r;
        delete dsc;
        delete capture_frame;
    }

    void checkSecurity()
    {
        /* Check the security */
        /* Request seed */
        r->securityAccess(0xFA10, {0x02, 0x27, 0x01});

        capture_frame->capture();
        if (capture_frame->frame.can_dlc >= 4)
        {
            seed.clear();
            /* from 3 to pci_length we have the seed generated in response */
            for (int i = 3; i <= capture_frame->frame.data[0]; i++)
            {
                seed.push_back(capture_frame->frame.data[i]);
            }
        }
        /* Compute key from seed */
        for (auto &elem : seed)
        {
            elem = computeKey(elem);
        }

        std::vector<uint8_t> data_frame = {static_cast<uint8_t>(seed.size() + 2), 0x27, 0x02};
        data_frame.insert(data_frame.end(), seed.begin(), seed.end());
        r->securityAccess(0xFA10, data_frame);
        capture_frame->capture();
    }
};

TEST_F(RoutineControlTest, IncorrectMessageLength)
{
     struct can_frame expected_frame = createFrame(0x001010FA, {0x03, 0x7F, 0x31, NegativeResponse::IMLOIF});

    routine_control->routineControl(0x0010fa10, {0x03, 0x31, 0x01, 0x02});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, SubFunctionNotSupported)
{
    struct can_frame expected_frame = createFrame(0x001010FA, {0x03, 0x7F, 0x31, 0x12});

    routine_control->routineControl(0x0010fa10, {0x05, 0x31, 0x00, 0x01, 0x03, 0x03});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, MCUSecurity)
{
    struct can_frame expected_frame = createFrame(0x001010FA, {0x03, 0x7F, 0x31, NegativeResponse::SAD});

    dsc->sessionControl(0x0010fa10, 0x03);
    capture_frame->capture();
    routine_control->routineControl(0x0010FA10, {0x05, 0x31, 0x01, 0x02, 0x01, 0x10});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, ECUSecurity)
{
    struct can_frame expected_frame = createFrame(0x001011FA, {0x03, 0x7F, 0x31, NegativeResponse::SAD});

    routine_control->routineControl(0x0010FA11, {0x05, 0x31, 0x01, 0x02, 0x01, 0x10});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, RequestOutOfRange)
{

    struct can_frame expected_frame = createFrame(0x001010FA, {0x03, 0x7F, 0x31, NegativeResponse::ROOR});

    checkSecurity();

    routine_control->routineControl(0x0010FA10, {0x05, 0x31, 0x01, 0x00, 0x01, 0x10});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, EraseMemory)
{
    struct can_frame expected_frame = createFrame(0x001010FA, {0x05, 0x71, 0x01, 0x01, 0x01, 0x00});

    checkSecurity();

    routine_control->routineControl(0x0010FA10, {0x05, 0x31, 0x01, 0x01, 0x01, 0x00});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, WrongSession)
{
    dsc->sessionControl(0x0010fa10, 0x02);
    capture_frame->capture();

    struct can_frame expected_frame = createFrame(0x001010FA, {0x03, 0x7F, 0x31, NegativeResponse::SFNSIAS});
    checkSecurity();

    routine_control->routineControl(0x0010FA10, {0x05, 0x31, 0x01, 0x02, 0x01, 0x00});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, OTAStateAlreadyInitialised)
{
    dsc->sessionControl(0x0010fa10, 0x03);
    capture_frame->capture();
    
    struct can_frame expected_frame = createFrame(0x001010FA, {0x03, 0x7F, 0x31, NegativeResponse::CNC});
    checkSecurity();

    std::unordered_map<uint16_t, std::vector<uint8_t>> data_map = {{0x01E0, {0x40}}};
    FileManager::writeMapToFile(file_path, data_map);

    routine_control->routineControl(0x0010FA10, {0x05, 0x31, 0x01, 0x02, 0x01, 0x00});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, VersionNotFound )
{
    std::unordered_map<uint16_t, std::vector<uint8_t>> data_map = {{0x01E0, {0xff}}};
    FileManager::writeMapToFile(file_path, data_map);
    struct can_frame expected_frame = createFrame(0x001010FA, {0x03, 0x7F, 0x31, NegativeResponse::IMLOIF});
    checkSecurity();

    routine_control->routineControl(0x0010FA10, {0x05, 0x31, 0x01, 0x02, 0x01, 0x70});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, InvalidEcuPath)
{
    struct can_frame expected_frame = createFrame(0x001015FA, {0x03, 0x7F, 0x31, NegativeResponse::SFNSIAS});
    checkSecurity();

    routine_control->routineControl(0x0010FA15, {0x05, 0x31, 0x01, 0x03, 0x01, 0x10});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, PositiveResponse1)
{
    struct can_frame expected_frame = createFrame(0x001010FA, {0x05, 0x71, 0x01, 0x04, 0x01, 0x00});
    checkSecurity();

    routine_control->routineControl(0x0010FA10, {0x05, 0x31, 0x01, 0x04, 0x01, 0x10});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, OTARollbackWrongSession)
{ 
    struct can_frame expected_frame = createFrame(0x001010FA, {0x03, 0x7F, 0x31, NegativeResponse::CNC});
    checkSecurity();

    routine_control->routineControl(0x0010FA10, {0x05, 0x31, 0x01, 0x05, 0x01, 0x00});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, SaveCurrentSoftwareFailed)
{ 
    struct can_frame expected_frame = createFrame(0x001010FA, {0x03, 0x7F, 0x31, NegativeResponse::IMLOIF});
    checkSecurity();

    routine_control->routineControl(0x0010FA10, {0x05, 0x31, 0x01, 0x06, 0x01, 0x00});
    capture_frame->capture();
    testFrames(expected_frame, *capture_frame);
}

TEST_F(RoutineControlTest, DISABLED_Rollback)
{ 
    // TODO: rework this test as activateSoftware is a private method
    // which is only called inside RoutineControl's ctor
    // EXPECT_EQ(routine_control->activateSoftware(), 0);
}

int main(int argc, char* argv[])
{
    socket_ = createSocket(0);
    socket2_ = createSocket(0);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}