#include <cstring>
#include <string>
#include <thread>
#include <fcntl.h>
#include <fstream>
#include <sys/ioctl.h>
#include <gtest/gtest.h>
#include <net/if.h>

#include "../include/WriteDataByIdentifier.h"
#include "../../../uds/authentication/include/SecurityAccess.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/ReceiveFrames.h"
#include "../../../utils/include/NegativeResponse.h"
#include "../../../utils/include/TestUtils.h"
#include "Globals.h"

int socket1;
int socket2;

std::vector<uint8_t> seed;

struct WriteDataByIdentifierTest : testing::Test
{
    WriteDataByIdentifier* w;
    SecurityAccess* r;
    CaptureFrame* c1;
    Logger* logger;
    WriteDataByIdentifierTest()
    {
        logger = new Logger();
        w = new WriteDataByIdentifier(*logger, socket2);
        r = new SecurityAccess(socket2, *logger);
        c1 = new CaptureFrame(socket1);
    }
    ~WriteDataByIdentifierTest()
    {
        delete w;
        delete r;
        delete c1;
        delete logger;
    }
};

/* Test for Incorrect Message Length */
TEST_F(WriteDataByIdentifierTest, IncorrectMessageLength) {
    std::cerr << "Running TestIncorrectMessageLength" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x2e, NegativeResponse::IMLOIF});

    w->WriteDataByIdentifierService(0xFA10, {0x01, 0x2e});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished TestIncorrectMessageLength" << std::endl;
}

