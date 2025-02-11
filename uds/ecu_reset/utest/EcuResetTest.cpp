/**
 * @file EcuResetTest.cpp
 * @author Theodor Stoica
 * @brief Unit test for ECU Reset Service
 * @version 0.1
 * @date 2024-10-11
 */
#include <cstddef>
#include <fcntl.h>
#include <memory>
#include <gtest/gtest.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include "../include/EcuReset.h"
#include "../include/DummyEcuReset.h"
#include "../../authentication/include/SecurityAccess.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/ReceiveFrames.h"
#include "../../../utils/include/NegativeResponse.h"
#include "../../../utils/include/TestUtils.h"

int socket1;
int socket2;

std::vector<uint8_t> seed;

struct EcuResetTest : testing::Test
{
    SecurityAccess* security;
    CaptureFrame* c1;
    Logger* logger;
    EcuResetTest()
    {
        logger = new Logger();
        security = new SecurityAccess(socket2, *logger);
        c1 = new CaptureFrame(socket1);
    }
    ~EcuResetTest()
    {
        delete security;
        delete c1;
        delete logger;
    }
};

TEST_F(EcuResetTest, ConstructorInitializesFieldsCorrectly)
{
    std::unique_ptr<DummyEcuReset> ecuReset;
    EXPECT_NO_THROW(
    {
        ecuReset = std::make_unique<DummyEcuReset>(0xFA10, 0x01, socket2, *logger);
    });
}

TEST_F(EcuResetTest, IncorrectMessageLength)
{
    EcuReset *ecuReset;
    ecuReset = new DummyEcuReset(0xFA10, 0x01, socket2, *logger);
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x11, NegativeResponse::IMLOIF});
    ecuReset->ecuResetRequest({0x01, 0x11});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;
}

TEST_F(EcuResetTest, SubFunctionNotSupported)
{
    EcuReset *ecuReset;
    ecuReset = new DummyEcuReset(0xFA10, 0x03, socket2, *logger);
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x11, NegativeResponse::SFNS});
    ecuReset->ecuResetRequest({0x02, 0x11,0x03});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;
}

TEST_F(EcuResetTest, MCUSecurity)
{
    EcuReset *ecuReset;
    ecuReset = new DummyEcuReset(0xFA10, 0x01, socket2, *logger);
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x11, NegativeResponse::SAD});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;
}

TEST_F(EcuResetTest, ECUSecurity)
{
    /* Battery */
    EcuReset *ecuReset;
    ecuReset = new DummyEcuReset(0xFA11, 0x01, socket2, *logger);
    struct can_frame result_frame = createFrame(0x11FA, {0x03, 0x7F, 0x11, NegativeResponse::SAD});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* Engine */
    ecuReset = new DummyEcuReset(0xFA12, 0x01, socket2, *logger);
    result_frame = createFrame(0x12FA, {0x03, 0x7F, 0x11, NegativeResponse::SAD});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* Doors */
    ecuReset = new DummyEcuReset(0xFA13, 0x01, socket2, *logger);
    result_frame = createFrame(0x13FA, {0x03, 0x7F, 0x11, NegativeResponse::SAD});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* HVAC */
    ecuReset = new DummyEcuReset(0xFA14, 0x01, socket2, *logger);
    result_frame = createFrame(0x14FA, {0x03, 0x7F, 0x11, NegativeResponse::SAD});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;
}

TEST_F(EcuResetTest, HardResetMCU)
{
    EcuReset *ecuReset;

    security->securityAccess(0xFA10, {0x02, 0x27, 0x01});

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
    security->securityAccess(0xFA10, data_frame);
    c1->capture();

    ecuReset = new DummyEcuReset(0xFA10, 0x01, socket2, *logger);
    struct can_frame result_frame = createFrame(0x10FA, {0x02, 0x51,0x01});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;
}

TEST_F(EcuResetTest, HardResetECU)
{
    /* Battery */
    EcuReset *ecuReset;
    ReceiveFrames* receiveFrames = new ReceiveFrames(socket2, 0x11, *logger);
    receiveFrames->setEcuState(true);
    ecuReset = new DummyEcuReset(0xFA11, 0x01, socket2, *logger);
    struct can_frame result_frame = createFrame(0x11FA, {0x02, 0x51,0x01});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;
    delete receiveFrames;

    /* Engine */
    ecuReset = new DummyEcuReset(0xFA12, 0x01, socket2, *logger);
    result_frame = createFrame(0x12FA, {0x02, 0x51,0x01});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* Doors */
    ecuReset = new DummyEcuReset(0xFA13, 0x01, socket2, *logger);
    result_frame = createFrame(0x13FA, {0x02, 0x51,0x01});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* HVAC */
    ecuReset = new DummyEcuReset(0xFA14, 0x01, socket2, *logger);
    result_frame = createFrame(0x14FA, {0x02, 0x51,0x01});
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* Other ECU */
    ecuReset = new DummyEcuReset(0xFA15, 0x01, socket2, *logger);
    ecuReset->ecuResetRequest({0x02, 0x11,0x01});
    c1->capture();
    delete ecuReset;
}

TEST_F(EcuResetTest, SoftResetMCU)
{
    EcuReset *ecuReset;
    ecuReset = new DummyEcuReset(0xFA10, 0x02, socket2, *logger);
    struct can_frame result_frame = createFrame(0x10FA, {0x02, 0x51,0x02});
    ecuReset->ecuResetRequest({0x02, 0x11, 0x02});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;
}

TEST_F(EcuResetTest, SoftResetECU)
{
    /* Battery */
    EcuReset *ecuReset;
    ecuReset = new DummyEcuReset(0xFA11, 0x02, socket2, *logger);
    struct can_frame result_frame = createFrame(0x11FA, {0x02, 0x51,0x02});
    ecuReset->ecuResetRequest({0x02, 0x11, 0x02});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* Engine */
    ecuReset = new DummyEcuReset(0xFA12, 0x02, socket2, *logger);
    result_frame = createFrame(0x12FA, {0x02, 0x51,0x02});
    ecuReset->ecuResetRequest({0x02, 0x11, 0x02});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* Doors */
    ecuReset = new DummyEcuReset(0xFA13, 0x02, socket2, *logger);
    result_frame = createFrame(0x13FA, {0x02, 0x51,0x02});
    ecuReset->ecuResetRequest({0x02, 0x11, 0x02});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* HVAC */
    ecuReset = new DummyEcuReset(0xFA14, 0x02, socket2, *logger);
    result_frame = createFrame(0x14FA, {0x02, 0x51,0x02});
    ecuReset->ecuResetRequest({0x02, 0x11, 0x02});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;

    /* Other ECU */
    ecuReset = new DummyEcuReset(0xFA15, 0x02, socket2, *logger);
    result_frame = createFrame(0x15FA, {0x02, 0x51,0x02});
    ecuReset->ecuResetRequest({0x02, 0x11, 0x02});
    c1->capture();
    testFrames(result_frame, *c1);
    delete ecuReset;
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