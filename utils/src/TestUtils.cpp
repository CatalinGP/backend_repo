#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include <linux/can.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "../include/CaptureFrame.h"
#include "../include/Globals.h"

bool containsLine(const std::string& output, const std::string& line){
    return output.find(line) != std::string::npos;
}

uint8_t computeKey(uint8_t& seed)
{
    return ~seed + 1;
}

void testFrames(struct can_frame expected_frame, struct can_frame captured_frame){
    EXPECT_EQ(expected_frame.can_id, captured_frame.can_id);
    EXPECT_EQ(expected_frame.can_dlc, captured_frame.can_dlc);
    for (int i = 0; i < expected_frame.can_dlc; ++i) {
        EXPECT_EQ(expected_frame.data[i], captured_frame.data[i]);
    }
}

void testFrames(struct can_frame expected_frame, CaptureFrame captured_frame){
    testFrames(expected_frame, captured_frame.frame);
}

struct can_frame createFrame(int id, std::vector<uint8_t> data, FrameType frameType)
{
    struct can_frame frame;
    switch (frameType)
    {
        case ERROR_FRAME:
        /* Handle ERROR_FRAME */
        break;
        case OVERLOAD_FRAME:
        /* Handle OVERLOAD_FRAME */
        break;
        case DATA_FRAME:
            frame.can_id = (id & CAN_EFF_MASK) | CAN_EFF_FLAG;
            frame.can_dlc = data.size();
            for (uint8_t byte = 0; byte < frame.can_dlc; byte++)
            {
                frame.data[byte] = data[byte];
            }
            break;
        case REMOTE_FRAME:
            frame.can_id = (id & CAN_EFF_MASK) | CAN_EFF_FLAG;
            frame.can_id |= CAN_RTR_FLAG;
            frame.can_dlc = data.size();
            for (uint8_t byte = 0; byte < frame.can_dlc; byte++)
            {
                frame.data[byte] = data[byte];
            }
            break;
    }
    return frame;
}

int createSocket(uint8_t interface_number) {

    /* Create socket */
    unsigned char lowerbits = interface_number & 0X0F;
    std::string name_interface = "vcan" + std::to_string(lowerbits);
    struct sockaddr_can addr;
    struct ifreq ifr;
    int s;

    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0)
    {
        std::cout<<"Error trying to create the socket\n";
        return 1;
    }
    /* Giving name and index to the interface created */
    strcpy(ifr.ifr_name, name_interface.c_str() );
    ioctl(s, SIOCGIFINDEX, &ifr);
    /* Set addr structure with info. of the CAN interface */
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    /* Bind the socket to the CAN interface */
    int b = bind(s, (struct sockaddr*)&addr, sizeof(addr));
    if( b < 0 )
    {
        std::cout<<"Error binding\n";
        return 1;
    }
    int flags = fcntl(s, F_GETFL, 0);
    if (flags == -1)
    {
        return 1;
    }
    /* Set the O_NONBLOCK flag to make the socket non-blocking */
    flags |= O_NONBLOCK;
    if (fcntl(s, F_SETFL, flags) == -1)
    {
        return -1;
    }
    return s;
}