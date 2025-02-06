/**
 * @brief Pure mock class of memory manager to be used in unit tests
 * @author Stefan Corneteanu
 * @date 2025-02-06
 *
 * @copyright (c) 2025
 */

#ifndef MOCK_MEMORY_MANAGER_H
#define MOCK_MEMORY_MANAGER_H

#include <cstdint>
#include <string>

#include <gmock/gmock.h>
#include <gmock/gmock-function-mocker.h>
#include <sys/types.h>

#include "Logger.h"
#include "IMemoryManager.h"

class MockMemoryManager : public IMemoryManager {
public:

    ~MockMemoryManager() override = default;
    MOCK_METHOD(bool, availableAddress, (off_t address));
    MOCK_METHOD(bool, availableMemory, (off_t size_of_data));
    MOCK_METHOD(std::vector<uint8_t>, readFromAddress, (std::string path, off_t address_start, off_t size, Logger& logger));
    MOCK_METHOD(std::string, getPath, ());
};

#endif