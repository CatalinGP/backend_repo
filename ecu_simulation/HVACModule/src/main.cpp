#include <thread>
#include <unistd.h>

#include "HVACModule.h"
#include "HVACModuleLogger.h"
#include "Globals.h"

int main()
{
    loadProjectPathForECU();
    #ifdef UNIT_TESTING_MODE
    hvacModuleLogger = new Logger;
    #else
    hvacModuleLogger = new Logger("hvacModuleLogger", std::string(PROJECT_PATH) + "/backend/ecu_simulation/HVACModule/logs/hvacModuleLogger.log");
    #endif

    hvac = new HVACModule();
    hvac->printHvacInfo();
    std::thread receiveFrThread([]()
                               { hvac->_ecu->startFrames(); });
    sleep(200);
    receiveFrThread.join();
    return 0;
}