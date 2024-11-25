/**
 * @file WriteDataByIdentifier.h
 * @author Dirva Nicolae
 * @brief This library represents the WriteDataByIdentifier UDS service.
 * It writes data based on the Data Identifier (DID) received in the CAN frame.
 * For example, if you want to update the battery voltage, 0x01B0 DID is responsible for that.
 * The request frame received by the service will have the format: frame.data = {PCI_L(1byte), SID(1byte = 0x2E), DID(2bytes), DATA}
 * The positive response frame sent by the service will have the format: frame.data = {PCI_L(1byte), RESPONSE_SID(1byte = 0x6E), DID(2bytes)}
 * The negative response frame sent by the service will have the format: frame.data = {PCI_L(1byte), 0x7F, SID(1byte = 0x2E), NRC(1byte)}
 */
#ifndef UDS_WDBI_SERVICE
#define UDS_WDBI_SERVICE

#include <iostream>
#include <vector>

#include <linux/can.h>


#include "GenerateFrames.h"
#include "Logger.h"


class WriteDataByIdentifier
{
private:
    GenerateFrames generate_frames;
    int socket = -1;
    Logger& wdbi_logger;

public:
    /* Define the service identifier for WriteDataByIdentifier*/
    static constexpr uint8_t WDBI_SID = 0x2E;
    /**
     * @brief Construct a new Write Data By Identifier object
     * 
     * @param socket The socket descriptor used for communication over the CAN bus.
     * @param wdbi_logger A logger instance used to record information and errors during the execution.
     */
    WriteDataByIdentifier(Logger& wdbi_logger, int socket);
    /**
     * @brief Destroy the Write Data By Identifier object
     * 
     */
    ~WriteDataByIdentifier();
    /**
     * @brief Execute the WriteDataByIdentifier service.
     * 
     * This function performs the WriteDataByIdentifier service, writing the received data to the specified
     * Data Identifier (DID) and sending the appropriate response frame or a negative response if an error occurs.
     * 
     * @param frame_id The frame id.
     * @param frame_data Data of the frame.
     */
    void WriteDataByIdentifierService(canid_t frame_id, std::vector<uint8_t> frame_data);
};

#endif