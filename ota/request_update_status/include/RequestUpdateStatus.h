/**
 * @file RequestUpdateStatus.h
 * @author Iancu Daniel
 * @brief ***********************MCU Service. Requested from API***********************
 * 			Request format: PCI_L(1byte) | SID(1byte)
 * 			Response format(positive): PCI_L(1byte) | SSID(1byte) | OTA_DID_MSB(1byte) | OTA_DID_LSB(1byte) | OTA_UPDATE_STATUS(1byte)
 * 			Response format(negative): pci_L(1byte) | NEGATIVE_RESPONSE(1byte) | SID(1byte) | NEGATIVE_RESPONSE_CODE(1byte)
 * 
 * 			The service uses ReadDataByIdentifier to get the value from Ota update status data identifier.
 * 			ReadDataByIdentifier directly returns the response back to RequestUpdateStatus, without sending it on the can-bus.
 * 			The OTA_UPDATE_STATUS_DID is in MCU memory.
 * 
 * 			The status is sent back to API from MCU.
 * @version 0.1
 * @date 2024-06-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef REQUEST_UPDATE_STATUS_H
#define REQUEST_UPDATE_STATUS_H

#include <iostream>
#include <vector>
#include <linux/can.h>
#include <map>
#include "Logger.h"
#include "NegativeResponse.h"
#include "ReadDataByIdentifier.h"
#include "GenerateFrames.h"

#define REQUEST_UPDATE_STATUS_REQUEST_SIZE      0x02
#define REQUEST_UPDATE_STATUS_RESPONSE_SUCCESS_SIZE	    0x03
#define REQUEST_UPDATE_STATUS_RESPONSE_NEGATIVE_SIZE	0x04

#define OTA_UPDATE_STATUS_DID 0xE001
#define OTA_UPDATE_STATUS_DID_MSB ((OTA_UPDATE_STATUS_DID & 0xFF00) >> 8)
#define OTA_UPDATE_STATUS_DID_LSB (OTA_UPDATE_STATUS_DID & 0x00FF)

#define REQUEST_UPDATE_STATUS_SID           0x32
#define REQUEST_UPDATE_STATUS_SID_SUCCESS   0x72

#define READ_DATA_BY_IDENTIFIER_SID 0x22
#define READ_DATA_BY_IDENTIFIER_SID_SUCCESS 0x62

#define WRITE_DATA_BY_IDENTIFIER_SID 0x2A

#define PCI_L 0x03
#define NEGATIVE_RESPONSE 0x7F
#define REQUEST_OUT_OF_RANGE 0x31

#define MCU_ID ((uint8_t)0x10)
#define API_ID 0xFA

/**
 * @brief Macro used for setting byte number i from n to byte b.
 * 	n: FFEEDDCC
 * 	i: 3 2 1 0
 */
#define SET_BYTE(n, i, b) (n = ((n & ~(0xFF << (i * 8))) | ((b & 0xFF) << (i * 8))))
/**
 * @brief Macro used for getting the byte number i from n.
 *  n: FFEEDDCC
 * 	i: 3 2 1 0
 */
#define GET_BYTE(n, i) ((n >> (i * 8)) & 0xFF)

/**
 * @brief Enumeration with the Ota Update possible states. 
 * 	MSN(nibble) represents the general state, LSN(nibble) represents the intermediary state, while a download/transfer/install is in progress.
 * 
 */
typedef enum OtaUpdateStatesEnum
{
	IDLE = 0x00,        /* Initial state of the FOTA Handler after the ECU startup procedure */
    INIT = 0x10,        /* The FOTA Handler is initialized, and Dcm is set into the correct state (in Dcm FOTA session and security access has been granted) */
	WAIT = 0x20,        /* The FOTA Handler has successfully processed the last received data chunk, returned the Dcm callout function, and is waiting for the next data chunk */
    WAIT_DOWNLOAD_COMPLETED = 0x21,        /* The FOTA Handler has successfully processed the last received data chunk, returned the Dcm callout function, and is waiting for the next data chunk */
    WAIT_DOWNLOAD_FAILED = 0x22,        /* The FOTA Handler has successfully processed the last received data chunk, returned the Dcm callout function, and is waiting for the next data chunk */	
   	PROCESSING = 0x30,  /* The FOTA Handler is triggered by the Dcm callout since a new chunk has been received and is processed in the callout context */
   	PROCESSING_TRANSFER_COMPLETE = 0x31,  /* The FOTA Handler is triggered by the Dcm callout since a new chunk has been received and is processed in the callout context */
   	PROCESSING_TRANSFER_FAILED = 0x32,  /* The FOTA Handler is triggered by the Dcm callout since a new chunk has been received and is processed in the callout context */
   	READY = 0x40,       /* All FOTA data chunks have been installed, activation procedure can be triggered */
	VERIFY = 0x50,      /* Optional and implementer specific step, since the FOTA Target does not specify  any details on the verification process */
	VERIFY_COMPLETE = 0x51,      /* Optional and implementer specific step, since the FOTA Target does not specify  any details on the verification process */
	VERIFY_FAILED = 0x52,      /* Optional and implementer specific step, since the FOTA Target does not specify  any details on the verification process */
	ACTIVATE = 0x60,    /* FOTA installation has finished and received a respective service job from the FOTA Master that indicates the partition switch during the next boot process */
    ACTIVATE_INSTALL_COMPLETE = 0x61,    /* FOTA installation has finished and received a respective service job from the FOTA Master that indicates the partition switch during the next boot process */
    ACTIVATE_INSTALL_FAILED = 0x62,    /* FOTA installation has finished and received a respective service job from the FOTA Master that indicates the partition switch during the next boot process */
	ERROR = 0xFF       /* Optional and implementer specific. Reserved state for e.g., implementer specific error handling, which is not (yet) covered by the FOTA Target */
}OtaUpdateStates;

class RequestUpdateStatus
{
private:
	Logger rus_logger;
public:
	int socket = -1;
	Logger& _logger;
	RequestUpdateStatus(int socket, Logger& logger);
	/**
	 * @brief Service method. Receive a request for reading Ota Update Status.
	 * 	Sends a ReadDataByIdentifier request to MCU, with the Ota Update Status Data Identifier.
	 * 	Gets the response from the service directly through return, not from can-bus.
	 * 	Sends the response back to the initial sender.
	 * 
	 * @param[in] frame_id The id of the received frame. 
	 * @param[in] frame_data Data of the frame.
	 * @return std::vector<uint8_t>
	 */
	std::vector<uint8_t> requestUpdateStatus(canid_t frame_id, std::vector<uint8_t> frame_data);

	/**
	 * @brief Method used for checking if a 8 bit number represents a valid OTA status.
	 * 
	 * @param[in] status Represents the current OTA update status.
	 * @return true 
	 * @return false 
	 */
	bool isValidStatus(uint8_t status);

	~RequestUpdateStatus();
};

#endif /* REQUEST_UPDATE_STATUS_H */