/* Test for MCU security */
TEST_F(WriteDataByIdentifierTest, MCUSecurity) {
    std::cerr << "Running MCUSecurity" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x2e, NegativeResponse::SAD});

    w->WriteDataByIdentifierService(0xFA10, {0x04, 0x2e, 0xf1, 0x90, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished MCUSecurity" << std::endl;
}

/* Test for ECUs security */
TEST_F(WriteDataByIdentifierTest, ECUsSecurity) {
    std::cerr << "Running ECUsSecurity" << std::endl;
    
    struct can_frame result_frame = createFrame(0x11FA, {0x03, 0x7F, 0x2e, NegativeResponse::SAD});

    w->WriteDataByIdentifierService(0xFA11, {0x04, 0x2e, 0xf1, 0x90, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished ECUsSecurity" << std::endl;
}

/* Test Request Out Of Range MCU */
TEST_F(WriteDataByIdentifierTest, RequestOutOfRangeMCU) {
    std::cerr << "Running RequestOutOfRangeMCU" << std::endl;

    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x2e, NegativeResponse::ROOR});

    /* Check the security */
    /* Request seed */
    r->securityAccess(0xFA10, {0x02, 0x27, 0x01});

    c1->capture();
    if (c1->frame.can_dlc >= 4)
    {
        seed.clear();
        /* from 3 to pci_length we have the seed generated in response */
        for (int i = 3; i <= c1->frame.data[0]; i++)
        {
            seed.push_back(c1->frame.data[i]);
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
    c1->capture();

    w->WriteDataByIdentifierService(0xFA10, {0x04, 0x2e, 0x11, 0x11, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished RequestOutOfRangeMCU" << std::endl;
}

/* Test Request Out Of Range Battery */
TEST_F(WriteDataByIdentifierTest, RequestOutOfRangeBattery) {
    std::cerr << "Running RequestOutOfRangeBattery" << std::endl;
    ReceiveFrames* receiveFrames = new ReceiveFrames(socket2, 0x11, *logger);
    receiveFrames->setEcuState(true);
    struct can_frame result_frame = createFrame(0x11FA, {0x03, 0x7F, 0x2e, NegativeResponse::ROOR});
    w->WriteDataByIdentifierService(0xFA11, {0x04, 0x2e, 0x11, 0x11, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished RequestOutOfRangeBattery" << std::endl;
    delete receiveFrames;
}

/* Test Request Out Of Range Engine */
TEST_F(WriteDataByIdentifierTest, RequestOutOfRangeEngine) {
    std::cerr << "Running RequestOutOfRangeEngine" << std::endl;

    struct can_frame result_frame = createFrame(0x12FA, {0x03, 0x7F, 0x2e, NegativeResponse::ROOR});

    w->WriteDataByIdentifierService(0xFA12, {0x04, 0x2e, 0x11, 0x11, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished RequestOutOfRangeEngine" << std::endl;
}

/* Test Request Out Of Range Doors */
TEST_F(WriteDataByIdentifierTest, RequestOutOfRangeDoors) {
    std::cerr << "Running RequestOutOfRangeDoors" << std::endl;

    struct can_frame result_frame = createFrame(0x13FA, {0x03, 0x7F, 0x2e, NegativeResponse::ROOR});

    w->WriteDataByIdentifierService(0xFA13, {0x04, 0x2e, 0x11, 0x11, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished RequestOutOfRangeDoors" << std::endl;
}

/* Test Request Out Of Range HVAC */
TEST_F(WriteDataByIdentifierTest, RequestOutOfRangeHVAC) {
    std::cerr << "Running RequestOutOfRangeHVAC" << std::endl;

    struct can_frame result_frame = createFrame(0x14FA, {0x03, 0x7F, 0x2e, NegativeResponse::ROOR});

    w->WriteDataByIdentifierService(0xFA14, {0x04, 0x2e, 0x11, 0x11, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished RequestOutOfRangeHVAC" << std::endl;
}

/* Test Corect DID MCU */
TEST_F(WriteDataByIdentifierTest, CorectDIDMCU) {
    std::cerr << "Running CorectDIDMCU" << std::endl;
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x6e, 0xf1, 0x90});
    w->WriteDataByIdentifierService(0xFA10, {0x04, 0x2e, 0xf1, 0x90, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished CorectDIDMCU" << std::endl;
}

/* Test Corect DID Battery */
TEST_F(WriteDataByIdentifierTest, CorectDIDBattery) {
    std::cerr << "Running CorectDIDBattery" << std::endl;
    struct can_frame result_frame = createFrame(0x11FA, {0x03, 0x6e, 0x01, 0xa0});
    w->WriteDataByIdentifierService(0xFA11, {0x04, 0x2e, 0x01, 0xa0, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished CorectDIDBattery" << std::endl;
}

/* Test Corect DID Engine */
TEST_F(WriteDataByIdentifierTest, CorectDIDEngine) {
    std::cerr << "Running CorectDIDEngine" << std::endl;

    struct can_frame result_frame = createFrame(0x12FA, {0x03, 0x6e, 0x01, 0x24});

    w->WriteDataByIdentifierService(0xFA12, {0x04, 0x2e, 0x01, 0x24, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished CorectDIDEngine" << std::endl;
}

/* Test Corect DID Doors */
TEST_F(WriteDataByIdentifierTest, CorectDIDDoors) {
    std::cerr << "Running CorectDIDDoors" << std::endl;

    struct can_frame result_frame = createFrame(0x13FA, {0x03, 0x6e, 0x03, 0xa0});

    w->WriteDataByIdentifierService(0xFA13, {0x04, 0x2e, 0x03, 0xa0, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished CorectDIDDoors" << std::endl;
}

/* Test Corect DID HVAC */
TEST_F(WriteDataByIdentifierTest, CorectDIDHVAC) {
    std::cerr << "Running CorectDIDHVAC" << std::endl;

    struct can_frame result_frame = createFrame(0x14FA, {0x03, 0x6e, 0x01, 0x34});

    w->WriteDataByIdentifierService(0xFA14, {0x04, 0x2e, 0x01, 0x34, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished CorectDIDHVAC" << std::endl;
}

/* Test Module not supported */
TEST_F(WriteDataByIdentifierTest, ModuleNotSupported) {
    std::cerr << "Running ModuleNotSupported" << std::endl;

    struct can_frame result_frame = createFrame(0x15FA, {0x03, 0x7F, 0x2e, NegativeResponse::ROOR});

    w->WriteDataByIdentifierService(0xFA15, {0x04, 0x2e, 0x11, 0x11, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished ModuleNotSupported" << std::endl;
}

TEST_F(WriteDataByIdentifierTest, ErrorReadingFromFile)
{
    std::string file_name = std::string(PROJECT_PATH) + "/backend/mcu/mcu_data.txt";
    std::string original_content;
    std::ifstream original_file(file_name);
    if (original_file)
    {
        original_content.assign((std::istreambuf_iterator<char>(original_file)),
                                std::istreambuf_iterator<char>());
        original_file.close();
    }
    std::remove(file_name.c_str());

    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x2e, NegativeResponse::ROOR});
    w->WriteDataByIdentifierService(0xFA10, {0x04, 0x2e, 0x11, 0x11, 0x11});
    c1->capture();

    std::ofstream new_file(file_name);
    if (new_file)
    {
        if (!original_content.empty())
        {
            new_file << original_content;
        }
        new_file.close();
    }
    else
    {
        FAIL() << "Error recreating the file.";
    }
    std::ifstream recreated_file(file_name);
    EXPECT_TRUE(recreated_file.good());

    if (!original_content.empty())
    {
        std::string recreated_content((std::istreambuf_iterator<char>(recreated_file)),
                                      std::istreambuf_iterator<char>());
        EXPECT_EQ(recreated_content, original_content);
    }
    testFrames(result_frame, *c1);
}

int main(int argc, char* argv[])
{
    socket1 = createSocket(1);
    socket2 = createSocket(1);
    testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    if (socket1 > 0)
    {
        close(socket1);
    }
    if (socket2 > 0)
    {
        close(socket2);
    }
    return result;
}