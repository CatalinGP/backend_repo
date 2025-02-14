#include <thread>
#include <unistd.h>

#include "BatteryModule.h"
#include "BatteryModuleLogger.h"
#include "Globals.h"

int main() {
    v_loadProjectPath();
    batteryModuleLogger = new Logger("batteryModuleLogger", std::string(PROJECT_PATH) + "/backend/ecu_simulation/BatteryModule/logs/batteryModuleLogger.log");
    battery = new BatteryModule();
    stopProcess("main_battery");
    std::thread receiveFrThread([]()
                               { battery->_ecu->startFrames(); });
    sleep(200);
    receiveFrThread.join();
    return 0;
}