/**
 * @brief Utility class for capturing frames in the databus. Previously duplicated inside test files
 * @author Stefan Corneteanu
 * @date 2025-02-04
 *
 * @copyright (c) 2025
 */

#ifndef CAPTURE_FRAME_H
#define CAPTURE_FRAME_H

#include <linux/can.h>

struct CaptureFrame {
    int socket;
    struct can_frame frame;

    /** 
     * @brief Parametrized ctor
     * @param socket The socket from which the frames are captured
     */
    CaptureFrame(const int& socket);

    /**
     * @brief read data from the socket into the frame
     */
    void capture();
};
#endif