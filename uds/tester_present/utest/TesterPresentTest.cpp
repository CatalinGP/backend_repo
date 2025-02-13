/**
 * @file TesterPresentTest.cpp
 * @author Theodor Stoica
 * @brief Unit test for TesterPresent service
 * @version 0.1
 * @date 2024-07-03
 * 
 */

#include "../include/TesterPresent.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/NegativeResponse.h"
#include "../../../utils/include/TestUtils.h"
#include <cstring>
#include <string>
#include <thread>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <gtest/gtest.h>
#include <net/if.h>

int socket_;
int socket2_;
/*ID frame for response */
const int id = 0x10FA;

struct TesterPresentTest : testing::Test
{
    TesterPresent* tp;
    CaptureFrame* c1;
    Logger* logger;
    DiagnosticSessionControl* session;
    TesterPresentTest()
    {
        logger = new Logger();
        session = new DiagnosticSessionControl(*logger, socket2_);
        tp = new TesterPresent(socket2_, *logger,*session);
        c1 = new CaptureFrame(socket_);
        /* Set session to default for each test */
        session->sessionControl(0xFA10,0x01);
        /* Read the frame to be ready to read the next frame */
        c1->capture();
    }
    ~TesterPresentTest()
    {
        delete tp;
        delete c1;
        delete session;
        delete logger;
    }
};

TEST_F(TesterPresentTest, SetEndTimeDefaultSession)
{
    tp->setEndTimeProgrammingSession();
    auto expected_programming_end_time = std::chrono::steady_clock::now();
    auto end_time = tp->getEndTimeProgrammingSession();
    EXPECT_NEAR(std::chrono::duration_cast<std::chrono::hours>
        (
            end_time - expected_programming_end_time
        ).count(),24 * 365, 1);
}

TEST_F(TesterPresentTest, SetEndTimeProgrammingSession)
{
    tp->setEndTimeProgrammingSession(true);
    auto expected_programming_end_time = std::chrono::steady_clock::now();
    auto end_time = tp->getEndTimeProgrammingSession();
    EXPECT_NEAR(std::chrono::duration_cast<std::chrono::seconds>
        (
            end_time - expected_programming_end_time
        ).count(),TesterPresent::S3_TIMER, 1);
}

TEST_F(TesterPresentTest, IncorrectMesssageLength)
{
    struct can_frame result_frame = createFrame(id, {0x03, 0x7F, 
                    TesterPresent::TESTER_PRESENT_SID, NegativeResponse::IMLOIF});

    tp->handleTesterPresent(0xFA10, {0x04, 0x2E, 0x00});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(TesterPresentTest, SubFunctionNotSupported)
{
    struct can_frame result_frame = createFrame(id, {0x03, 0x7F, 
                    TesterPresent::TESTER_PRESENT_SID, NegativeResponse::SFNS});

    tp->handleTesterPresent(0xFA10, {0x02, 0x3E, 0x01});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(TesterPresentTest, ProgrammingToProgrammingSession)
{
    struct can_frame result_frame = createFrame(id, {0x02, 0x7E, 0x00});
    session->sessionControl(0xFA10, 0x02);
    c1->capture();
    tp->handleTesterPresent(0xFA10, {0x02, 0x3E, 0x00});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(TesterPresentTest, DefaultToProgrammingSession)
{
    struct can_frame result_frame = createFrame(id, {0x02, 0x7E, 0x00});
    tp->handleTesterPresent(0xFA10, {0x02, 0x3E, 0x00});
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
