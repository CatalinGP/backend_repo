#include <cstdint>
#include <gmock/gmock-cardinalities.h>
#include <gmock/gmock-spec-builders.h>
#include <gtest/gtest.h>
#include <linux/can.h>
#include <memory>
#include <sys/types.h>
#include <vector>

#include "../include/ReadMemoryByAddress.h"
#include "../../authentication/include/SecurityAccess.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/GenerateFrames.h"
#include "../../../utils/include/Logger.h"
#include "../../../utils/include/MemoryManager.h"
#include "../../../utils/include/MockGenerateFrames.h"
#include "../../../utils/include/MockMemoryManager.h"
#include "../../../utils/include/NegativeResponse.h"
#include "../../../utils/include/TestUtils.h"

using namespace testing;

const int kFrameId = 0x01;
const int kCanId = 0x01;
const int kVCanInterface = 0x01;

struct ReadMemoryByAddressTest : Test {

    ReadMemoryByAddressTest() {
        socket = createSocket(kVCanInterface);
        socket2 = createSocket(kVCanInterface);
        spCapturedFrame = std::make_shared<CaptureFrame>(socket2);
        pMockMemoryManager = new MockMemoryManager();
        spMockGenerateFrames =  std::make_shared<MockGenerateFrames>(logger);
        readMemoryByAddress = std::make_unique<ReadMemoryByAddress>(pMockMemoryManager, *spMockGenerateFrames, socket, logger);
        spSecurityAccess = std::make_shared<SecurityAccess>(socket, logger);
    }

    ~ReadMemoryByAddressTest(){
        delete pMockMemoryManager;
        close(socket);
    }

    std::shared_ptr<CaptureFrame> spCapturedFrame;
    std::shared_ptr<SecurityAccess> spSecurityAccess;
    int socket, socket2;
    Logger logger;
    MockMemoryManager* pMockMemoryManager;
    std::shared_ptr<MockGenerateFrames> spMockGenerateFrames;
    std::unique_ptr<ReadMemoryByAddress> readMemoryByAddress;
};

TEST_F(ReadMemoryByAddressTest, InvalidMemoryAddress){
    struct can_frame expectedFrame = createFrame(kFrameId, {0x03, 0x7F, 0x23, NegativeResponse::IMLOIF});

    // One call to availableAddress may occur
    EXPECT_CALL(*pMockMemoryManager, availableAddress).Times(AtMost(1)).WillOnce(Return(false));
    // One call to availableMemory may occur
    EXPECT_CALL(*pMockMemoryManager, availableMemory).Times(AtMost(1)).WillOnce(Return(false));
    // Do not expect calls from readFromAddress method
    EXPECT_CALL(*pMockMemoryManager, readFromAddress(_,_,_,_)).Times(0);

    // Expect no calls from mock generate frames
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddress(_,_,_,_)).Times(0);
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddressLongResponse(_,_,_,_,_)).Times(0);

    // Request with a negative (invalid) memory address)
    readMemoryByAddress->handleRequest(kCanId, -1, 1);
    spCapturedFrame->capture(); //IMLOIF from invalid memory address
    testFrames(expectedFrame, *spCapturedFrame);
    spCapturedFrame->capture(); //IMLOIF from unavailable memory address
    testFrames(expectedFrame, *spCapturedFrame);
}

TEST_F(ReadMemoryByAddressTest, InvalidMemorySize){
    struct can_frame expectedFrame = createFrame(kFrameId, {0x03, 0x7F, 0x23, NegativeResponse::IMLOIF});

    // One call to availableAddress may occur
    EXPECT_CALL(*pMockMemoryManager, availableAddress).Times(AtMost(1)).WillOnce(Return(false));
    // One call to availableMemory may occur
    EXPECT_CALL(*pMockMemoryManager, availableMemory).Times(AtMost(1)).WillOnce(Return(false));
    // Do not expect calls from readFromAddress method
    EXPECT_CALL(*pMockMemoryManager, readFromAddress(_,_,_,_)).Times(0);

    // Expect no calls from mock generate frames
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddress(_,_,_,_)).Times(0);
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddressLongResponse(_,_,_,_,_)).Times(0);

    // Request with a negative (invalid) memory size)
    readMemoryByAddress->handleRequest(kCanId, 1, -1);
    spCapturedFrame->capture(); //IMLOIF from invalid memory size
    testFrames(expectedFrame, *spCapturedFrame);
    spCapturedFrame->capture(); //IMLOIF from unavailable memory size
    testFrames(expectedFrame, *spCapturedFrame);
}

