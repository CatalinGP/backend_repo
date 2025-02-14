/*
 * The ReceiveFrameModule library facilitates the reception of Controller Area Network (CAN) 
 * frames through an interface utilizing sockets. This library allows you to read CAN frames 
 * from the CAN bus and frames from an API, then process them based on specific criteria.
 *
 * The library provides methods for receiving and handling CAN and API frames:
 *  - receiveFramesFromCANBus: Continuously reads CAN frames from the specified socket and processes them.
 *  - receiveFramesFromAPI: Continuously reads frames from the API socket and processes them.
 *  - processQueue: Processes frames from the queue and calls HandleFrames and GenerateFrame as needed.
 *  - You can choose to listen to frames from either the CAN bus or the API by setting the corresponding listen flags.
 *
 * How to use example:
 *    int socketCANBus = //... initialize your CAN bus socket here
 *    int socketAPI = //... initialize your API socket here
 *    ReceiveFrames rfm(socketCANBus, socketAPI);
 *
 *    // Start listening to CAN bus frames
 *    rfm.startListenCANBus();
 *    rfm.receiveFramesFromCANBus();
 *
 *    // Start listening to API frames
 *    rfm.startListenAPI();
 *    rfm.receiveFramesFromAPI();
 *
 *    // To stop listening, you can call the stop functions
 *    rfm.stopListenCANBus();
 *    rfm.stopListenAPI();
 *
 *    // Process the frames in the queue
 *    rfm.processQueue();
 *
 * Author: Dirva Nicolae, 2024
 */

#ifndef POC_SRC_MCU_RECEIVE_FRAME_MODULE_H
#define POC_SRC_MCU_RECEIVE_FRAME_MODULE_H

#include<string>
#include<stdlib.h>
#include<iostream>
#include<unistd.h>
#include<cstring>
#include<sstream>
#include<vector>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<algorithm>
#include<linux/can.h>
#include<map>
#include<chrono>
#include<thread>
#include <future>
#include <atomic>
#include <set>
#include <future>
#include <atomic>
#include <set>

#include "HandleFrames.h"
#include "GenerateFrames.h"
#include "MCULogger.h"


namespace MCU
{
  /* List of service we have implemented. */
  const std::vector<uint8_t> service_sids = {
      /* ECU Reset */
      0x11,
      /* Diagnostic Session Control */
      0x10,
      /* Read Data By Identifier */
      0x22,
      /* Authentication */
      0x29,
      /* Routine Control (Testing) -> will be decided */
      0x31,
      /* Tester Present */
      0x3E,
      /* Read Memory By Address */
      0x23,
      /* Write Data By Identifier */
      0x2E,
      /* Read DTC Information */
      0x19,
      /* Clear Diagnostic Information */
      0x14,
      /* Access Timing Parameters */
      0x83,
      /* Request Download */
      0x34,
      /* Transfer Data */
      0x36,
      /* Request Transfer Exit */
      0x37,
      /* Request update status */
      0x32
  };

  /* Define lists of SIDs using p2_max_time and p2_star_max_time */
  static const std::set<uint8_t> sids_using_p2_max_time = {
      /* ECU Reset */
      0x11,
      /* Diagnostic Session Control */
      0x10,
      /* Read Data By Identifier */
      0x22,
      /* Tester Present */
      0x3E,
      /* Read Memory By Address */
      0x23,
      /* Write Data By Identifier */
      0x2E,
      /* Read DTC Information */
      0x19,
      /* Clear Diagnostic Information */
      0x14,
      /* Access Timing Parameters */
      0x83,
      /* Request update status */
      0x32
  };

  static const std::set<uint8_t> sids_using_p2_star_max_time = {
      /* Routine Control */
      0x31,
      /* Request Download */
      0x34,
      /* Transfer Data */
      0x36,
      /* Request Transfer Exit */
      0x37,
      /* Authentication */
      0x27
  };

  class ReceiveFrames
  {
  public:

    /**
     * @brief Parameterized constructor.
     * @param socket_canbus Socket for ECU communication.
     * @param socket_api Socket for API communication.
     */
    ReceiveFrames(int socket_canbus, int socket_api);

