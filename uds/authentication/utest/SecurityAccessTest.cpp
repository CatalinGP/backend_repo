/**
 * @file SecurityAccessTest.cpp
 * @author Theodor Stoica
 * @brief Unit test for SecurityAccess Service
 * @version 0.1
 * @date 2024-07-16
 */
#include "../include/SecurityAccess.h"
#include "../../../utils/include/CaptureFrame.h"
#include "../../../utils/include/NegativeResponse.h"
#include "../../../utils/include/TestUtils.h"

#include <cstring>
#include <string>
#include <thread>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <gtest/gtest.h>
#include <net/if.h>

int socket_;
int socket2_;
const int id = 0x10FA;

std::vector<uint8_t> seed;

struct SecurityTest : testing::Test
{
    SecurityAccess* r;
    CaptureFrame* c1;
    Logger* logger;
    SecurityTest()
    {
        logger = new Logger();
        r = new SecurityAccess(socket2_, *logger);
        c1 = new CaptureFrame(socket_);
    }
    ~SecurityTest()
    {
        delete r;
        delete c1;
        delete logger;
    }
};

/* MCU is locked by default */
TEST_F(SecurityTest, DefaultMCUStateTest)
{
    EXPECT_FALSE(SecurityAccess::getMcuState(*logger));
}

TEST_F(SecurityTest, IncorrectMesssageLength)
{
    struct can_frame result_frame = createFrame(id, {0x03, 0x7F, 
                    SecurityAccess::SECURITY_ACCESS_SID, NegativeResponse::IMLOIF});

    r->securityAccess(0xFA10, {0x03, 0x27, 0x03, 0x3F, 0x01, 0x00, 0x02});
    c1->capture();
    testFrames(result_frame, *c1);
}

/* We support only sf 0x01 and 0x02*/
TEST_F(SecurityTest, SubFunctionNotSupported)
{
    struct can_frame result_frame = createFrame(id, {0x03, 0x7F, 
                    SecurityAccess::SECURITY_ACCESS_SID, NegativeResponse::SFNS});

    r->securityAccess(0xFA10, {0x06, SecurityAccess::SECURITY_ACCESS_SID, 
                    0x03, 0x3F, 0x01, 0x00, 0x02});
    c1->capture();
    testFrames(result_frame, *c1);
}

/* Send Key Request without RequestSeed */
TEST_F(SecurityTest, SendKeyBeforeRequestSeed)
{
    struct can_frame result_frame = createFrame(id, {0x03, 0x7F, 
                    SecurityAccess::SECURITY_ACCESS_SID, NegativeResponse::RSE});
    r->securityAccess(0xFA10, {0x04, 0x27, 0x02, 0xCA, 0xA5});
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(SecurityTest, RequestSeed)
{
    std::string output_security = "";
    std::string searchLine = "Seed was sent.";

    testing::internal::CaptureStdout();
    r->securityAccess(0xFA10, {0x02, 0x27, 0x01});
    output_security = testing::internal::GetCapturedStdout();

    c1->capture();
    EXPECT_TRUE(containsLine(output_security, searchLine));
    if (c1->frame.can_dlc >= 4)
    {
        seed.clear();
        /* from 3 to pci_length we have the seed generated in response */
        for (int i = 3; i <= c1->frame.data[0]; i++)
        {
            seed.push_back(c1->frame.data[i]);
        }
    }
}

TEST_F(SecurityTest, InvalidKey)
{
    struct can_frame result_frame = createFrame(id, {0x03, 0x7F, 
                    SecurityAccess::SECURITY_ACCESS_SID, NegativeResponse::IK});
    /* Send 3 wrong key */
    std::vector<uint8_t> data_frame = {static_cast<uint8_t>(seed.size() + 2), 0x27, 0x02};

    /** We now add the key to the sendKey frame
     * If we send the key == seed, the key will be wrong
    */
    data_frame.insert(data_frame.end(), seed.begin(), seed.end());
    for (int i = 0; i < 3; i++)
    {
        r->securityAccess(0xFA10, data_frame);
        c1->capture();
        testFrames(result_frame, *c1);
    }
}

TEST_F(SecurityTest, ExceededNrOfAttempts)
{
    struct can_frame result_frame = createFrame(id, {0x03, 0x7F, 
                    SecurityAccess::SECURITY_ACCESS_SID, NegativeResponse::ENOA});
    /* Send another wrong key */
    std::vector<uint8_t> data_frame = {static_cast<uint8_t>(seed.size() + 2), 0x27, 0x02};
    
    /** We now add the key to the sendKey frame
     * If we send the key == seed, the key will be wrong
    */
    data_frame.insert(data_frame.end(), seed.begin(), seed.end());
    r->securityAccess(0xFA10, data_frame);
    c1->capture();
    testFrames(result_frame, *c1);
}

TEST_F(SecurityTest, RequiredTimeDelayNotExpired)
{
    /* Send another wrong key while delay timer activated */
    std::string output_security = "";
    std::string searchLine = "Please wait";

    testing::internal::CaptureStdout();
    std::vector<uint8_t> data_frame = {static_cast<uint8_t>(seed.size() + 2), 0x27, 0x02};
    data_frame.insert(data_frame.end(), seed.begin(), seed.end());
    r->securityAccess(0xFA10, data_frame);
    output_security = testing::internal::GetCapturedStdout();

    c1->capture();
    EXPECT_TRUE(containsLine(output_security, searchLine));

    /* Wait until delay timer expires. */
    sleep(SecurityAccess::TIMEOUT_IN_SECONDS);
}

TEST_F(SecurityTest, SendCorrectKey)
{
    struct can_frame result_frame = createFrame(id, {0x02, 0x67, 0x02});
    /* Compute key from seed */
    for (auto &elem : seed)
    {
        elem = computeKey(elem);
    }
    std::vector<uint8_t> data_frame = {static_cast<uint8_t>(seed.size() + 2), 0x27, 0x02};
    data_frame.insert(data_frame.end(), seed.begin(), seed.end());
    r->securityAccess(0xFA10, data_frame);
    c1->capture();
    /* Security should be unlocked now */
    EXPECT_TRUE(SecurityAccess::getMcuState(*logger));

    testFrames(result_frame, *c1);
}

TEST_F(SecurityTest, RequestNotFromAPI)
{
    std::string output_security = "";
    std::string searchLine = "Security service can be accessed only from API.";
    testing::internal::CaptureStdout();
    r->securityAccess(0x1011, {0x02, 0x27, 0x01});
    output_security = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(containsLine(output_security, searchLine));
}

TEST_F(SecurityTest, SendKeyAfterUnlock)
{
    std::string output_security = "";
    std::string searchLine = "Server is already unlocked.";
    testing::internal::CaptureStdout();
    /* Send a key, no matter if it is valid or not. */
    std::vector<uint8_t> data_frame = {static_cast<uint8_t>(seed.size() + 2), 0x27, 0x02};
    data_frame.insert(data_frame.end(), seed.begin(), seed.end());
    r->securityAccess(0xFA10, data_frame);
    output_security = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(containsLine(output_security, searchLine));
}

TEST_F(SecurityTest, SecurityAccessExpires)
{
    sleep(3);
    /* Security expires */
    EXPECT_FALSE(SecurityAccess::getMcuState(*logger));
}

int main(int argc, char* argv[])
{
    socket_ = createSocket(1);
    socket2_ = createSocket(1);
    testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    if (socket_ > 0)
    {
        close(socket_);
    }
    if (socket2_ > 0)
    {
        close(socket2_);
    }
    return result;
}
