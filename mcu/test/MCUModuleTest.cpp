#include "gtest/gtest.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include "../include/MCUModule.h"
#include <linux/can.h>
#include "../../utils/include/Logger.h"
#include "../../utils/include/GenerateFrames.h"
#include "../../utils/include/Globals.h"
#include "../../utils/include/TestUtils.h"
#include "../../ecu_simulation/BatteryModule/include/BatteryModule.h"

int socket_canbus = -1;

void createMCUProcess()
{
    pid_t pid = fork();

    if (pid == 0)
    {
        prctl(PR_SET_NAME, "main_mcu", 0, 0, 0);
    }
    else if (pid > 0)
    {
        std::cout << "Parent process, launched main_mcu_process with PID " << pid << "\n";
    }
    else
    {
        std::cerr << "Fork failed\n";
    }
}

void testByteVectors(const std::vector<uint8_t>& expected, const std::vector<uint8_t>& actual)
{
    EXPECT_EQ(expected.size(), actual.size()) << "Vectors have different sizes";

    /* Compare the contents of the vectors */
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(expected[i], actual[i]) << "Vectors differ at index " << i;
    }
}

/**
 * @warning Running any unit test from this test suite excepting ErrorKillMCUProcess has led to the following issue:
 * Whatever is typed in the terminal used for running the test executable is invisible
 * After running `./mcuModuleTest`, in order to fix the issue, type `reset`
 */
class MCUModuleTest : public ::testing::Test {
protected:
    Logger* mockLogger;
    MCUModuleTest()
    {
        mockLogger = new Logger;
        loadProjectPathForMCUTest();
    }
    ~MCUModuleTest()
    {
        delete mockLogger;
    }
};

TEST_F(MCUModuleTest, ErrorKillMCUProcess)
{
    MCU::mcu = new MCU::MCUModule(0x01);
    MCU::mcu->StartModule();
    MCU::mcu->StopModule();
    testing::internal::CaptureStdout();
    delete MCU::mcu;
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(containsLine(output, "Error when trying to kill main_mcu process"));
}

/* Start and stop module Test  */
TEST_F(MCUModuleTest, StartModuleTest)
{
    /* Cover default constructor */
    createMCUProcess();
    MCU::mcu = new MCU::MCUModule();
    delete MCU::mcu;
    createMCUProcess();
    /* Cover parameterized constructor */
    testing::internal::CaptureStdout();
    MCU::mcu = new MCU::MCUModule(0x01);
    MCU::mcu->StartModule();
    std::string output = testing::internal::GetCapturedStdout();
    /* Expect mcu module to start */
    EXPECT_NE(output.find("Diagnostic Session Control (0x10) started."), std::string::npos);
    MCU::mcu->StopModule();
    delete MCU::mcu;
}

/* Get mcu_api socket test  */
TEST_F(MCUModuleTest, GetMcuApiSocketTest)
{
    MCU::mcu = new MCU::MCUModule(0x01);
    createMCUProcess();
    int socket = MCU::mcu->getMcuApiSocket();
    EXPECT_TRUE(socket > 0);
    delete MCU::mcu;
}

/* Get mcu_ecu socket test  */
TEST_F(MCUModuleTest, GetMcuEcuSocketTest)
{
    MCU::mcu = new MCU::MCUModule(0x01);
    createMCUProcess();
    int socket = MCU::mcu->getMcuEcuSocket();
    EXPECT_TRUE(socket > 0);
    delete MCU::mcu;
}

/* Set mcu_api socket test  */
TEST_F(MCUModuleTest, SetMcuApiSocketTest)
{
    MCU::mcu = new MCU::MCUModule(0x01);
    createMCUProcess();
    MCU::mcu->setMcuApiSocket(0x01);
    int socket = MCU::mcu->getMcuApiSocket();
    EXPECT_TRUE(socket > 0);
    delete MCU::mcu;
}

/* Set mcu_ecu socket test  */
TEST_F(MCUModuleTest, SetMcuEcuSocketTest)
{
    MCU::mcu = new MCU::MCUModule(0x01);
    createMCUProcess();
    MCU::mcu->setMcuEcuSocket(0x01);
    int socket = MCU::mcu->getMcuEcuSocket();
    EXPECT_TRUE(socket > 0);
    delete MCU::mcu;
}

TEST_F(MCUModuleTest, receiveFramesTest)
{

    /* Initialize the MCU module and interfaces */
    MCU::mcu = new MCU::MCUModule(0x01);
    createMCUProcess();
    MCU::mcu->StartModule();
    CreateInterface* interface = CreateInterface::getInstance(0x01, *mockLogger);
    interface->startInterface();
    testing::internal::CaptureStdout();
    std::thread receiver_thread([this] {
        MCU::mcu->recvFrames();
    });
    GenerateFrames* generate_frames = new GenerateFrames(socket_canbus, *mockLogger);
    generate_frames->sendFrame(0x1110, {0x00, 0x01, 0x02}, DATA_FRAME);
    sleep(1);
    MCU::mcu->StopModule();
    /* Join the threads to ensure completion */
    receiver_thread.join();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(containsLine(output, "Frame processing method invoked!"));
    delete MCU::mcu;
}

TEST_F(MCUModuleTest, WriteFailMCUData)
{
    std::ofstream outfile(std::string(PROJECT_PATH) + "/backend/mcu/old_mcu_data.txt");
    MCU::mcu = new MCU::MCUModule(0x01);
    createMCUProcess();
    MCU::mcu->writeDataToFile();
    delete MCU::mcu;
    outfile.close();
}

TEST_F(MCUModuleTest, WriteExceptionThrown)
{
    MCU::mcu = new MCU::MCUModule(0x01);
    createMCUProcess();
    std::string path = std::string(PROJECT_PATH) + "/backend/mcu/mcu_data.txt";
    std::ofstream outfile(path);
    chmod(path.c_str(), 0);
    EXPECT_THROW(
    {
        MCU::mcu->writeDataToFile();
    }, std::runtime_error);
    path = std::string(PROJECT_PATH) + "/backend/mcu/mcu_data.txt";
    chmod(path.c_str(), 0666);
    outfile.close();
    delete MCU::mcu;
    /* Restore the file */
    MCU::mcu = new MCU::MCUModule(0x01);
    delete MCU::mcu;
}

int main(int argc, char* argv[])
{
    socket_canbus = createSocket(1);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}