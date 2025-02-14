#include "ReceiveFrames.h"
#include "BatteryModule.h"
#include "EngineModule.h"
#include "DoorsModule.h"
#include "HVACModule.h"
bool ReceiveFrames::ecu_state = false;
ReceiveFrames::ReceiveFrames(int socket, int current_module_id, Logger& receive_logger) : socket(socket),
                                                                                            current_module_id(current_module_id),
                                                                                            running(true), 
                                                                                            receive_logger(receive_logger),
                                                                                            handle_frame(socket, receive_logger)
{
    if (socket < 0) 
    {
        /* std::cerr << "Error: Pass a valid Socket\n"; */
        LOG_WARN(receive_logger.GET_LOGGER(), "Error: Pass a valid Socket\n");
        throw std::runtime_error("Error: Pass a valid Socket\n");
    }

    const int MIN_VALID_ID = 0x00000000;
    const int MAX_VALID_ID = 0x7FFFFFFF;

    if (current_module_id < MIN_VALID_ID || current_module_id > MAX_VALID_ID) 
    {
        /* std::cerr << "Error: Pass a valid Module ID\n"; */
        LOG_WARN(receive_logger.GET_LOGGER(), "Error: Pass a valid Module ID\n");
        throw std::runtime_error("Error: Pass a valid Module ID\n");
    }

    /* Print the frame_id for debugging */ 
    LOG_INFO(receive_logger.GET_LOGGER(), "Module ID: 0x{0:x}", this->current_module_id);
}

ReceiveFrames::~ReceiveFrames() 
{
    stop();
}

bool ReceiveFrames::getEcuState()
{
    return ecu_state;
}

void ReceiveFrames::setEcuState(bool value)
{
    ecu_state = value;
}

void ReceiveFrames::receive(HandleFrames &handle_frame) 
{
        bufferFrameInThread = std::thread(&ReceiveFrames::bufferFrameIn, this);
        this->bufferFrameOut(handle_frame);
}

/* Set the socket to non-blocking mode. */
void ReceiveFrames::stop() 
{   
    {
        std::unique_lock<std::mutex> lock(mtx);
        running = false;
    }
    cv.notify_all();
    if (bufferFrameInThread.joinable())
    {
    bufferFrameInThread.join();
    }
}

void ReceiveFrames::bufferFrameIn() 
{
    /* Define a pollfd structure to monitor the socket */ 
    struct pollfd pfd;
    /* Set the socket file descriptor */ 
    pfd.fd = this->socket; 
    /* We are interested in read events -use POLLING */ 
    pfd.events = POLLIN;    

    while (running) 
    {
        /* Use poll to wait for data to be available with a timeout of 1000ms (1 second) */ 
        int poll_result = poll(&pfd, 1, 1000);

        if (poll_result > 0) 
        {
            if (pfd.revents & POLLIN) 
            {
                struct can_frame frame = {};
                int nbytes = read(this->socket, &frame, sizeof(struct can_frame));
                uint8_t frame_receiver = frame.can_id & 0xFF;
                if (nbytes > 0 && frame_receiver == current_module_id) 
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    frame_buffer.push_back(std::make_tuple(frame, nbytes));
                    cv.notify_one();
                }
            }
        } 
        else if (poll_result == 0) 
        {
            /* Timeout occurred, no data available, but we just continue to check */ 
            continue;
        } 
        else 
        {
            /* An error occurred */ 
            LOG_ERROR(receive_logger.GET_LOGGER(), "poll error: {}", strerror(errno));
            break;
        }
    }
}

void ReceiveFrames::bufferFrameOut(HandleFrames &handle_frame) 
{
    while (running) 
    {
        label1: 
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !frame_buffer.empty() || !running; });
        if (!running && frame_buffer.empty()) 
        {
            break;
        }

        /* Extract frame and nbytes from the tuple in frame_buffer */ 
        auto frameTuple = frame_buffer.front();
        frame_buffer.pop_front();
        lock.unlock();

        struct can_frame frame = std::get<0>(frameTuple);

        /* Print the frame for debugging */ 
        printFrame(frame);

        uint8_t frame_dest_id = frame.can_id & 0xFF;

        /* Starting frame processing timing if is it a frame request for MCU */
        auto it = std::find(service_sids.begin(), service_sids.end(), frame.data[1]);

        if (it != service_sids.end() && frame.data[1] != TRANSFER_DATA_SID)
        {
            startTimer(frame_dest_id, frame.data[1]);
        }

        if (((frame.can_id >> 8) & 0xFF) == 0) 
        {
            LOG_WARN(receive_logger.GET_LOGGER(), "Invalid CAN ID: upper 8 bits are zero\n");
            return;
        }
        /* Notify from MCU to tell ECU's that MCU state is unlocked */
        if (frame.data[0] == 0x01 && frame.data[1] == 0xCE)
        {
            LOG_INFO(receive_logger.GET_LOGGER(), "Notification from the MCU that the server is unlocked.");
            ecu_state = true;
            goto label1;
        }
        /* Notify from MCU to tell ECU's that MCU state is locked */
        else if (frame.data[0] == 0x01 && frame.data[1] == 0xCF)
        {
            LOG_INFO(receive_logger.GET_LOGGER(), "Notification from the MCU that the server is locked.");
            ecu_state = false;
            goto label1;
        }

        /* Check if the frame is a request of type 'Up-Notification' from MCU */
        if (frame.data[1] == 0x99)
        {
            LOG_DEBUG(receive_logger.GET_LOGGER(), "Request received from MCU");
            /* Create and instance of GenerateFrames with the CAN socket */
            GenerateFrames frame = GenerateFrames(this->socket, receive_logger);

            /* Create a vector of uint8_t (bytes) containing the data to be sent */
            std::vector<uint8_t> data = {0x01, 0xD9};
            
            uint16_t id = (frame_dest_id << 8) | 0x10;
            frame.sendFrame(id, data);
            LOG_DEBUG(receive_logger.GET_LOGGER(), "Response sent to MCU");

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            goto label1;
        }
        /* Process the received frame */
        LOG_DEBUG(receive_logger.GET_LOGGER(), "Calling HandleFrames module to parse the frame.");
        handle_frame.handleFrame(socket, frame);
    }
}

