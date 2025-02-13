#include "linux/can.h"
#include "unistd.h"
#include "CaptureFrame.h"

CaptureFrame::CaptureFrame(const int& socket){
    this->socket = socket;
}

void CaptureFrame::capture(){
    read(socket, &frame, sizeof(struct can_frame));
}