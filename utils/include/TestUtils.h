/**
 * @brief File containing functions used in multiple tests.
 * If any function is ever suitable for production, move it to Globals
 * @author Stefan Corneteanu
 * @date 2025-02-05
 *
 * @copyright (c) 2025
 */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <cstdint>
#include <linux/can.h>
#include <memory>
#include <string>
#include <vector>

#include "CaptureFrame.h"
#include "Globals.h"
#include "../../uds/authentication/include/SecurityAccess.h"

/**
 * @brief Check if a line is inside of an output
 * @param output The output string to be investigated
 * @param line The line to be looked up
 * @return true if the line is inside false otherwise
 */
bool containsLine(const std::string& output, const std::string& line);

/**
 * @brief Compute the two's complement of a seed to be used as a key
 * @param seed The seed used for computing the key
 * @return The computed key
 */
uint8_t computeKey(uint8_t& seed);

/**
 * @brief Check if a captured frame is equal to the expected frame (same id, data and dlc)
 * @param expected_frame The expected CAN frame
 * @param captured_frame The frame to be tested
 */
void testFrames(struct can_frame expected_frame, struct can_frame captured_frame);

/**
 * @brief Check if a captured frame is equal to the expected frame (same id, data and dlc)
 * @param expected_frame The expected CAN frame
 * @param captured_frame The frame to be tested
 */
void testFrames(struct can_frame expected_frame, CaptureFrame captured_frame);

/**
 * @brief Create a Frame object
 * 
 * @param id if of the frame
 * @param data data to be put in the frame
 * @param frameType type of frame
 * @return struct can_frame 
 */
struct can_frame createFrame(int id, std::vector<uint8_t> data, FrameType frameType = DATA_FRAME);

/**
 * @brief Create the socket
 * 
 * @warning Before running tests containing this function, do the following:
 * 1) sudo modprobe vcan (loads the virtual can kernel module)
 * 2) sudo ip link add type vcan name vcan<interface_number> (@see @param interface_number)
 * 3) ip link show(verify interfaces, make sure that vcan<interface_number> is listed)
 * 4) sudo ip link set vcan<interface_number> up (set up the interface before communication)
 * @param interface_number The interface indicator number.
 * @return Returns the socket file descriptor.
 */
int createSocket(uint8_t interface_number);

/**
 * @brief request security access 
 * @param securityAccess a security access object used for requesting the seed
 * @param capturedFrame a capture frame object used to capture the security access seed
 * @param sid the id of the service for which we request access
 */
void v_requestSecurityAccess(std::shared_ptr<SecurityAccess> spSecurityAccess, std::shared_ptr<CaptureFrame> spCapturedFrame, uint8_t sid);
#endif