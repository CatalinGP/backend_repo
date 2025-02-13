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
#include <string>
#include <vector>

#include "CaptureFrame.h"
#include "Globals.h"

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
 * @param interface_number The interface indicator number.
 * @return Returns the socket file descriptor.
 */
int createSocket(uint8_t interface_number);

#endif