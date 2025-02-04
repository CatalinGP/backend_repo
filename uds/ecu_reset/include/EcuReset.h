/**
 * @file ecu_reset.h
 * @author Daniel Constantinescu
 * @brief Logic for ECU Reset service.
 * This class offer the needed methods for using the ECU Reset service.
 * It allows either a Hard Reset or a Key Off Reset
 */

#ifndef ECU_RESET_H
#define ECU_RESET_H

#include <iostream>

#include "Logger.h"

class EcuReset
{
protected:
    uint32_t can_id;
    uint8_t sub_function;
    int socket = -1;
    
    Logger& ECUResetLog;
public:
    /**
     * @brief Parameterized constructor.
     * 
     * @param can_id CAN identifier
     * @param sub_function Subfunction for ECU Reset
     * @param socket Socket to be used for response
     * @param logger The logger
     */
    EcuReset(uint32_t can_id, uint8_t sub_function, int socket, Logger &logger);

    /**
     * @brief Destroy the Ecu Reset object
     * 
     */
    virtual ~EcuReset();
    /**
     * @brief Method that checks the subfunction and calls either hardReset() or keyOffReset().
     * data A vector containing the data bytes to be processed.
     */
    void ecuResetRequest(const std::vector<uint8_t>& data); 

    /**
     * @brief Method that does the Hard Reset.
     * 
     */
    virtual void hardReset();

    /**
     * @brief Method that does the Key Off Reset.
     * 
     */
    void keyOffReset();

    /**
     * @brief Method that sends the response on the given socket.
     * 
     */
    void ecuResetResponse();
};

#endif /* ECU_RESET_H */ 