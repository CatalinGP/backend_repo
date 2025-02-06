/**
 * @file read_dtc_information_test.cpp
 * @author Mujdei Ruben
 * @brief Unit test for Read_dtc service
 * @version 0.1
 * @date 2024-07-03
 * 
 * @copyright Copyright (c) 2024
 * 
 * PS: Test work with the next data in the file 'dtcs.txt':
 *          P0A9B-17 24
 *          P0805-11 2F
 *          B1234-00 00
 *          P0B12-01 10
 * 
 */
#include "../include/ReadDtcInformation.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/TestUtils.h"

#include <cstring>
#include <linux/can.h>
#include <string>
#include <thread>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <gtest/gtest.h>
#include <net/if.h>
#include "Globals.h"

int socket_;
int socket2_;

/* Create object for all tests */
struct ReadDtcTest : testing::Test
{
    ReadDTC* r;
    CaptureFrame* c1;
    Logger* logger;
    ReadDtcTest()
    {
        std::string dtc_file_path = std::string(PROJECT_PATH) + "/backend/ecu_simulation/EngineModule/dtcs.txt";
        logger = new Logger("log_test_read_dtc","./log_test_read_dtc.log");
        r = new ReadDTC(*logger, dtc_file_path,socket2_);
        c1 = new CaptureFrame(socket_);
    }
    ~ReadDtcTest()
    {
        delete r;
        delete c1;
    }
};

TEST_F(ReadDtcTest, IncorrectMessageLength)
{
    struct can_frame result_frame = createFrame(0x12fa, {0x03, 0x7F, 0x19, 0x13});

    r->read_dtc(0xfa12, {0x03, 0x19, 0x01});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDtcTest, SubFunction1)
{
    struct can_frame result_frame = createFrame(0x12fa, {0x06, 0x59, 0x01, 0x24, 0x01, 0x00, 0x02});

    r->read_dtc(0xfa12, {0x4,0x19,0x01, 0xff});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDtcTest, SubFunction2)
{
    struct can_frame result_frame = createFrame(0x10fa, {0x010, 0x09, 0x59, 0x02, 0x24, 0x01, 0x90, 0x24});
    r->read_dtc(0xfa10, {0x4,0x19,0x02, 0xff});
    /* First frame */
    c1->capture();
    testFrames(result_frame, *c1);

    result_frame = createFrame(0x10fa, {0x21, 0x01, 0x96, 0x24});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDtcTest, SubfunctionNotSupported)
{
    struct can_frame result_frame = createFrame(0x12fa, {0x03, 0x7F, 0x19, 0x12});

    r->read_dtc(0xfa12, {0x4,0x19,0x03, 0xff});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDtcTest, UnableToOpenFile1)
{
    std::string dtc_file_path = std::string(PROJECT_PATH) + "/backend/ecu_simulation/WrongPath/dtcs.txt";
    r = new ReadDTC(*logger, dtc_file_path,socket2_);
    struct can_frame result_frame = createFrame(0x12fa, {0x03, 0x7F, 0x19, 0x94});

    r->read_dtc(0xfa12, {0x4,0x19,0x01, 0xff});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDtcTest, UnableToOpenFile2)
{
    std::string dtc_file_path = std::string(PROJECT_PATH) + "/backend/ecu_simulation/WrongPath/dtcs.txt";
    r = new ReadDTC(*logger, dtc_file_path,socket2_);
    struct can_frame result_frame = createFrame(0x12fa, {0x03, 0x7F, 0x19, 0x94});

    r->read_dtc(0xfa12, {0x4,0x19,0x02, 0xff});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(ReadDtcTest, NoDtcFound)
{
    std::string dtc_file_path = std::string(PROJECT_PATH) + "/backend/uds/read_dtc_information/dtcs.txt";
    r = new ReadDTC(*logger, dtc_file_path, socket2_);
    struct can_frame result_frame = createFrame(0x12fa, {0x03, 0x59, 0x02, 0x00});

    r->read_dtc(0xfa12, {0x4,0x19,0x02, 0xff});
    c1->capture();
    testFrames(result_frame, *c1);
}

int main(int argc, char* argv[])
{
    socket_ = createSocket(0);
    socket2_ = createSocket(0);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}