#include <thread>
#include <unistd.h>

#include "MCUModule.h"
#include "MCULogger.h"
#include "Globals.h"

int main() {

    loadProjectPathForMCU();
    MCULogger = new Logger("MCULogger", std::string(PROJECT_PATH) + "/backend/mcu/logs/MCULogs.log");
    MCU::mcu = new MCU::MCUModule(0x01);
    stopProcess("main_mcu");
    MCU::mcu->vStartModule();
    std::thread thReceiveFr([]()
    { 
        MCU::mcu->vRecvFrames();
    });
    sleep(3);
    /* commented this as we don't want it to stop, it was for test purposes */
    /* MCU::mcu->StopModule(); */
    thReceiveFr.join();
    return 0;
}