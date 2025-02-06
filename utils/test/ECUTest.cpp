
#include <cstring>
#include <string>
#include <thread>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <gtest/gtest.h>
#include <net/if.h>

#include "../include/ECU.h"
#include "../include/GenerateFrames.h"
#include "../../utils/include/CaptureFrame.h"
#include "../../utils/include/TestUtils.h"

int socket1;
int socket2;

struct ECUTest : testing::Test
{
    Logger* logger;
    ECU* ecu;
    CaptureFrame* c1;
    GenerateFrames* g;
    ECUTest()
    {
        logger = new Logger();
        ecu = new ECU(0x11, *logger);
        c1 = new CaptureFrame(socket1);
        g = new GenerateFrames(socket1, *logger);
    }
    ~ECUTest()
    {
        delete ecu;
        delete g;
        delete c1;
        delete logger;
    }
};

TEST_F(ECUTest, NotificationToMcu)
{
    std::cerr << "Running NotificationToMcu" << std::endl;

    struct can_frame result_frame = createFrame(0x1110, {0x01, 0xD9});
    ecu->sendNotificationToMCU();
    c1->capture();
    testFrames(result_frame, *c1);

    std::cerr << "Finished NotificationToMcu" << std::endl;
}

TEST_F(ECUTest, StartFrames)
{
    std::cerr << "Running StartFrames" << std::endl;

    testing::internal::CaptureStdout();
    std::thread processor_thread([this] {
        ecu->startFrames();
    });
    ecu->stopFrames();
    processor_thread.join();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("11 starts the frame receiver"), std::string::npos);

    std::cerr << "Finished StartFrames" << std::endl;
}

int main(int argc, char* argv[])
{
    socket1 = createSocket(0);
    socket2 = createSocket(0);
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