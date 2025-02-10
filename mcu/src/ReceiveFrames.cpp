#include <algorithm>
#include <unistd.h>

#include "ReceiveFrames.h"
#include "MCUModule.h"
#include "MCULogger.h"
#include "SecurityAccess.h"
#include "TransferData.h"
#include "AccessTimingParameter.h"
#include "NegativeResponse.h"
namespace MCU
{
    ReceiveFrames::ReceiveFrames(int iSocketCanBus, int iSocketApi)
        : timeout_duration(120), running(true), iSocketCanBus(iSocketCanBus), 
        iSocketApi(iSocketApi), handler(iSocketApi, *MCULogger),
        generate_frames(iSocketCanBus, *MCULogger)
        
    {
        startTimerThread();
    }

    ReceiveFrames::~ReceiveFrames() 
    {
        vStopListenAPI();
        vStopListenCANBus();
        stopTimerThread();
    }

    uint32_t ReceiveFrames::gethexValueId()
    {
        return hex_value_id;
    }

    /**
     * Function to read frames from the CAN bus and add them to a queue.
     * This function runs in a loop and continually reads frames from the CAN bus.
     */
  bool ReceiveFrames::receiveFramesFromCANBus()
{
    struct can_frame frame;
    while (listen_canbus)
    {
        /* Read frames from the CAN socket */
        int nbytes = read(iSocketCanBus, &frame, sizeof(frame));
        
        if (nbytes < 0) 
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN) 
            {
                /* No data available, continue the loop */
                continue;
            } 
            else 
            {
                LOG_ERROR(MCULogger->GET_LOGGER(), "Read error on CANBus socket: {}", strerror(errno));
                return false;
            }
        } 
        else if (nbytes == 0) 
        {
            /* Connection closed */
            LOG_INFO(MCULogger->GET_LOGGER(), "CANBus connection closed.");
            return false;
        } 
        else 
        {
            LOG_DEBUG(MCULogger->GET_LOGGER(), "Captured a frame on the CANBus socket");
            /* Lock the queue before adding the frame to ensure thread safety */
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                uint8_t receiver_id = frame.can_id & 0xFF;

                /* If frame is for MCU module, for API or test frame */
                if (receiver_id == hex_value_id || receiver_id == 0xFF || receiver_id == 0xFA) 
                {
                    frame_queue.push(frame);
                    LOG_DEBUG(MCULogger->GET_LOGGER(), fmt::format("Passed a valid Module ID: 0x{:x} and frame added to the processing queue.", frame.can_id));
                }
            }
            /* Notify one waiting thread that a new frame has been added to the queue */
            queue_cond_var.notify_one();
        }
    }
    return true;
}

