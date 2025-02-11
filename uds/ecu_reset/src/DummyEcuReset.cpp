#include "EcuReset.h"
#include "DummyEcuReset.h"
#include "Logger.h"

#include <cstdint>

DummyEcuReset::DummyEcuReset(uint32_t can_id, uint8_t sub_function, int socket, Logger &logger)
    : EcuReset(can_id, sub_function, socket, logger) {}

void DummyEcuReset::hardReset(){
    uint8_t lowerbits = can_id & 0xFF;
    /* Send response */
    this->ecuResetResponse();
    /* During tests we do not want to actually hard reset the MCU/ECU. We will only check
       if the lower bits match the MCU/ECU id. If not, log an error */
    if (lowerbits != 0x10
    && lowerbits != 0x11
    && lowerbits != 0x12
    && lowerbits != 0x13
    && lowerbits != 0x14) {
        LOG_ERROR(ECUResetLog.GET_LOGGER(), "ECU doesn't exist in hardReset");
    }
}