TEST_F(ReadMemoryByAddressTest, UnavailableMemoryAddress){
    struct can_frame expectedFrame = createFrame(kFrameId, {0x03, 0x7F, 0x23, NegativeResponse::IMLOIF});

    // Expect failure of availableAddress
    EXPECT_CALL(*pMockMemoryManager, availableAddress).Times(AtMost(1)).WillOnce(Return(false));
    // One call to availableMemory may occur
    EXPECT_CALL(*pMockMemoryManager, availableMemory).Times(AtMost(1));
    // Do not expect calls from readFromAddress method
    EXPECT_CALL(*pMockMemoryManager, readFromAddress(_,_,_,_)).Times(0);

    // Expect no calls from mock generate frames
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddress(_,_,_,_)).Times(0);
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddressLongResponse(_,_,_,_,_)).Times(0);

    readMemoryByAddress->handleRequest(kCanId, 1, 1);
    spCapturedFrame->capture();
    testFrames(expectedFrame, *spCapturedFrame);
}

TEST_F(ReadMemoryByAddressTest, UnavailableMemorySize){
    struct can_frame expectedFrame = createFrame(kFrameId, {0x03, 0x7F, 0x23, NegativeResponse::IMLOIF});

    // Expect failure of availableMemory
    EXPECT_CALL(*pMockMemoryManager, availableMemory).Times(AtMost(1)).WillOnce(Return(false));
    // One call to availableAddress may occur
    EXPECT_CALL(*pMockMemoryManager, availableAddress).Times(AtMost(1));
    // Do not expect calls from readFromAddress method
    EXPECT_CALL(*pMockMemoryManager, readFromAddress(_,_,_,_)).Times(0);

    // Expect no calls from mock generate frames
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddress(_,_,_,_)).Times(0);
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddressLongResponse(_,_,_,_,_)).Times(0);

    readMemoryByAddress->handleRequest(kCanId, 1, 1);
    spCapturedFrame->capture();
    testFrames(expectedFrame, *spCapturedFrame);
}

TEST_F(ReadMemoryByAddressTest, SecurityAccessDenied){
    struct can_frame expectedFrame = createFrame(kFrameId, {0x03, 0x7F, 0x23, NegativeResponse::SAD});

    // Expect pass of both availableAddress availableMemory
    EXPECT_CALL(*pMockMemoryManager, availableAddress).WillOnce(Return(true));
    EXPECT_CALL(*pMockMemoryManager, availableMemory).WillOnce(Return(true));
    // Do not expect calls from readFromAddress method
    EXPECT_CALL(*pMockMemoryManager, readFromAddress(_,_,_,_)).Times(0);

    // Expect no calls from mock generate frames
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddress(_,_,_,_)).Times(0);
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddressLongResponse(_,_,_,_,_)).Times(0);

    // handleRequest will return as mcu_state is false
    readMemoryByAddress->handleRequest(kCanId, 1, 1);
    spCapturedFrame->capture();
    testFrames(expectedFrame, *spCapturedFrame);
}

