#include <cstring>
#include <string>
#include <thread>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <gtest/gtest.h>
#include <net/if.h>

#include "../include/AccessTimingParameter.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/TestUtils.h"
#include "../../../ecu_simulation/BatteryModule/include/BatteryModule.h"
#include "../../../ecu_simulation/EngineModule/include/EngineModule.h"
#include "../../../ecu_simulation/DoorsModule/include/DoorsModule.h"
#include "../../../ecu_simulation/HVACModule/include/HVACModule.h"

int socket1;
int socket2;

std::vector<uint8_t> seed;

struct AccessTimingParameterTest : testing::Test
{
    AccessTimingParameter* atp;
    CaptureFrame* c1;
    Logger* logger;
    AccessTimingParameterTest()
    {
        logger = new Logger();
        atp = new AccessTimingParameter(*logger, socket2);
        c1 = new CaptureFrame(socket1);
    }
    ~AccessTimingParameterTest()
    {
        delete atp;
        delete c1;
        delete logger;
    }
};

/* Test for Read Extended Timing Parameter */
TEST_F(AccessTimingParameterTest, ReadExtendedTimingParameter) {
    std::cerr << "Running ReadExtendedTimingParameter" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x06, 0xC3, 0x01, 0x00, 0x28, 0x01, 0x90});

    atp->handleRequest(0xFA10, 0x01, {});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished ReadExtendedTimingParameter" << std::endl;
}

/* Test for Read Currently Active Timing Parameter */
TEST_F(AccessTimingParameterTest, ReadCurrentlyActiveTimingParameter) {
    std::cerr << "Running ReadCurrentlyActiveTimingParameter" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x06, 0xC3, 0x03, 0x00, 0x28, 0x01, 0x90});

    atp->handleRequest(0xFA10, 0x03, {});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished ReadCurrentlyActiveTimingParameter" << std::endl;
}

/* Test for Set Timing Parameter To Default */
TEST_F(AccessTimingParameterTest, SetTimingParameterToDefault) {
    std::cerr << "Running SetTimingParameterToDefault" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x02, 0xC3, 0x02});

    atp->handleRequest(0xFA10, 0x02, {});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished SetTimingParameterToDefault" << std::endl;
}

/* Test for Set Timing Parameter */
TEST_F(AccessTimingParameterTest, SetTimingParameter) {
    std::cerr << "Running SetTimingParameter" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x02, 0xC3, 0x04});

    atp->handleRequest(0xFA10, 0x04, {0x00, 0x50, 0x03, 0x20});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished SetTimingParameter" << std::endl;
}

/* Test for Set Timing Parameter Wrong Lenght */
TEST_F(AccessTimingParameterTest, SetTimingParameterWrongLenght) {
    std::cerr << "Running SetTimingParameterWrongLenght" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x83, 0x13});

    atp->handleRequest(0xFA10, 0x04, {0x00, 0x50});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished SetTimingParameterWrongLenght" << std::endl;
}

/* Test for Unsupported Subfunction */
TEST_F(AccessTimingParameterTest, UnsupportedSubfunction) {
    std::cerr << "Running UnsupportedSubfunction" << std::endl;
    
    struct can_frame result_frame = createFrame(0x10FA, {0x03, 0x7F, 0x83, 0x12});

    atp->handleRequest(0xFA10, 0x05, {});
    c1->capture();
    testFrames(result_frame, *c1);
    std::cerr << "Finished UnsupportedSubfunction" << std::endl;
}

/* Test for Stop TimingFlag for battery */
TEST_F(AccessTimingParameterTest, StopTimingFlagBattery) {
    std::cerr << "Running StopTimingFlagBattery" << std::endl;
    
    battery->_ecu->stop_flags[0x83] = true;
    atp->stopTimingFlag(0x11, 0x83);

    EXPECT_FALSE(battery->_ecu->stop_flags[0x83]);
    std::cerr << "Finished StopTimingFlagBattery" << std::endl;
}

/* Test for Stop TimingFlag for engine */
TEST_F(AccessTimingParameterTest, StopTimingFlagEngine) {
    std::cerr << "Running StopTimingFlagEngine" << std::endl;
    
    engine->_ecu->stop_flags[0x83] = true;
    atp->stopTimingFlag(0x12, 0x83);

    EXPECT_FALSE(engine->_ecu->stop_flags[0x83]);
    std::cerr << "Finished StopTimingFlagEngine" << std::endl;
}

/* Test for Stop TimingFlag for dooors */
TEST_F(AccessTimingParameterTest, StopTimingFlagDoors) {
    std::cerr << "Running StopTimingFlagDoors" << std::endl;
    
    doors->_ecu->stop_flags[0x83] = true;
    atp->stopTimingFlag(0x13, 0x83);

    EXPECT_FALSE(doors->_ecu->stop_flags[0x83]);
    std::cerr << "Finished StopTimingFlagDoors" << std::endl;
}

/* Test for Stop TimingFlag for HVAC */
TEST_F(AccessTimingParameterTest, StopTimingFlagHVAC) {
    std::cerr << "Running StopTimingFlagHVAC" << std::endl;
    
    hvac->_ecu->stop_flags[0x83] = true;
    atp->stopTimingFlag(0x14, 0x83);

    EXPECT_FALSE(hvac->_ecu->stop_flags[0x83]);
    std::cerr << "Finished StopTimingFlagHVAC" << std::endl;
}

/* Test for Stop TimingFlag for default case */
TEST_F(AccessTimingParameterTest, StopTimingFlagDefault) {
    std::cerr << "Running StopTimingFlagDefault" << std::endl;
    
    atp->stopTimingFlag(0x15, 0x83);

    std::cerr << "Finished StopTimingFlagDefault" << std::endl;
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