bool ReceiveFrames::bReceiveFramesFromAPI()
{
    struct can_frame frame;
    while (listen_api)
    {
        /* Read frames from the CAN socket */
        int nbytes = read(iSocketApi, &frame, sizeof(frame));
        
        if (nbytes < 0) 
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN) 
            {
                /* No data available, continue the loop */
                continue;
            } 
            else 
            {
                LOG_ERROR(MCULogger->GET_LOGGER(), "Read error on API socket: {}", strerror(errno));
                return false;
            }
        } 
        else if (nbytes == 0) 
        {
            /* Connection closed */
            LOG_INFO(MCULogger->GET_LOGGER(), "API connection closed.");
            return false;
        } 
        else 
        {
            LOG_DEBUG(MCULogger->GET_LOGGER(), "Captured a frame on the API socket");
            /* Lock the queue before adding the frame to ensure thread safety */
            {
                {
                    /* Lock the queue before adding the frame to ensure thread safety */
                    std::lock_guard<std::mutex> lock(queue_mutex);

                    /* Take receiver_id */
                    uint8_t receiver_id = frame.can_id & 0xFF;

                    /* If frame is for MCU module, for API or test frame */
                    if( receiver_id != 0xFA)
                    {
                        frame_queue.push(frame);
                        LOG_DEBUG(MCULogger->GET_LOGGER(), fmt::format("Pass a valid Module ID: 0x{:x} and frame added to the processing queue.", frame.can_id));
                    }
                }
                /* Notify one waiting thread that a new frame has been added to the queue */
                queue_cond_var.notify_one();
            }
            /* Notify one waiting thread that a new frame has been added to the queue */
            queue_cond_var.notify_one();
        }
    }
    return true;
}

    /*
    * Function to process frames from the queue.
    * This function runs in a loop and processes each frame from the queue.
    */
    void ReceiveFrames::vProcessQueue() 
    {
        LOG_DEBUG(MCULogger->GET_LOGGER(),"Frame processing method invoked!");
        while (true)
        {
            /* Wait until the queue is not empty, then lock the queue */
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cond_var.wait(lock, [this]{ return !frame_queue.empty()|| !process_queue; });
            if (!process_queue && frame_queue.empty()) 
            {
                break;
            }
            /* Extract the first element from the queue */
            struct can_frame frame = frame_queue.front();
            frame_queue.pop();
            LOG_DEBUG(MCULogger->GET_LOGGER(), fmt::format("Frame with ID: 0x{:x} is taken from processing queue", frame.can_id));
            /* Unlock the queue to allow other threads to add frames */
            lock.unlock();

            /* Print the received CAN frame details */
            printFrames(frame);

            /* Extracting the components from can_id */

            /* Last byte: id_sender */
            uint8_t sender_id = (frame.can_id >> 8) & 0xFF;
            /* First byte: id_receiver or id_api */
            uint8_t receiver_id = frame.can_id & 0xFF;

            /* Starting frame processing timing if is it a frame request for MCU */
            auto it = std::find(vecU8_ServiceSids.begin(), vecU8_ServiceSids.end(), frame.data[1]);

            if (it != vecU8_ServiceSids.end() && receiver_id == 0x10) {
                startTimer(frame.data[1], sender_id);
            }

            /* Compare the CAN ID with the expected hexValueId */
            if (receiver_id == hex_value_id) 
            {
                if (frame.data[1] == 0xD9) 
                {
                    LOG_INFO(MCULogger->GET_LOGGER(), fmt::format("Frame received to notify MCU that ECU with ID: 0x{:x} is up", sender_id));
                    /* Set the corresponding value from the array with the ECU id */
                    switch(sender_id)
                    {
                        case 0x11:
                            ecus_up[0] = sender_id;
                            break;
                        case 0x12:
                            ecus_up[1] = sender_id;
                            break;
                        case 0x13:
                            ecus_up[2] = sender_id;
                            break;
                        case 0x14:
                            ecus_up[3] = sender_id;
                            break;
                        default:
                            break;
                    }

                    resetTimer(sender_id);
                }
                else 
                {
                    LOG_INFO(MCULogger->GET_LOGGER(), fmt::format("Received frame for MCU to execute service with SID: 0x{:x}", frame.data[1]));
                    LOG_INFO(MCULogger->GET_LOGGER(), "Calling HandleFrames module to execute the service and parse the frame.");
                    handler.handleFrame(getMcuSocket(sender_id), frame);
                    std::vector<uint8_t> response;
                    if (!SecurityAccess::getMcuState(*MCULogger))
                    {
                        securityNotifyECU({0x01,0xCF});
                        LOG_INFO(MCULogger->GET_LOGGER(), "Server is locked.");
                    }
                    else
                    {
                        securityNotifyECU({0x01,0xCE});
                        LOG_INFO(MCULogger->GET_LOGGER(), "Server is unlocked.");
                    }
                }
            }
            else if (receiver_id == 0xFA) 
            {
                LOG_DEBUG(MCULogger->GET_LOGGER(), fmt::format("Frame received from device with sender ID: 0x{:x} sent for API processing", sender_id));
                std::vector<uint8_t> data(frame.data, frame.data + frame.can_dlc);
                generate_frames.sendFrame(frame.can_id, data, iSocketApi, DATA_FRAME);
                LOG_DEBUG(MCULogger->GET_LOGGER(), fmt::format("Frame with ID: 0x{:x} sent on API socket", frame.can_id));
            } 

            if (sender_id == 0xFA && receiver_id != hex_value_id) 
            {
                if(frame.data[1] == 0x99)
                {
                    /** sends back to api a response with all ECUs IDs that are up 
                        response structure: 
                        id: MCU_id + API_id 
                        data: {PCI_L, SID(0xD9), MCU_id, BATTERY_id, DOORS_id, ENGINE_id, ECU4_id}
                    */
                    LOG_INFO(MCULogger->GET_LOGGER(), "Received frame to update status of ECUs still up.");
                    generate_frames.sendFrame(0x10FA,{0x06, 0xD9, MCU_ID, ecus_up[0], ecus_up[1], ecus_up[2], ecus_up[3]}, iSocketApi, DATA_FRAME);
                    LOG_INFO(MCULogger->GET_LOGGER(), "Frame sent to API on API socket to update status of ECUs still up.");
                }
                else
                {
                    std::vector<uint8_t> data(frame.data, frame.data + frame.can_dlc);
                    LOG_DEBUG(MCULogger->GET_LOGGER(), fmt::format("Received frame for ECU to execute service with SID: 0x{:x}", frame.data[1]));
                    /* Transfer data service need to have the data in the request body.
                        Here, if we have a transfer data request, we add the data to the request.
                        This is needed only if the transfer data request does not already contain the data to be sent => it's size == 3 (pci, sid, bl_indx)
                    */
                    if(frame.data[1] == TRANSFER_DATA_SID && data.size() == 3)
                    {
                        TransferData::processDataForTransfer(receiver_id, data, iSocketCanBus, *MCULogger);
                    }
                    generate_frames.sendFrame(frame.can_id, data);
                    LOG_DEBUG(MCULogger->GET_LOGGER(), fmt::format("Frame with ID: 0x{:x} sent on CANBus socket", frame.can_id));
                }
            }

            if (!listen_api && !listen_canbus) 
            {
                break;
            }
        }
    }

    void ReceiveFrames::startTimer(uint8_t sid, uint8_t sender_id) {
        /* Define the correct timer value based on SID */
        uint16_t timer_value;
        if (setU8_SidsUsingP2MaxTime.find(sid) != setU8_SidsUsingP2MaxTime.end())
        {
            timer_value = AccessTimingParameter::p2_max_time;
        } else {
            timer_value = AccessTimingParameter::p2_star_max_time;
        }

        LOG_INFO(MCULogger->GET_LOGGER(), "Started frame processing timing for frame with SID {:x} with max_time = {}.", sid, timer_value);

        auto start_time = std::chrono::steady_clock::now();
        mcu->mapU8F_TimingParameters[sid] = start_time.time_since_epoch().count();

        /* Initialize stop flag for this SID */
        mcu->mapU8AB_StopFlags[sid] = true;

        if (mcu->mapU8F_ActiveTimers.find(sid) != mcu->mapU8F_ActiveTimers.end()) {
            mcu->mapU8F_ActiveTimers.erase(sid);
        }

        mcu->mapU8F_ActiveTimers[sid] = std::async(std::launch::async, [sid, this, start_time, timer_value, sender_id]()
        {
            while (mcu->mapU8AB_StopFlags[sid])
            {
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = now - start_time;
                if (elapsed.count() > timer_value / 1000.0)
                { 
                    stopTimer(sid,sender_id);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    void ReceiveFrames::stopTimer(uint8_t sid, uint8_t sender_id) {
        LOG_INFO(MCULogger->GET_LOGGER(), "stopTimer function called for frame with SID {:x}.", sid);

        auto end_time = std::chrono::steady_clock::now();
        auto start_time = std::chrono::steady_clock::time_point(std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::nanoseconds((long long)mcu->mapU8F_TimingParameters[sid])));
        std::chrono::duration<double> processing_time = end_time - start_time;

        mcu->mapU8F_TimingParameters[sid] = processing_time.count();

        if (mcu->mapU8F_ActiveTimers.find(sid) != mcu->mapU8F_ActiveTimers.end())
        {
            /* Set stop flag to false for this SID */
            if(mcu->mapU8AB_StopFlags[sid])
            {
                int id = (0x10 << 8) | sender_id;
                LOG_INFO(MCULogger->GET_LOGGER(), "Service with SID {:x} sent the response pending frame.", sid);
                NegativeResponse negative_response(iSocketApi, *MCULogger);
                negative_response.sendNRC(id, sid, 0x78);
                mcu->mapU8AB_StopFlags[sid] = false;
            }
            mcu->mapU8AB_StopFlags.erase(sid);
        }
    }

    void ReceiveFrames::resetTimer(uint8_t ecu_id) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        ecu_timers[ecu_id] = std::chrono::steady_clock::now();
    }

    /**
     * Function to print the frames.
     */
    void ReceiveFrames::printFrames(const struct can_frame &frame)
    {
        LOG_DEBUG(MCULogger->GET_LOGGER(), "");
        LOG_DEBUG(MCULogger->GET_LOGGER(), "Received CAN frame");
        LOG_DEBUG(MCULogger->GET_LOGGER(), "Module ID: 0x{:x}", frame.can_id);
        LOG_DEBUG(MCULogger->GET_LOGGER(), "Data Length: {}", int(frame.can_dlc));
        std::ostringstream dataStream;
        dataStream << "Data:";
        for (int frame_byte = 0; frame_byte < frame.can_dlc; ++frame_byte) 
        {
            dataStream << " 0x" << std::hex << int(frame.data[frame_byte]);
        }
        LOG_DEBUG(MCULogger->GET_LOGGER(), "{}", dataStream.str());
        LOG_DEBUG(MCULogger->GET_LOGGER(), "");
    }

    void ReceiveFrames::vStopListenAPI()
    {
        listen_api = false;
        queue_cond_var.notify_all();
    }

    void ReceiveFrames::vStopListenCANBus()
    {
        listen_canbus = false;
        queue_cond_var.notify_all();
    }

    void ReceiveFrames::vStartListenAPI()
    {
        ReceiveFrames::listen_api = true;
        queue_cond_var.notify_all();
    }

    void ReceiveFrames::vStartListenCANBus()
    {
        listen_canbus = true;
        queue_cond_var.notify_all();
    }

    bool ReceiveFrames::getListenAPI()
    {
        return listen_api;
    }

    bool ReceiveFrames::getListenCANBus()
    {
        return listen_canbus;
    }

    const uint8_t* ReceiveFrames::getECUsUp() const {
        return ecus_up;
    }

    void ReceiveFrames::vStopProcessingQueue()
    {
        process_queue = false;
    }
    void ReceiveFrames::startTimerThread()
    {
        running = true;
        timer_thread = std::thread(&ReceiveFrames::timerCheck, this);
    }

    void ReceiveFrames::stopTimerThread() {
        running = false;
        if (timer_thread.joinable()) {
            timer_thread.join();
        }
    }

    void ReceiveFrames::timerCheck()
    {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto now = std::chrono::steady_clock::now();
            std::lock_guard<std::mutex> lock(queue_mutex);
            for (auto it = ecu_timers.begin(); it != ecu_timers.end();) {
                if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second) >= timeout_duration) {
                        ecus_up[(it->first-0x11)] = 0;
                    std::vector<uint8_t> data = {0x01, 0x99};
                    uint16_t id = (0x10 << 8) | it->first;
                    ReceiveFrames::generate_frames.sendFrame(id, data);
                    it = ecu_timers.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    void ReceiveFrames::securityNotifyECU(std::vector<uint8_t> response)
    {
        /* MCU ID as sender */
        uint8_t lowerbits = 0x10;
        /* Battery, Engine, Doors, HVAC receivers */
        uint8_t upperbits[] = {0x11, 0x12, 0x13, 0x14};

        // Send frames to each receiver
        for (uint8_t upper : upperbits)
        {
            generate_frames.sendFrame((lowerbits << 8) | upper, response, DATA_FRAME);
        }
    }

    int ReceiveFrames::getMcuSocket(uint8_t sender_id)
    {
        if(sender_id == API_ID)
        {
            return iSocketApi;
        }
        else
        {
            return iSocketCanBus;
        }
    }
}
