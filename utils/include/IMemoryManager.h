/**
 * @brief Interface of memory manager
 * @author Stefan Corneteanu
 * @date 2025-02-07
 *
 * @copyright (c) 2025
 */

#ifndef IMEMORY_MANAGER_H
#define IMEMORY_MANAGER_H

#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>

#include "Logger.h"

class IMemoryManager{
public:

    /** 
     * @brief Default dtor
     */
    virtual ~IMemoryManager() = default;
    /** 
     * @brief Call to see if address is available
     * @param address The checked address
     * @return Availability of address
     */
    virtual bool availableAddress(off_t address) = 0;
    /** 
     * @brief Call to see if memory size is available
     * @param size_of_data The size of the memory checked
     * @return Availability of memory size
     */
    virtual bool availableMemory(off_t size_of_data) = 0;
    /** 
     * @brief Call to read from address
     * @param path Path from which data is read
     * @param address_start The address from which data is read
     * @param size The size of data to be read
     * @param logger a logger that logs necessary data to console/file
     * @return The data at address
     */
    virtual std::vector<uint8_t> readFromAddress(std::string path, off_t address_start, off_t size, Logger& logger) = 0;
    /** 
     * @brief Call to get the memory manager's path
     * @return The path
     */
    virtual std::string getPath() = 0;
};

#endif