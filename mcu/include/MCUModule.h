/**
 * This class represents the MCU module that interacts with the CAN bus.
 * It provides methods to start and stop the module, as well as to receive CAN frames.
 * These methods are from the InterfaceModule and ReceiveFrames classes.
 */

#ifndef POC_MCU_MODULE_H
#define POC_MCU_MODULE_H

#include <future>
#include <fstream>
#include <stdexcept>
#include <filesystem>

#include "CreateInterface.h"
#include "ReceiveFrames.h"
#include "RequestUpdateStatus.h"

#define SYSTEM_SUPPLIER_ECU_SOFTWARE_VERSION_NUMBER_DID 0xF1A2

namespace MCU
{
    class MCUModule
    {
    public:
        /* Static dictionary to store SID and processing time */
        static std::map<uint8_t, double> mapU8F_TimingParameters;
        /* Store active timers for SIDs */
        static std::map<uint8_t, std::future<void>> mapU8F_ActiveTimers;
        /* Stop flags for each SID. */
        static std::map<uint8_t, std::atomic<bool>> mapU8AB_StopFlags;

        /* Variable to store mcu data */
        static std::unordered_map<uint16_t, std::vector<uint8_t>> umapU16V8_DefaultDIDMCU;
        static const std::vector<uint16_t> vecU16_WritableMCUDID;

        /**
         * @brief Constructor that takes the interface number as an argument.
         * When the constructor is called, it creates a new interface with the
         * given number and starts the interface.
         *
         * @param u8InterfacesNumber The number of the vcan interface
         */
        MCUModule(uint8_t u8InterfacesNumber);

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
        void vStartModule();

        /**
         * @brief Method to stop the module. Sets isRunning flag to false.
         */
        void vStopModule();

        /**
         * @brief Method to receive frames.
         * This method starts a thread to process the queue and receives frames
         * from the CAN bus.
         */
        void vRecvFrames();
        /**
         * @brief Get the Mcu Api Socket
         *
         * @return Returns the socket id for API
         */
        int iGetMcuApiSocket() const;

        /**
         * @brief Get the Mcu Ecu Socket
         *
         * @return Returns the socket id for ECU
         */
        int iGetMcuEcuSocket() const;

        /**
         * @brief Recreates and bind the socket of API on a given interface
         *
         * @param u8InterfaceNumber The interface on which the API socket will be created
         */
        void vSetMcuApiSocket(uint8_t u8InterfaceNumber);

        /**
         * @brief Recreates and bind the socket of ECU on a given interface
         *
         * @param u8InterfaceNumber The interface on which the ECU socket will be created
         */
        void vSetMcuEcuSocket(uint8_t u8InterfaceNumber);

        /**
         * @brief Write the default_did or the date before reset in mcu_data.txt
         *
         */
        void vWriteDataToFile();

        /**
         * @brief Method to check if a software update has been made.
         *
         */
        void vCheckSwVersion();
        /**
         * @brief This function generates random values for all DID entries defined in the default_DID_MCU map
         *
         */
        void vFetchMcuData();

    private:
        bool bIsRunning;
        CreateInterface *createInterface;
        ReceiveFrames *receiveFrames;
        /* These two will remain ints as uint_*t types cannot store negative values */
        int iMcuApiSocket = -1;
        int iMcuEcuSocket = -1;
    };

    extern MCUModule *mcu;
}
#endif