void ReceiveFrames::startTimer(uint8_t frame_dest_id, uint8_t sid) {
    /* Define the correct timer value based on SID */
    uint16_t timer_value;
    if (sids_using_p2_max_time.find(sid) != sids_using_p2_max_time.end())
    {
        timer_value = AccessTimingParameter::p2_max_time;
    } else {
        timer_value = AccessTimingParameter::p2_star_max_time;
    }

    LOG_INFO(receive_logger.GET_LOGGER(), "Started frame processing timing for frame with SID {:x} with max_time = {} on ECU with id {}.", sid, timer_value, frame_dest_id);

    auto start_time = std::chrono::steady_clock::now();
    switch(frame_dest_id)
    {
    case 0x11:
        battery->_ecu->timing_parameters[sid] = start_time.time_since_epoch().count();

        // Initialize stop flag for this SID
        battery->_ecu->stop_flags[sid] = true;

        battery->_ecu->active_timers[sid] = std::async(std::launch::async, [sid, this, start_time, timer_value, frame_dest_id]() {
            while (battery->_ecu->stop_flags[sid]) {
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = now - start_time;
                if (elapsed.count() > timer_value / 20.0) {
                    stopTimer(frame_dest_id, sid);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        break;
    case 0x12:
        engine->_ecu->timing_parameters[sid] = start_time.time_since_epoch().count();

        // Initialize stop flag for this SID
        engine->_ecu->stop_flags[sid] = true;

        engine->_ecu->active_timers[sid] = std::async(std::launch::async, [sid, this, start_time, timer_value, frame_dest_id]() {
            while (engine->_ecu->stop_flags[sid]) {
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = now - start_time;
                if (elapsed.count() > timer_value / 20.0) {
                    stopTimer(frame_dest_id, sid);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        break;
    case 0x13:
        doors->_ecu->timing_parameters[sid] = start_time.time_since_epoch().count();

        // Initialize stop flag for this SID
        doors->_ecu->stop_flags[sid] = true;

        doors->_ecu->active_timers[sid] = std::async(std::launch::async, [sid, this, start_time, timer_value, frame_dest_id]() {
            while (doors->_ecu->stop_flags[sid]) {
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = now - start_time;
                if (elapsed.count() > timer_value / 20.0) {
                    stopTimer(frame_dest_id, sid);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        break;
    case 0x14:
        hvac->_ecu->timing_parameters[sid] = start_time.time_since_epoch().count();

        // Initialize stop flag for this SID
        hvac->_ecu->stop_flags[sid] = true;

        hvac->_ecu->active_timers[sid] = std::async(std::launch::async, [sid, this, start_time, timer_value, frame_dest_id]() {
            while (hvac->_ecu->stop_flags[sid]) {
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = now - start_time;
                if (elapsed.count() > timer_value / 20.0) {
                    stopTimer(frame_dest_id, sid);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        break;
    default:
        LOG_INFO(receive_logger.GET_LOGGER(), "starTimer function called with an ecu id unknown {:x}.", frame_dest_id);
        break;
    }
}

void ReceiveFrames::stopTimer(uint8_t frame_dest_id, uint8_t sid) {
    LOG_INFO(receive_logger.GET_LOGGER(), "stopTimer function called for frame with SID {:x}.", sid);
    
    auto end_time = std::chrono::steady_clock::now();

    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::duration<double> processing_time;

    switch (frame_dest_id) {
    case 0x11:
        start_time = std::chrono::steady_clock::time_point(
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::nanoseconds((long long)battery->_ecu->timing_parameters[sid])
            )
        );
        processing_time = end_time - start_time;

        battery->_ecu->timing_parameters[sid] = processing_time.count();

        if (battery->_ecu->active_timers.find(sid) != battery->_ecu->active_timers.end()) {
            /* Set stop flag to false for this SID */
            if (battery->_ecu->stop_flags[sid]) {
                int id = ((sid & 0xFF) << 8) | ((sid >> 8) & 0xFF);
                LOG_INFO(receive_logger.GET_LOGGER(), 
                         "Service with SID {:x} sent the response pending frame.", sid);
                
                NegativeResponse negative_response(socket, receive_logger);
                negative_response.sendNRC(id, sid, 0x78);
                battery->_ecu->stop_flags[sid] = false;
            }
            battery->_ecu->stop_flags.erase(sid);
            battery->_ecu->active_timers[sid].wait();
        }
        break;
    case 0x12:
        start_time = std::chrono::steady_clock::time_point(
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::nanoseconds((long long)engine->_ecu->timing_parameters[sid])
            )
        );
        processing_time = end_time - start_time;

        engine->_ecu->timing_parameters[sid] = processing_time.count();

        if (engine->_ecu->active_timers.find(sid) != engine->_ecu->active_timers.end()) {
            /* Set stop flag to false for this SID */
            if (engine->_ecu->stop_flags[sid]) {
                int id = ((sid & 0xFF) << 8) | ((sid >> 8) & 0xFF);
                LOG_INFO(receive_logger.GET_LOGGER(), 
                         "Service with SID {:x} sent the response pending frame.", sid);
                
                NegativeResponse negative_response(socket, receive_logger);
                negative_response.sendNRC(id, sid, 0x78);
                engine->_ecu->stop_flags[sid] = false;
            }
            engine->_ecu->stop_flags.erase(sid);
            engine->_ecu->active_timers[sid].wait();
        }
        break;
    case 0x13:
        start_time = std::chrono::steady_clock::time_point(
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::nanoseconds((long long)doors->_ecu->timing_parameters[sid])
            )
        );
        processing_time = end_time - start_time;

        doors->_ecu->timing_parameters[sid] = processing_time.count();

        if (doors->_ecu->active_timers.find(sid) != doors->_ecu->active_timers.end()) {
            /* Set stop flag to false for this SID */
            if (doors->_ecu->stop_flags[sid]) {
                int id = ((sid & 0xFF) << 8) | ((sid >> 8) & 0xFF);
                LOG_INFO(receive_logger.GET_LOGGER(), 
                         "Service with SID {:x} sent the response pending frame.", sid);
                
                NegativeResponse negative_response(socket, receive_logger);
                negative_response.sendNRC(id, sid, 0x78);
                doors->_ecu->stop_flags[sid] = false;
            }
            doors->_ecu->stop_flags.erase(sid);
            doors->_ecu->active_timers[sid].wait();
        }
        break;
    case 0x14:
        start_time = std::chrono::steady_clock::time_point(
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::nanoseconds((long long)hvac->_ecu->timing_parameters[sid])
            )
        );
        processing_time = end_time - start_time;

        hvac->_ecu->timing_parameters[sid] = processing_time.count();

        if (hvac->_ecu->active_timers.find(sid) != hvac->_ecu->active_timers.end()) {
            /* Set stop flag to false for this SID */
            if (hvac->_ecu->stop_flags[sid]) {
                int id = ((sid & 0xFF) << 8) | ((sid >> 8) & 0xFF);
                LOG_INFO(receive_logger.GET_LOGGER(), 
                         "Service with SID {:x} sent the response pending frame.", sid);
                
                NegativeResponse negative_response(socket, receive_logger);
                negative_response.sendNRC(id, sid, 0x78);
                hvac->_ecu->stop_flags[sid] = false;
            }
            hvac->_ecu->stop_flags.erase(sid);
            hvac->_ecu->active_timers[sid].wait();
        }
        break;
    default:
        LOG_INFO(receive_logger.GET_LOGGER(), "stopTimer function called with an ecu id unknown {:x}.", frame_dest_id);
        break;
    }
}

void ReceiveFrames::printFrame(const struct can_frame &frame) 
{
    LOG_DEBUG(receive_logger.GET_LOGGER(), "");
    LOG_DEBUG(receive_logger.GET_LOGGER(), "Received CAN frame");
    LOG_DEBUG(receive_logger.GET_LOGGER(), fmt::format("CAN ID: 0x{:x}", frame.can_id));
    LOG_DEBUG(receive_logger.GET_LOGGER(), "Data Length: {}", int(frame.can_dlc));
    std::ostringstream dataStream;
    dataStream << "Data:";
    for (int frame_byte = 0; frame_byte < frame.can_dlc; ++frame_byte) 
    {
        dataStream << " 0x" << std::hex << int(frame.data[frame_byte]);
    }
    LOG_DEBUG(receive_logger.GET_LOGGER(), "{}", dataStream.str());
}