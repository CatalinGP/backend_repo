#include <thread>
#include <unistd.h>

#include "EngineModule.h"
#include "EngineModuleLogger.h"
#include "Globals.h"

int main() {
    loadProjectPathForECU();
    engineModuleLogger = new Logger("engineModuleLogger", std::string(PROJECT_PATH) + "/backend/ecu_simulation/EngineModule/logs/engineModuleLogger.log");
    engine = new EngineModule();
    stopProcess("main_engine");
    std::thread receiveFrThread([]()
                               { engine->_ecu->startFrames(); });
    sleep(200);
    receiveFrThread.join();
    return 0;
}
