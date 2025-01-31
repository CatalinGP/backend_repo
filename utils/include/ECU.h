/**
 * @file ECU.h
 * @author Iancu Daniel
 * @brief 
 * @version 0.1
 * @date 2024-08-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ECU_H
#define ECU_H

#include <iostream>
#include <future>
#include <map>

#include "Logger.h"
#include "CreateInterface.h"
#include "ReceiveFrames.h"

#define ECU_INTERFACE_NUMBER 0x00
#define SYSTEM_SUPPLIER_ECU_SOFTWARE_VERSION_NUMBER_DID 0xF1A2

class ECU
{
public:

    int _ecu_socket = -1;
    uint8_t _module_id;
    ReceiveFrames *_frame_receiver = nullptr;
    CreateInterface *_can_interface;
    Logger& _logger;

    /* Static dictionary to store SID and processing time */
    static std::map<uint8_t, double> timing_parameters;
    /* Store active timers for SIDs */
    static std::map<uint8_t, std::future<void>> active_timers;
    /* Stop flags for each SID. */
    static std::map<uint8_t, std::atomic<bool>> stop_flags;

    /**
     * @brief Construct a new ECU object
     * 
     * @param module_id Custom module identifier.
     * @param logger A logger instance used to record information and errors during the execution.
     */
    ECU(uint8_t module_id, Logger& logger);

    /**
     * @brief Function to notify MCU if the module is Up & Running.
     * 
     */
    void sendNotificationToMCU();

    /**
     * @brief Function that starts the frame receiver.
     * 
     */
    void startFrames();

    /**
     * @brief Function that stops the frame receiver.
     * 
     */
    void stopFrames();

    /**
     * @brief Method that checks if there has been a change to the software version. It checks the current software version with the previous one.
     *  The previous sw version is saved in memory
     * 
     */
    void checkSwVersion();
    /**
     * @brief Destroy the ECU object
     * 
     */
    ~ECU();

    void stopProcess(std::string process_name);
};

#endif