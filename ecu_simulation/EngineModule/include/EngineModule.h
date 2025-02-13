#ifndef POC_INCLUDE_ENGINE_MODULE_H
#define POC_INCLUDE_ENGINE_MODULE_H

#define ENGINE_MODULE_ID 0x12


#include <iostream>
#include <unordered_map>
#include <vector> 

#include "ECU.h"

class EngineModule
{
private:
    const int ENGINE_ID = 0x12;

public:
    /* ECU object used for sockets, frame handling and ecu specific parameters (timing, flags etc)*/
    ECU *_ecu;
    /* Variable to store ecu data */
    static std::unordered_map<uint16_t, std::vector<uint8_t>> default_DID_engine;
    static const std::vector<uint16_t> writable_Engine_DID;
    /**
     * @brief Default constructor for Engine Module object.
     */
    EngineModule();
    /**
     * @brief Destructor Engine Module object.
     */
    virtual ~EngineModule();

    /**
     * @brief Retrieves existing DID values from the data file.
     * 
     */
    std::unordered_map<uint16_t, std::string> getExistingDIDValues(const std::string& file_path);

    /**
     * @brief This function generates random values for all DID entries defined in the default_DID_engine map
     * 
     */
    void fetchEngineData();

    /**
     * @brief Get the Engine Socket.
     * 
     * @return Returns the sid of the socket. 
     */
    int getEngineSocket() const;

    /**
     * @brief Write the default_did or the date before reset in engine_data.txt
     * 
     */
    void writeDataToFile();

    /**
     * @brief Check if the engine components exceed the normal values
     * 
     */
    void checkDTC();
};
extern EngineModule* engine;

#endif