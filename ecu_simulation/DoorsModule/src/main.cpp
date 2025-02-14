#include <thread>
#include <unistd.h>

#include "DoorsModule.h"
#include "DoorsModuleLogger.h"
#include "Globals.h"

int main() {
    v_loadProjectPath();
    doorsModuleLogger = new Logger("doorsModuleLogger", std::string(PROJECT_PATH) + "/backend/ecu_simulation/DoorsModule/logs/doorsModuleLogger.log");
    doors = new DoorsModule();
    stopProcess("main_doors");
    std::thread receiveFrThread([]()
                               { doors->_ecu->startFrames(); });
    sleep(200);
    receiveFrThread.join();
    return 0;
}