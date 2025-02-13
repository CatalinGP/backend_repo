#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <gtest/gtest.h>
#include <linux/can.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "../include/TransferData.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/Logger.h"
#include "../../../utils/include/TestUtils.h"

int socket_;
int socket2_;
const int id = 0x1011;

class TransferDataTest : public ::testing::Test {
protected:
    canid_t frame_id;
    std::vector<uint8_t> frame_data;
    Logger mockLogger;
    TransferData* transfer_data;
    CaptureFrame* captured_frame;
    TransferDataTest()
    {
        transfer_data = new TransferData(socket_, mockLogger);
        captured_frame = new CaptureFrame(socket_);
    }
    ~TransferDataTest()
    {
        delete captured_frame;
    }
};


/* Test for Incorrect Message Length */
TEST_F(TransferDataTest, IncorrectMessageLengthTest) {

    std::vector<uint8_t> invalid_frame_data = {0x02, 0x36};
    std::vector<uint8_t> expected_frame_data = {0x03, 0x7F, 0x36, 0x13};

    transfer_data->transferData(0x1011, invalid_frame_data);
    captured_frame->capture();
    for (int i = 0; i < captured_frame->frame.can_dlc; ++i) {
        EXPECT_EQ(expected_frame_data[i], captured_frame->frame.data[i]);
    }
}

/* Test for Wrong block sequence number */
TEST_F(TransferDataTest, WrongBlockSequenceNumberTest) {

    std::vector<uint8_t> invalid_frame_data = {0x02, 0x36, 0x03, 0x02};
    std::vector<uint8_t> expected_frame_data = {0x03, 0x7F, 0x36, 0x73};

    transfer_data->transferData(0x1011, invalid_frame_data);
    captured_frame->capture();
    for (int i = 0; i < captured_frame->frame.can_dlc; ++i) {
        EXPECT_EQ(expected_frame_data[i], captured_frame->frame.data[i]);
    }
}

/* Test for PositiveResponse */
TEST_F(TransferDataTest, PositiveResponseTest) {

    std::vector<uint8_t> frame_data = {0x02, 0x36, 0x01, 0x02, 0x33};
    std::vector<uint8_t> expected_frame_data = {0x02, 0x76, 0x01};

    transfer_data->transferData(0x1011, frame_data);
    captured_frame->capture();
    for (int i = 0; i < captured_frame->frame.can_dlc; ++i) {
        EXPECT_EQ(expected_frame_data[i], captured_frame->frame.data[i]);
    }
}

int main(int argc, char **argv) {
    socket_ = createSocket(0);
    socket2_ = createSocket(0);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}