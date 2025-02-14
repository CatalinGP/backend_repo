/**
 * This class represents the MCU module that interacts with the CAN bus.
 * It provides methods to start and stop the module, as well as to receive CAN frames.
 * These methods are from the InterfaceModule and ReceiveFrames classes.
*/

#ifndef POC_MCU_MODULE_H
#define POC_MCU_MODULE_H

#include "HandleFrames.h"
#include "CreateInterface.h"
#include "ReceiveFrames.h"
#include "MCULogger.h"
#include "TesterPresent.h"

#include <thread>
#include <future>
#include <fstream>
#include <stdexcept>
#include <filesystem>

namespace MCU
{
    class MCUModule {
    public:
        /* Static dictionary to store SID and processing time */
        static std::map<uint8_t, double> timing_parameters;
        /* Store active timers for SIDs */
        static std::map<uint8_t, std::future<void>> active_timers;
        /* Stop flags for each SID. */
        static std::map<uint8_t, std::atomic<bool>> stop_flags;

        /* Variable to store mcu data */
        std::unordered_map<uint16_t, std::vector<uint8_t>> default_DID_MCU = 
        {
            {0xE001, {IDLE}},
#ifdef SOFTWARE_VERSION
            {0xF1A2, {static_cast<uint8_t>(SOFTWARE_VERSION)}}
#else
            {0xF1A2, {0x00}}
#endif
        };
        static const std::vector<uint16_t> VALID_DID_MCU;

        /** 
         * @brief Constructor that takes the interface number as an argument.
         * When the constructor is called, it creates a new interface with the
         * given number and starts the interface.
         * 
         * @param interface_number The number of the vcan interface
        */
        MCUModule(uint8_t interfaces_number);
        
        /**
         * @brief Default constructor for MCU Module.
         */
        MCUModule();

        /**
         * @brief Destructor for MCU Module.
         */
        ~MCUModule();

        /**
         * @brief Method to start the module. Sets isRunning flag to true.
        */
        void StartModule();

        /**
         * @brief Method to stop the module. Sets isRunning flag to false.
        */
        void StopModule();

        /**
         * @brief Method to receive frames.
         * This method starts a thread to process the queue and receives frames
         * from the CAN bus.
        */
        void recvFrames();
        /**
         * @brief Get the Mcu Api Socket
         * 
         * @return Returns the socket id for API
         */
        int getMcuApiSocket() const;

        /**
         * @brief Get the Mcu Ecu Socket
         * 
         * @return Returns the socket id for ECU
         */
        int getMcuEcuSocket() const;

        /**
         * @brief Recreates and bind the socket of API on a given interface
         * 
         * @param interface_number The interface on which the API socket will be created
         */
        void setMcuApiSocket(uint8_t interface_number);

        /**
         * @brief Recreates and bind the socket of ECU on a given interface
         * 
         * @param interface_number The interface on which the ECU socket will be created
         */
        void setMcuEcuSocket(uint8_t interface_number);

        /**
         * @brief Write the default_did or the date before reset in mcu_data.txt
         * 
         */
        void writeDataToFile();
 
    private:
        bool is_running;
        CreateInterface* create_interface;
        ReceiveFrames* receive_frames;
        int mcu_api_socket = -1;
        int mcu_ecu_socket = -1;
    };
extern MCUModule* mcu;
}
#endif