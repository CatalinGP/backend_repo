/**
 * @brief Pure mock class of GenerateFrames to be used in unit tests
 * @author Stefan Corneteanu
 * @date 2025-02-07
 *
 * @copyright (c) 2025
 */

#ifndef MOCK_GENERATE_FRAMES_H
#define MOCK_GENERATE_FRAMES_H

#include <gmock/gmock-function-mocker.h>
#include <gmock/gmock.h>

#include "GenerateFrames.h"
#include "Logger.h"

class MockGenerateFrames: public GenerateFrames {
public:

    /**
     * @brief Super ctor
     */
    MockGenerateFrames(Logger &logger)
    : GenerateFrames(logger) {}

    /**
     * @brief Default dtor
     */
    ~MockGenerateFrames() override = default;

    MOCK_METHOD(void, readMemoryByAddress, (int id, int memory_address, int memory_size, std::vector<uint8_t> response));
    MOCK_METHOD(void, readMemoryByAddressLongResponse, (int id, int memory_address, int memory_size, std::vector<uint8_t> response, bool first_frame));
};

#endif