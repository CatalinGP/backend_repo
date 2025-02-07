/**
 * @file ReadDataByIdentifierTest.cpp
 * @author Theodor Stoica
 * @brief Unit test for Read Data By Identifier Service
 * @version 0.1
 * @date 2024-10-9
 */
#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include "../include/ReadDataByIdentifier.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/ReceiveFrames.h"
#include "../../../utils/include/NegativeResponse.h"
#include "../../../utils/include/TestUtils.h"
#include "../../../uds/authentication/include/SecurityAccess.h"
#include "Globals.h"

int socket1;
int socket2;

std::vector<uint8_t> seed;

struct ReadDataByIdentifierTest : testing::Test
{
    ReadDataByIdentifier* rdbi;
    SecurityAccess* security;
    CaptureFrame* c1;
    Logger* logger;
    ReadDataByIdentifierTest()
    {
        logger = new Logger();
        rdbi = new ReadDataByIdentifier(socket2, *logger);
        security = new SecurityAccess(socket2, *logger);
        c1 = new CaptureFrame(socket1);
    }
    ~ReadDataByIdentifierTest()
    {
        delete rdbi;
        delete security;
        delete c1;
        delete logger;
    }
};

TEST_F(ReadDataByIdentifierTest, IncorrectMessageLength)
{
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x22, NegativeResponse::IMLOIF});
    rdbi->readDataByIdentifier(0xFA10, {0x01, 0x22}, true);
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDataByIdentifierTest, MCUSecurity)
{
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x22, NegativeResponse::SAD});
    rdbi->readDataByIdentifier(0xFA10, {0x04, 0x22, 0xf1, 0x90, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDataByIdentifierTest, ECUsSecurity)
{
    /* Battery Module */
    struct can_frame result_frame = createFrame(0x11FA, {0x03, 0x7F, 0x22, NegativeResponse::SAD});

    rdbi->readDataByIdentifier(0xFA11, {0x04, 0x22, 0xf1, 0x90, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);

    /* Engine Module */
    result_frame = createFrame(0x12FA, {0x03, 0x7F, 0x22, NegativeResponse::SAD});

    rdbi->readDataByIdentifier(0xFA12, {0x04, 0x22, 0xf1, 0x90, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);

    /* Doors Module */
    result_frame = createFrame(0x13FA, {0x03, 0x7F, 0x22, NegativeResponse::SAD});

    rdbi->readDataByIdentifier(0xFA13, {0x04, 0x22, 0xf1, 0x90, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);

    /* HVAC Module */
    result_frame = createFrame(0x14FA, {0x03, 0x7F, 0x22, NegativeResponse::SAD});
    rdbi->readDataByIdentifier(0xFA14, {0x04, 0x22, 0xf1, 0x90, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDataByIdentifierTest, RequestOutOfRangeMCU)
{

    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x22, NegativeResponse::ROOR});

    /* Check the security */
    /* Request seed */
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

    rdbi->readDataByIdentifier(0xFA10, {0x04, 0x22, 0x11, 0x11, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);
}

/* Test Request Out Of Range Battery */
TEST_F(ReadDataByIdentifierTest, RequestOutOfRangeBattery)
{
    ReceiveFrames* receiveFrames = new ReceiveFrames(socket2, 0x11, *logger);
    receiveFrames->setEcuState(true);
    struct can_frame result_frame = createFrame(0x11FA, {0x03, 0x7F, 0x22, NegativeResponse::ROOR});
    rdbi->readDataByIdentifier(0xFA11, {0x04, 0x22, 0x11, 0x11, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);
    delete receiveFrames;
}

/* Test Request Out Of Range Engine */
TEST_F(ReadDataByIdentifierTest, RequestOutOfRangeEngine)
{
    struct can_frame result_frame = createFrame(0x12FA, {0x03, 0x7F, 0x22, NegativeResponse::ROOR});
    rdbi->readDataByIdentifier(0xFA12, {0x04, 0x22, 0x11, 0x11, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);
}

/* Test Request Out Of Range Doors */
TEST_F(ReadDataByIdentifierTest, RequestOutOfRangeDoors)
{
    struct can_frame result_frame = createFrame(0x13FA, {0x03, 0x7F, 0x22, NegativeResponse::ROOR});
    rdbi->readDataByIdentifier(0xFA13, {0x04, 0x22, 0x11, 0x11, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);
}

/* Test Request Out Of Range HVAC */
TEST_F(ReadDataByIdentifierTest, RequestOutOfRangeHVAC)
{
    struct can_frame result_frame = createFrame(0x14FA, {0x03, 0x7F, 0x22, NegativeResponse::ROOR});
    rdbi->readDataByIdentifier(0xFA14, {0x04, 0x22, 0x11, 0x11, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);
}

/* Test Module not supported */
TEST_F(ReadDataByIdentifierTest, ModuleNotSupported)
{
    struct can_frame result_frame = createFrame(0x15FA, {0x03, 0x7F, 0x22, NegativeResponse::ROOR});

    rdbi->readDataByIdentifier(0xFA15, {0x04, 0x22, 0x11, 0x11, 0x11}, true);
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDataByIdentifierTest, ErrorReadingFromFile)
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

    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x22, NegativeResponse::ROOR});
    rdbi->readDataByIdentifier(0xFA10, {0x04, 0x22, 0x11, 0x11, 0x11}, true);
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

/* Test Corect DID MCU */
TEST_F(ReadDataByIdentifierTest, CorectDIDMCU)
{
    struct can_frame result_frame = createFrame(0x10FA, {0x04, 0x62, 0xF1, 0xA2, 0x00});
    rdbi->readDataByIdentifier(0xFA10, {0x03, 0x22, 0xF1, 0xA2}, true);
    c1->capture();
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