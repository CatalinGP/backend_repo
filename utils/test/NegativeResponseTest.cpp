/**
 * @file SecurityAccessTest.cpp
 * @author Theodor Stoica
 * @brief Unit test for NegativeResponse Service
 * @version 0.1
 * @date 2024-07-16
 */
#include "../include/NegativeResponse.h"
#include "../../utils/include/CaptureFrame.h"
#include "../../utils/include/TestUtils.h"

#include <cstring>
#include <linux/can.h>
#include <string>
#include <thread>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <gtest/gtest.h>
#include <net/if.h>

int socket_;
int socket2_;
const int id = 0x10FA;

std::vector<uint8_t> seed;

struct NegativeResponseTest : testing::Test
{
    NegativeResponse* r;
    CaptureFrame* c1;
    Logger* logger;
    NegativeResponseTest()
    {
        logger = new Logger();
        r = new NegativeResponse(socket2_, *logger);
        c1 = new CaptureFrame(socket_);
    }
    ~NegativeResponseTest()
    {
        delete r;
        delete c1;
        delete logger;
    }
};

TEST_F(NegativeResponseTest, SendNRC)
{
    struct can_frame result_frame = createFrame(id, {0x03, 0x7F, 0x27, 0x13});
    r->sendNRC(0xFA10, 0x27, 0x13);
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(NegativeResponseTest, InvalidNRC)
{
    testing::internal::CaptureStdout();
    r->sendNRC(0xFA10, 0x27, 0x99);
    c1->capture();
    std::string output_nrc = "";
    std::string searchLine = "Negative Response not supported";
    output_nrc = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(containsLine(output_nrc, searchLine));
}

TEST_F(NegativeResponseTest, AccessTimingNRC)
{
    struct can_frame result_frame = createFrame(id, {0x03, 0x7F, 0x27, 0x78});
    r->sendNRC(0xFA10, 0x27, 0x78);
    c1->capture();
    testFrames(result_frame, *c1);
}

int main(int argc, char* argv[])
{
    socket_ = createSocket(1);
    socket2_ = createSocket(1);
    testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    if (socket_ > 0)
    {
        close(socket_);
    }
    if (socket2_ > 0)
    {
        close(socket2_);
    }
    return result;
}