TEST_F(ReadMemoryByAddressTest, ReadMemoryFailure){
    struct can_frame expectedFrame = createFrame(kFrameId, {0x03, 0x7F, 0x23, NegativeResponse::ROOR});

    // Expect pass of both availableAddress availableMemory
    EXPECT_CALL(*pMockMemoryManager, availableAddress).WillOnce(Return(true));
    EXPECT_CALL(*pMockMemoryManager, availableMemory).WillOnce(Return(true));
    // Expect call of readFromAddress, return nothing leading to read memory failure
    EXPECT_CALL(*pMockMemoryManager, readFromAddress(_,_,_,_))
    .WillOnce(Return(std::vector<uint8_t>{}));
    // readFromAddress will call getPath
    EXPECT_CALL(*pMockMemoryManager, getPath());

    // Expect no calls from mock generate frames
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddress(_,_,_,_)).Times(0);
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddressLongResponse(_,_,_,_,_)).Times(0);

    // grant security access
    v_requestSecurityAccess(spSecurityAccess, spCapturedFrame, 0x23);

    // handleRequest will return due to failure to read
    readMemoryByAddress->handleRequest(kCanId, 1, 1);
    spCapturedFrame->capture(); // This capture will get the frame generated by securityAccessSendKey
    spCapturedFrame->capture(); // This capture will get the Negative Response frame
    testFrames(expectedFrame, *spCapturedFrame);
}

TEST_F(ReadMemoryByAddressTest, Response){
    auto expectedResponse = std::vector<uint8_t>{0x01,0x02,0x03};
    // Expect pass of both availableAddress availableMemory
    EXPECT_CALL(*pMockMemoryManager, availableAddress).WillOnce(Return(true));
    EXPECT_CALL(*pMockMemoryManager, availableMemory).WillOnce(Return(true));
    // Expect call of readFromAddress, return less than 6 bytes leading to normal response
    EXPECT_CALL(*pMockMemoryManager, readFromAddress(_,_,_,_))
    .WillOnce(Return(expectedResponse));
    // readFromAddress will call getPath
    EXPECT_CALL(*pMockMemoryManager, getPath());

    // Expect a call from readMemoryByAddress
    EXPECT_CALL(*spMockGenerateFrames, 
        readMemoryByAddress(
            kCanId, 1, 1, expectedResponse
        )
    );
    // Expect no long response
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddressLongResponse(_,_,_,_,_)).Times(0);

    // grant security access
    v_requestSecurityAccess(spSecurityAccess, spCapturedFrame, 0x23);

    // handleRequest will lead to normal response
    readMemoryByAddress->handleRequest(kCanId, 1, 1);
}

TEST_F(ReadMemoryByAddressTest, LongResponse){
    auto expectedResponse = std::vector<uint8_t>{0x01,0x02,0x03,0x04,0x05,0x06, 0x07};
    // Expect pass of both availableAddress availableMemory
    EXPECT_CALL(*pMockMemoryManager, availableAddress).WillOnce(Return(true));
    EXPECT_CALL(*pMockMemoryManager, availableMemory).WillOnce(Return(true));
    // Expect call of readFromAddress, return less than 6 bytes leading to long response
    EXPECT_CALL(*pMockMemoryManager, readFromAddress(_,_,_,_))
    .WillOnce(Return(expectedResponse));
    // readFromAddress will call getPath
    EXPECT_CALL(*pMockMemoryManager, getPath());

    // Expect no normal response
    EXPECT_CALL(*spMockGenerateFrames, readMemoryByAddress(_,_,_,_)).Times(0);
    // Expect 2 calls to normal response
    InSequence longResponseCallsSequence;
    EXPECT_CALL(*spMockGenerateFrames, 
        readMemoryByAddressLongResponse(
            kCanId, 1, 1, expectedResponse, true
        )
    );
    EXPECT_CALL(*spMockGenerateFrames, 
        readMemoryByAddressLongResponse(
            kCanId, 1, 1, expectedResponse, false
        )
    );

    // grant security access
    v_requestSecurityAccess(spSecurityAccess, spCapturedFrame, 0x23);

    // handleRequest will lead to long response
    readMemoryByAddress->handleRequest(kCanId, 1, 1);
}

int main(int argc, char **argv)
{
testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}