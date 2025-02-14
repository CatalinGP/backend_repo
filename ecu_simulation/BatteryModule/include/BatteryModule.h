/**
 *  @file BatteryModule.h
 *  This library will be used to simulate a Battery Module
 *  providing some readings and informations (like energy, voltage, percentage, etc)
 *  extracted from Linux, in order to be passed through a virtual CAN interface.
 * 
 *  How to use : Simply instantiate the class in Main solution, and access it's methods there.
 *  BatteryModule batteryObj;  *  Default Constructor for battery object with
 *                             *  moduleID 0x11 and interface name 'vcan0'
 *  BatteryModule batteryObj(interfaceNumber, moduleID);  *  Custom Constructor, for battery object
 *                             *  with custom moduleID, custom interface name
 * @author: Alexandru Nagy
 * @date: May 2024
 * @version 1.0
 */

#ifndef POC_INCLUDE_BATTERY_MODULE_H
#define POC_INCLUDE_BATTERY_MODULE_H

#define BATTERY_MODULE_ID 0x11

#include <thread>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <unistd.h>
#include <future>
#include <fstream>
#include "CreateInterface.h"
#include "ReceiveFrames.h"
#include "GenerateFrames.h"
#include "BatteryModuleLogger.h"
#include "ECU.h"

class BatteryModule
{
private:
    const uint8_t BATTERY_ID = 0x11;

    float energy;
    float voltage;
    float percentage;
    std::string state;

public:
    /* ECU object used for sockets, frame handling and ecu specific parameters (timing, flags etc)*/
    ECU *_ecu;
    /* Variable to store ecu data */
    static std::unordered_map<uint16_t, std::vector<uint8_t>> default_DID_battery;
    /**
     * @brief Default constructor for Battery Module object.
     */
    BatteryModule();
    /**
     * @brief Destructor Battery Module object.
     */
    virtual ~BatteryModule();

    /**
     * @brief Helper function to execute shell commands and fetch output
     * in order to read System Information about built-in Battery.
     * This method is currently 'virtual' in order to be overridden in Test.
     * 
     * @param cmd Shell command to be executed.
     * @param mode A string that specifies I/O mode.
     * @return Output returned by the shell command. 
     */
    virtual std::string exec(const char *cmd, const char *mode);

    /**
     * @brief This function will parse the data from the system about battery,
     * and will store all values in separate variables
     * 
     * @param data Data taken from the system.
     * @param _energy System energy level.
     * @param _voltage System voltage.
     * @param _percentage System battery percentage.
     * @param _temperature System temperature.
     */
    void parseBatteryInfo(const std::string &data, float &energy, float &voltage, float &percentage, std::string &state);

    /**
     * @brief Function to fetch data from system about battery.
     * 
     * @param mode A string that specifies I/O mode for command.
     */
    void fetchBatteryData(const char *mode);

    /* Member Accessors */
    /**
     * @brief Get function for energy.
     * 
     * @return Returns energy.
     */
    float getEnergy() const;

    /**
     * @brief Get function for voltage.
     * 
     * @return Returns voltage. 
     */
    float getVoltage() const;

    /**
     * @brief Get function for battery percentage.
     * 
     * @return Returns battery percentage. 
     */
    float getPercentage() const;

    /**
     * @brief Get the Linux Battery State - charging, discharging, fully-charged, etc.
     * 
     * @return Returns Battery State - charging, discharging, fully-charged, etc.
     */
    std::string getLinuxBatteryState();

    /**
     * @brief Get the Battery Socket.
     * 
     * @return Returns the sid of the socket. 
     */
    int getBatterySocket() const;

    /**
     * @brief Write the default_did or the date before reset in battery_data.txt
     * 
     */
    void writeDataToFile();

    /**
     * @brief Check if the battery components exceed the normal values
     * 
     */
    void checkDTC();
};
extern BatteryModule* battery;

#endif