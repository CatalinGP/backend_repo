/**
 * @file MemoryManagerTest.cpp
 * @author Mujei Ruben
 * @brief 
 * @version 0.1
 * @date 2024-07-18
 * 
 * @copyright Copyright (c) 2024
 * This test works if a sd simulated card exists in the /dev/loop15 of 300 Mb
 */
#include "../include/MemoryManager.h"

#include <gtest/gtest.h>
#include <fstream>

off_t address = 100000 * 512;
Logger* logger = new Logger("test","test_memory.log");

/* Create object for all tests */
struct MemoryManagerTest : testing::Test
{
    MemoryManager* memory;
    MemoryManagerTest()
    {
        memory = MemoryManager::getInstance(*logger);
        memory->resetInstance();
        memory = MemoryManager::getInstance(address,"/dev/loop15", *logger);
    }
    ~MemoryManagerTest()
    {
        memory->resetInstance();
    }
};

/* Test to verify the singleton pattern of MemoryManager */
TEST_F(MemoryManagerTest, VerifySingleton)
{
    MemoryManager* m_test = MemoryManager::getInstance(*logger);
    EXPECT_EQ(memory->getAddress(), m_test->getAddress());
    EXPECT_EQ(memory->getPath(), m_test->getPath());
}

/* Test to verify setters for address and path */
TEST_F(MemoryManagerTest, VerifySetters) 
{
    off_t new_address = 230482 * 512;
    std::string new_path = "/new/path";
    memory->setAddress(new_address);
    memory->setPath(new_path);
    EXPECT_EQ(memory->getAddress(), new_address);
    EXPECT_EQ(memory->getPath(), new_path);
    /* set back the correct path */
    memory->setPath("/dev/loop15");
    memory->setAddress(address);
}

/* Test writing to and reading from an address */
TEST_F(MemoryManagerTest, WriteReadInAddress) 
{
    std::vector<uint8_t> data = {97,98,99,100};
    memory->writeToAddress(data);
    std::vector<uint8_t> data_from_address = memory->readFromAddress(memory->getPath(),address,data.size(), *logger);
    EXPECT_EQ(data, data_from_address);
}

/* Test reading from an address with an invalid address */
TEST_F(MemoryManagerTest, ReadFromAddress_ErrorTryingToRead) 
{
    off_t new_address = -1;
    testing::internal::CaptureStdout();
    std::vector<uint8_t> data_from_address = memory->readFromAddress(memory->getPath(),new_address, 5, *logger);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Error trying to read from address:"), std::string::npos);
}

/* Test writing to a file and reading from it */
TEST_F(MemoryManagerTest, WriteReadInFile) 
{
    /* Clean the old data from test_memory.txt */
    std::ofstream clear_file("test_memory.txt", std::ios::out | std::ios::trunc);
    clear_file.close();

    std::vector<uint8_t> data = {97,98,99,100};
    memory->writeToFile(data, "test_memory.txt", *logger);
    std::vector<uint8_t> data_from_address = memory->readBinary("test_memory.txt", *logger);
    EXPECT_EQ(data, data_from_address);
}

/* Test for error when attempting to read from a non-existent file */
TEST_F(MemoryManagerTest, ReadBinary_FileNotOpen) 
{
    /* Test for error when file cannot be opened */
    std::string invalid_path = "/nonexistent/path/to/file.txt";
    testing::internal::CaptureStdout();
    std::vector<uint8_t> data = memory->readBinary(invalid_path, *logger);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(data.empty());
    EXPECT_NE(output.find("Error opening SD card device"), std::string::npos);
}

/* Test writing to an address when memory allocation fails */
TEST_F(MemoryManagerTest, WriteInAddress_MemoryFail) 
{
    off_t new_address = 400000000;
    std::vector<uint8_t> data = {97,98,99,100};
    memory->setAddress(new_address);
    testing::internal::CaptureStdout();
    memory->writeToAddress(data);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Error: Not enough memory."), std::string::npos);
}

/* Test available memory when no partition is found */
TEST_F(MemoryManagerTest, AvailableMemory_NoPartition) 
{
    off_t data_size = 1000;
    memory->setPath("/path/to/nonexistent/device");

    testing::internal::CaptureStdout();
    bool result = memory->availableMemory(data_size);
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_FALSE(result);
    EXPECT_NE(output.find("No partition found"), std::string::npos);
}

/* Test writing to a file with an invalid path */
TEST_F(MemoryManagerTest, WriteInFile_WrongPathFile) 
{
    std::vector<uint8_t> data = {97, 98, 99, 100};
    std::string invalid_path = "/invalid_directory/incorrect_file.txt";
    testing::internal::CaptureStdout();
    bool result = memory->writeToFile(data, invalid_path, *logger);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_FALSE(result);
    EXPECT_NE(output.find("Error opening file"), std::string::npos);
}

/* Test writing to a read-only file */
TEST_F(MemoryManagerTest, WriteInFile_ReadOnly) 
{
    /* Create a read-only file for testing */
    std::string read_only_file = "read_only_test_memory.txt";
    std::ofstream out_file(read_only_file, std::ios::out | std::ios::trunc);
    out_file.close();
    
    /* Change the file permissions to read-only */
    /* Make it read-only */
    system(("chmod 444 " + read_only_file).c_str());

    std::vector<uint8_t> data = {97, 98, 99, 100};
    testing::internal::CaptureStdout();
    bool result = memory->writeToFile(data, read_only_file, *logger);
    std::string output = testing::internal::GetCapturedStdout();

    /* Verify that the write operation failed */
    EXPECT_FALSE(result);
    EXPECT_NE(output.find("Error opening file:"), std::string::npos);

    /* Clean up - revert permissions and delete the test file */
    system(("chmod 666 " + read_only_file).c_str());
    std::remove(read_only_file.c_str());
}

/* Main function to run all tests */
int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}