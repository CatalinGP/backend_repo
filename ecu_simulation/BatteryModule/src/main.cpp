#include "BatteryModule.h"
#include "BatteryModuleLogger.h"
#include <thread>
#include <unistd.h>

int main() {
    #ifdef UNIT_TESTING_MODE
    batteryModuleLogger = new Logger;
    #else
    batteryModuleLogger = new Logger("batteryModuleLogger", std::string(PROJECT_PATH) + "/backend/ecu_simulation/BatteryModule/logs/batteryModuleLogger.log");
    #endif /* UNIT_TESTING_MODE */
    battery = new BatteryModule();
    std::thread receiveFrThread([]()
                               { battery->_ecu->startFrames(); });
    sleep(200);
    receiveFrThread.join();
    return 0;
}