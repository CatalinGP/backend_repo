#include <cstring>
#include <string>
#include <thread>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <gtest/gtest.h>
#include <net/if.h>

#include "../include/DiagnosticSessionControl.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/TestUtils.h"

int socket1;
int socket2;

struct DiagnosticSessionControlTest : testing::Test
{
    DiagnosticSessionControl * dsc;
    CaptureFrame* c1;
    Logger* logger;
    DiagnosticSessionControlTest()
    {
        logger = new Logger();
        dsc = new DiagnosticSessionControl(*logger, socket2);
        c1 = new CaptureFrame(socket1);
    }
    ~DiagnosticSessionControlTest()
    {
        delete dsc;
        delete c1;
        delete logger;
    }
};

/* Test for get Current Session To String method */
TEST_F(DiagnosticSessionControlTest, GetCurrentSessionToStringUnknownSession) {
    dsc->getCurrentSessionToString();
}

/* Test for Switching to Default Session */
TEST_F(DiagnosticSessionControlTest, SwitchToDefaultSession) {
    std::cerr << "Running SwitchToDefaultSession" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x02, 0x50, 0x01});

    dsc->sessionControl(0xFA10, 0x01);
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished SwitchToDefaultSession" << std::endl;
}

/* Test for get Current Session method */
TEST_F(DiagnosticSessionControlTest, GetCurrentSession) {
    EXPECT_EQ(dsc->getCurrentSession(), DEFAULT_SESSION);
}

/* Test for get Current Session To String method in Default Session*/
TEST_F(DiagnosticSessionControlTest, GetCurrentSessionToStringDefaultSession) {
    EXPECT_EQ(dsc->getCurrentSessionToString(), "DEFAULT_SESSION");
}


/* Test for Switching to Programming Session */
TEST_F(DiagnosticSessionControlTest, SwitchToProgrammingSession) {
    std::cerr << "Running SwitchToProgrammingSession" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x02, 0x50, 0x02});

    dsc->sessionControl(0xFA10, 0x02);
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished SwitchToProgrammingSession" << std::endl;
}

/* Test for Switching to Programming Session by Tester present*/
TEST_F(DiagnosticSessionControlTest, SwitchToProgrammingSessionTesterPresent) {
    std::cerr << "Running SwitchToProgrammingSession" << std::endl;

    dsc->sessionControl(0xFA10, 0x02,true);
    c1->capture();
    EXPECT_EQ(dsc->getCurrentSessionToString(), "PROGRAMMING_SESSION");
    std::cerr << "Finished SwitchToProgrammingSession" << std::endl;
}

/* Test for get Current Session To String method in Programming Session*/
TEST_F(DiagnosticSessionControlTest, GetCurrentSessionToStringProgrammingSession) {
    EXPECT_EQ(dsc->getCurrentSessionToString(), "PROGRAMMING_SESSION");
}

/* Test for Switching to Extended Session */
TEST_F(DiagnosticSessionControlTest, SwitchToExtendedSession) {
    std::cerr << "Running SwitchToExtendedSession" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x02, 0x50, 0x03});

    dsc->sessionControl(0xFA10, 0x03);
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished SwitchToExtendedSession" << std::endl;
}

/* Test for get Current Session To String method in Extended Session */
TEST_F(DiagnosticSessionControlTest, GetCurrentSessionToStringExtendedSession) {
    EXPECT_EQ(dsc->getCurrentSessionToString(), "EXTENDED_DIAGNOSTIC_SESSION");
}

/* Test for Unsupported Subfunction */
TEST_F(DiagnosticSessionControlTest, UnsupportedSubfunction) {
    std::cerr << "Running UnsupportedSubfunction" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x10, 0x12});

    dsc->sessionControl(0xFA10, 0x04);
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished UnsupportedSubfunction" << std::endl;
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
