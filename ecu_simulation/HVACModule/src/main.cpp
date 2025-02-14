#include <thread>
#include <unistd.h>

#include "HVACModule.h"
#include "HVACModuleLogger.h"
#include "Globals.h"

int main()
{
    v_loadProjectPath();
    hvacModuleLogger = new Logger("hvacModuleLogger", std::string(PROJECT_PATH) + "/backend/ecu_simulation/HVACModule/logs/hvacModuleLogger.log");

    hvac = new HVACModule();
    stopProcess("main_hvac");
    hvac->printHvacInfo();
    std::thread receiveFrThread([]()
                               { hvac->_ecu->startFrames(); });
    sleep(200);
    receiveFrThread.join();
    return 0;
}