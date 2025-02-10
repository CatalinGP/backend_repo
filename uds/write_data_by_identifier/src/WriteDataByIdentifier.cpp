#include <algorithm>

#include "WriteDataByIdentifier.h"
#include "BatteryModule.h"
#include "EngineModule.h"
#include "DoorsModule.h"
#include "HVACModule.h"
#include "DoorsModule.h"
#include "HVACModule.h"
#include "MCUModule.h"
#include "NegativeResponse.h"
#include "SecurityAccess.h"
#include "FileManager.h"
#include "AccessTimingParameter.h"
#include "Globals.h"

WriteDataByIdentifier::WriteDataByIdentifier(Logger& wdbi_logger, int socket)
            : generate_frames(socket, wdbi_logger), wdbi_logger(wdbi_logger)
{   
    this->socket = socket;
}

WriteDataByIdentifier::~WriteDataByIdentifier()
{
};

void WriteDataByIdentifier::WriteDataByIdentifierService(canid_t frame_id, std::vector<uint8_t> frame_data)
{
    LOG_INFO(wdbi_logger.GET_LOGGER(), "Write Data By Identifier Service invoked.");

    std::vector<uint8_t> response;
    NegativeResponse nrc(socket, wdbi_logger);

    /* Form the new id */
    int id = ((frame_id & 0xFF) << 8) | ((frame_id >> 8) & 0xFF);
    uint8_t receiver_id = frame_id & 0xFF;

    /* Checks if frame_data has the required minimum length */
    if (frame_data.size() < 3)
    {
        nrc.sendNRC(id, WDBI_SID, NegativeResponse::IMLOIF);
        uint8_t receiver_id = frame_id & 0xFF;
        AccessTimingParameter::stopTimingFlag(receiver_id, 0x2E);
    }
    else if (receiver_id == 0x10 && !SecurityAccess::getMcuState(wdbi_logger))
    {
        nrc.sendNRC(id, WDBI_SID, NegativeResponse::SAD);
        AccessTimingParameter::stopTimingFlag(receiver_id, 0x2E);
    }
    else if ((receiver_id == 0x11 || receiver_id == 0x12 ||
              receiver_id == 0x13 || receiver_id == 0x14) &&
              !ReceiveFrames::getEcuState())
    {
        nrc.sendNRC(id, WDBI_SID, NegativeResponse::SAD);
        AccessTimingParameter::stopTimingFlag(receiver_id, 0x2E);
    }
    else
    {
        typedef uint16_t DID;
        typedef std::vector<uint8_t> DataParameter;

        /* Extract Data Identifier (DID) */
        DID did = (frame_data[2] << 8) | frame_data[3];
        switch (receiver_id)
        {
            case 0x10:
                if((std::find(MCU::mcu->vecU16_WritableMCUDID.begin(), MCU::mcu->vecU16_WritableMCUDID.end(), did)) == MCU::mcu->vecU16_WritableMCUDID.end())
                {
                    LOG_ERROR(wdbi_logger.GET_LOGGER(), "Request Out Of Range: Identifier not found in memory");
                    nrc.sendNRC(id,WDBI_SID,NegativeResponse::ROOR);
                    MCU::mcu->mapU8AB_StopFlags[0x2E] = false;
                    return;
                }
                break;
            case 0x11:
                if((std::find(battery->writable_Battery_DID.begin(), battery->writable_Battery_DID.end(), did)) == battery->writable_Battery_DID.end())
                {
                    LOG_ERROR(wdbi_logger.GET_LOGGER(), "Request Out Of Range: Identifier not found in memory");
                    nrc.sendNRC(id,WDBI_SID,NegativeResponse::ROOR);
                    battery->_ecu->stop_flags[0x2E] = false;
                    return;
                }
                break;
            case 0x12:
                if((std::find(engine->writable_Engine_DID.begin(), engine->writable_Engine_DID.end(), did)) == engine->writable_Engine_DID.end())
                {
                    LOG_ERROR(wdbi_logger.GET_LOGGER(), "Request Out Of Range: Identifier not found in memory");
                    nrc.sendNRC(id,WDBI_SID,NegativeResponse::ROOR);
                    engine->_ecu->stop_flags[0x2E] = false;
                    return;
                }
                break;
            case 0x13:
                if((std::find(doors->writable_Doors_DID.begin(), doors->writable_Doors_DID.end(), did)) == doors->writable_Doors_DID.end())
                {
                    LOG_ERROR(wdbi_logger.GET_LOGGER(), "Request Out Of Range: Identifier not found in memory");
                    nrc.sendNRC(id,WDBI_SID,NegativeResponse::ROOR);
                    doors->_ecu->stop_flags[0x2E] = false;
                    return;
                }
                break;
            case 0x14:
                if((std::find(hvac->writable_HVAC_DID.begin(), hvac->writable_HVAC_DID.end(), did)) == hvac->writable_HVAC_DID.end())
                {
                    LOG_ERROR(wdbi_logger.GET_LOGGER(), "Request Out Of Range: Identifier not found in memory");
                    nrc.sendNRC(id,WDBI_SID,NegativeResponse::ROOR);
                    hvac->_ecu->stop_flags[0x2E] = false;
                    return;
                }
                break;
            default:
                LOG_ERROR(wdbi_logger.GET_LOGGER(), "Invalid module.");
                break;
        }

        /* Extract Data Parameter */
        DataParameter data_parameter(frame_data.begin() + 4, frame_data.end());
        uint8_t receiver_id = frame_id & 0xFF;

        std::string file_name = std::string(PROJECT_PATH);
        if (receiver_id == 0x10)
        {
            file_name += "/backend/mcu/mcu_data.txt";
        }
        else if (receiver_id == 0x11)
        {
            file_name += "/backend/ecu_simulation/BatteryModule/battery_data.txt";
        }
        else if (receiver_id == 0x12)
        {
            file_name += "/backend/ecu_simulation/EngineModule/engine_data.txt";
        }
        else if (receiver_id == 0x13)
        {
            file_name += "/backend/ecu_simulation/DoorsModule/doors_data.txt";
        }
        else if (receiver_id == 0x14)
        {
            file_name += "/backend/ecu_simulation/HVACModule/hvac_data.txt";
        }
        else
        {
            LOG_ERROR(wdbi_logger.GET_LOGGER(), "Module with id {:x} not supported.", receiver_id);
            nrc.sendNRC(id, WDBI_SID, NegativeResponse::ROOR);
            AccessTimingParameter::stopTimingFlag(receiver_id, 0x2E);
            return;
        }

        try
        {
            /* Load current data from the file */
            auto data_map = FileManager::readMapFromFile(file_name);

            // Update or add the new data for the given DID
            data_map[did] = data_parameter;

            /* Save the updated data back to the file */
            FileManager::writeMapToFile(file_name, data_map);

            /* Check the new value */
            switch (receiver_id)
            {
            case 0x11:
                battery->checkDTC();
                break;
            case 0x12:
                engine->checkDTC();
                break;
            
            default:
                break;
            }
            LOG_INFO(wdbi_logger.GET_LOGGER(), "Data written to DID 0x{:x} in the module with id {}.", did, receiver_id);
        } catch (const std::exception& e) 
        {
            LOG_ERROR(wdbi_logger.GET_LOGGER(), "Error processing file: {}", e.what());
            nrc.sendNRC(id, WDBI_SID, NegativeResponse::ROOR);
            AccessTimingParameter::stopTimingFlag(receiver_id, 0x2E);
            return;
        }

        /* Send response frame */
        generate_frames.writeDataByIdentifier(id, did, {});
        LOG_INFO(wdbi_logger.GET_LOGGER(), "Service with SID {:x} successfully sent the response frame.", 0x2E);
        AccessTimingParameter::stopTimingFlag(receiver_id, 0x2E);
    }
};