    /**
     * @brief Destructor.
     */
    ~ReceiveFrames();

    /**
     * @brief Function that take the frame from CANBus and put it in process queue.
     * 
     * @return Returns 1 for error and 0 for successfully.
     */
    bool receiveFramesFromCANBus();

    /**
     * @brief Function that take the frame from API and put it in process queue.
     * 
     * @return Returns 1 for error and 0 for successfully.
     */
    bool receiveFramesFromAPI();


    /**
     * @brief Function that take each frame from process queue and partially parse the frame to know
      for who is the frame. After, call handle class.
    */
    void processQueue();

    /**
     * @brief Function that print a frame with all information.
     * 
     * @param frame The frame that will be printed.
     */
    void printFrames(const struct can_frame &frame);

    /**
     * @brief Set listenAPI member to false.
     */
    void stopListenAPI();

      /**
     * @brief Set listenCANBus member to false.
     */
    void stopListenCANBus();

    /**
     * @brief Set listenAPI member to true.
     */
    void startListenAPI();

    /**
     * @brief Set listenCANBus member to true.
     */
    void startListenCANBus();

    /**
     * @brief Get method for hexValueId.
     * 
     * @return Returns hexValueId for the object. 
     */
    uint32_t gethexValueId();

    /**
     * @brief Get method for listen_api flag.
     * 
     * @return Returns the listen_api flag. 
     */
    bool getListenAPI();

    /**
     * @brief Get method for listen_canbus flag.
     * 
     * @return Returns the listen_canbus flag. 
     */
    bool getListenCANBus();

    /**
     * @brief Get method for the list of ECUs that are up.
     * 
     * @return Returns ecus_up (the list of ECUs that are up). 
     */
    const uint8_t* getECUsUp() const;

    /**
     * @brief set method used to set the processing flag to false in order to be able to stop mcu module.
     * 
     * @return Returns ecus_up (the list of ECUs that are up). 
     */
    void stopProcessingQueue();
    std::map<uint8_t, std::chrono::steady_clock::time_point> ecu_timers;
    std::chrono::seconds timeout_duration;
    std::thread timer_thread;
    bool running;

    /* Method that start time processing frame. */
    void startTimer(uint8_t sid);
    /* Method that stop time processing frame. */
    void stopTimer(uint8_t sid);

  protected:
    /* The socket from where we read the frames */
    int socket_canbus;
    int socket_api;
    const uint32_t hex_value_id = 0x10;
    std::queue<struct can_frame> frame_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cond_var;
    bool listen_api;
    bool listen_canbus;
    HandleFrames handler;
    GenerateFrames generate_frames;
    /* Vector contains all the ECUs up ids */
    uint8_t ecus_up[4] = {0};
    bool process_queue = true;

    /**
     * @brief Starts timer_thread and sets running flag on true.
     */
    void startTimerThread();

    /**
     * @brief Stops timer_thread and sets running flag on false.
     */
    void stopTimerThread();

    /**
     * @brief Checks timer and send requests to check if the ECUs are up.
     */
    void timerCheck();

    /**
     * @brief Reset the timer and add the ECU to the list.
     * 
     * @param ecu_id The identifier of the ECU (will be added to the list).
     */
    void resetTimer(uint8_t ecu_id);

    /**
     * @brief return the socket, either vcan1 socket or vcan0 socket
     * 
     * @param[in] sender_id The sender id.
     * @return int Returns socket, either API, either canbus
     */
    int getMcuSocket(uint8_t sender_id);
    /**
     * @brief Notify each ECU that the MCU state is unlocked.
     * This method sends a notification frame to multiple ECUs (Battery, Engine, Doors, HVAC)
     * indicating that the MCU state is unlocked. It appends the necessary data to the provided
     * response vector and uses the class's generate_frames attribute to send the frames.
     * 
     * @param[in] response vector containing the data to be sent in each frame.
     */
    void securityNotifyECU(std::vector<uint8_t> response);

  };
}
#endif /* POC_SRC_MCU_RECEIVE_FRAME_MODULE_H */