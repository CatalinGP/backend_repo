/**
 * @file ReadMemoryByAddress.h
 * @brief This file contains the declaration of the ReadMemoryByAddress class.
 * @version 0.1
 * @date 2024-08-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef READ_MEMORY_BY_ADDRESS_H
#define READ_MEMORY_BY_ADDRESS_H

#include <vector>
#include "Logger.h"
#include "MemoryManager.h"
#include "GenerateFrames.h"
#include "NegativeResponse.h"
#include "SecurityAccess.h"

class ReadMemoryByAddress {
private:
    MemoryManager* memoryManager;
    GenerateFrames& frameGenerator;
    int socket;
    Logger& logger;

public:
    /**
     * @brief Construct a new ReadMemoryByAddress object
     * 
     * @param memManager Instance of MemoryManager
     * @param frameGen Instance of GenerateFrames
     * @param socket Socket to send frames
     * @param log Instance of Logger
     */
    ReadMemoryByAddress(MemoryManager* memManager, GenerateFrames& frameGen, int socket,  Logger& log);

    /**
     * @brief Method to handle a Read Memory By Address request
     * 
     * @param can_id CAN ID for the frame
     * @param memory_address Start address to read from
     * @param memory_size Number of bytes to read
     */
    void handleRequest(int can_id, off_t memory_address, off_t memory_size);
};

#endif // READ_MEMORY_BY_ADDRESS_H