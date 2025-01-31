#include <thread>
#include <unistd.h>

#include "DoorsModule.h"
#include "DoorsModuleLogger.h"
#include "Globals.h"

int main() {
    loadProjectPathForECU();
    #ifdef UNIT_TESTING_MODE
    doorsModuleLogger = new Logger;
    #else
    doorsModuleLogger = new Logger("doorsModuleLogger", std::string(PROJECT_PATH) + "/backend/ecu_simulation/DoorsModule/logs/doorsModuleLogger.log");
    #endif /* UNIT_TESTING_MODE */
    doors = new DoorsModule();
    doors->_ecu->stopProcess("main_doors");
    std::thread receiveFrThread([]()
                               { doors->_ecu->startFrames(); });
    sleep(200);
    receiveFrThread.join();
    return 0;
}