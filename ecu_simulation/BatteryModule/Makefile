######### Makefile for BatteryModule
######### Author: Theodor Stoica (contact theodor.stoica@randstaddigital.com for any help)

#POC Project root path
PROJECT_PATH := $(shell cd $(shell pwd)/../../../ && pwd)
SOFTWARE_VERSION = 0
PROJECT_DEFINES = -DPROJECT_PATH=\"$(PROJECT_PATH)\" -DSOFTWARE_VERSION=${SOFTWARE_VERSION}

#Python flags
PYTHON_INCL_DIR=-I/usr/include/python3.8
PYTHON_LDFLAGS = -L/usr/lib/python3.8/config-3.8-x86_64-linux-gnu -lpython3.8 -lcrypt -ldl -lutil -lm -lpthread

# Uncomment this lines in production when running on Raspberry Pi
# +PYTHON_INCL_DIR=-I/usr/local/include/python3.8
# +PYTHON_LDFLAGS = -L/usr/local/lib/python3.8/config-3.8-x86_64-linux-gnu -lpython3.8 -lcrypt -ldl -lutil -lm -lpthread

# Compiler flags
CXX = g++
CFLAGS = -std=c++17 -g -Wall -Werror -pthread \
         -I./include \
         -I../../utils/include \
         -I../../mcu/include \
         -I../DoorsModule/include \
         -I../EngineModule/include \
         -I../HVACModule/include \
         -I../../uds/access_timing_parameters/include \
         -I../../uds/authentication/include \
         -I../../uds/clear_dtc/include \
         -I../../uds/diagnostic_session_control/include \
         -I../../uds/ecu_reset/include \
         -I../../uds/read_data_by_identifier/include \
         -I../../uds/read_dtc_information/include \
         -I../../uds/read_memory_by_address/include \
         -I../../uds/routine_control/include \
         -I../../uds/tester_present/include \
         -I../../uds/write_data_by_identifier/include \
         -I../../ota/request_download/include \
         -I../../ota/request_transfer_exit/include \
         -I../../ota/request_update_status/include \
         -I../../ota/transfer_data/include \
         ${PYTHON_INCL_DIR} ${PROJECT_DEFINES}
         
LDFLAGS = -lspdlog -lfmt -lzip ${PYTHON_LDFLAGS}

# Test flags
CFLAGSTST = -std=c++17 -I/include -fprofile-arcs -ftest-coverage
CFLAGSTST2 = -lgtest -lgtest_main -lpthread
CFLAGSTST += -DUNIT_TESTING_MODE

# Directories
SRC_DIR = src
SRC_TEST = test
UTILS_DIR = ../../utils/src
UTILS_TEST = ../../utils/test
UDS_DIR = ../../uds
MCU_DIR = ../../mcu/src
ENGINE_DIR = ../EngineModule/src
DOORS_DIR = ../DoorsModule/src
HVAC_DIR = ../HVACModule/src
OTA_DIR = ../../ota
OBJ_DIR = obj

#----------------------------------------------------MCU MODULE-----------------------------------------------------

# Object files
BATTERY_OBJS = $(OBJ_DIR)/main.o \
			   $(OBJ_DIR)/BatteryModule.o \

# MCU object files
MCU_OBJS = $(OBJ_DIR)/MCUModule.o \
		   $(OBJ_DIR)/MCUModule_ReceiveFrames.o 

# Engine object files
ENGINE_OBJS = $(OBJ_DIR)/EngineModule.o \

# Doors object files
DOORS_OBJS = $(OBJ_DIR)/DoorsModule.o \

# HVAC object files
HVAC_OBJS = $(OBJ_DIR)/HVACModule.o \
		   
# Utils object files
UTILS_OBJS = $(OBJ_DIR)/MemoryManager.o \
             $(OBJ_DIR)/Logger.o \
             $(OBJ_DIR)/GenerateFrames.o \
             $(OBJ_DIR)/CreateInterface.o \
             $(OBJ_DIR)/NegativeResponse.o \
			 $(OBJ_DIR)/ReceiveFrames.o \
			 $(OBJ_DIR)/HandleFrames.o \
			 $(OBJ_DIR)/FileManager.o \
			 $(OBJ_DIR)/ECU.o

# UDS object files
UDS_OBJS = $(OBJ_DIR)/DiagnosticSessionControl.o \
           $(OBJ_DIR)/ReadDataByIdentifier.o \
           $(OBJ_DIR)/WriteDataByIdentifier.o \
           $(OBJ_DIR)/EcuReset.o \
           $(OBJ_DIR)/SecurityAccess.o \
		   $(OBJ_DIR)/TesterPresent.o \
           $(OBJ_DIR)/ReadDtcInformation.o \
           $(OBJ_DIR)/ClearDtc.o \
           $(OBJ_DIR)/RoutineControl.o \
		   $(OBJ_DIR)/AccessTimingParameter.o \
		   $(OBJ_DIR)/ReadMemoryByAddress.o

# OTA object files
OTA_OBJS = $(OBJ_DIR)/RequestUpdateStatus.o \
           $(OBJ_DIR)/RequestTransferExit.o \
           $(OBJ_DIR)/RequestDownload.o \
           $(OBJ_DIR)/TransferData.o
           
OBJS = $(MCU_OBJS) \
       $(BATTERY_OBJS) \
	   $(ENGINE_OBJS) \
	   $(DOORS_OBJS) \
	   $(HVAC_OBJS) \
       $(UTILS_OBJS) \
       $(UDS_OBJS) \
       $(OTA_OBJS)

# Target executables
FINAL = main_battery

.PHONY: all clean

all: $(OBJ_DIR) $(FINAL)

$(FINAL): $(OBJS)
	$(CXX) $(CFLAGS) -o $(FINAL) $(OBJS) $(LDFLAGS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile individual source files into object files
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp
	$(CXX) $(CFLAGS) -c $(SRC_DIR)/main.cpp -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/MCUModule.o: $(MCU_DIR)/MCUModule.cpp
	$(CXX) $(CFLAGS) -c $(MCU_DIR)/MCUModule.cpp -o $(OBJ_DIR)/MCUModule.o

$(OBJ_DIR)/BatteryModule.o: $(SRC_DIR)/BatteryModule.cpp
	$(CXX) $(CFLAGS) -c $(SRC_DIR)/BatteryModule.cpp -o $(OBJ_DIR)/BatteryModule.o
	
$(OBJ_DIR)/EngineModule.o: $(ENGINE_DIR)/EngineModule.cpp
	$(CXX) $(CFLAGS) -c $(ENGINE_DIR)/EngineModule.cpp -o $(OBJ_DIR)/EngineModule.o

$(OBJ_DIR)/DoorsModule.o: $(DOORS_DIR)/DoorsModule.cpp
	$(CXX) $(CFLAGS) -c $(DOORS_DIR)/DoorsModule.cpp -o $(OBJ_DIR)/DoorsModule.o

$(OBJ_DIR)/HVACModule.o: $(HVAC_DIR)/HVACModule.cpp
	$(CXX) $(CFLAGS) -c $(HVAC_DIR)/HVACModule.cpp -o $(OBJ_DIR)/HVACModule.o

$(OBJ_DIR)/ECU.o: $(UTILS_DIR)/ECU.cpp
	$(CXX) $(CFLAGS) -c $(UTILS_DIR)/ECU.cpp -o $(OBJ_DIR)/ECU.o

$(OBJ_DIR)/GenerateFrames.o: $(UTILS_DIR)/GenerateFrames.cpp
	$(CXX) $(CFLAGS) -c $(UTILS_DIR)/GenerateFrames.cpp -o $(OBJ_DIR)/GenerateFrames.o

$(OBJ_DIR)/HandleFrames.o: $(UTILS_DIR)/HandleFrames.cpp
	$(CXX) $(CFLAGS) -c $(UTILS_DIR)/HandleFrames.cpp -o $(OBJ_DIR)/HandleFrames.o

$(OBJ_DIR)/CreateInterface.o: $(UTILS_DIR)/CreateInterface.cpp
	$(CXX) $(CFLAGS) -c $(UTILS_DIR)/CreateInterface.cpp -o $(OBJ_DIR)/CreateInterface.o

$(OBJ_DIR)/DiagnosticSessionControl.o: $(UDS_DIR)/diagnostic_session_control/src/DiagnosticSessionControl.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/diagnostic_session_control/src/DiagnosticSessionControl.cpp -o $(OBJ_DIR)/DiagnosticSessionControl.o

$(OBJ_DIR)/ReceiveFrames.o: $(UTILS_DIR)/ReceiveFrames.cpp
	$(CXX) $(CFLAGS) -c $(UTILS_DIR)/ReceiveFrames.cpp -o $(OBJ_DIR)/ReceiveFrames.o

$(OBJ_DIR)/ReadDataByIdentifier.o: $(UDS_DIR)/read_data_by_identifier/src/ReadDataByIdentifier.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/read_data_by_identifier/src/ReadDataByIdentifier.cpp -o $(OBJ_DIR)/ReadDataByIdentifier.o

$(OBJ_DIR)/WriteDataByIdentifier.o: $(UDS_DIR)/write_data_by_identifier/src/WriteDataByIdentifier.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/write_data_by_identifier/src/WriteDataByIdentifier.cpp -o $(OBJ_DIR)/WriteDataByIdentifier.o

$(OBJ_DIR)/EcuReset.o: $(UDS_DIR)/ecu_reset/src/EcuReset.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/ecu_reset/src/EcuReset.cpp -o $(OBJ_DIR)/EcuReset.o

$(OBJ_DIR)/SecurityAccess.o: $(UDS_DIR)/authentication/src/SecurityAccess.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/authentication/src/SecurityAccess.cpp -o $(OBJ_DIR)/SecurityAccess.o

$(OBJ_DIR)/TesterPresent.o: $(UDS_DIR)/tester_present/src/TesterPresent.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/tester_present/src/TesterPresent.cpp -o $(OBJ_DIR)/TesterPresent.o

$(OBJ_DIR)/ReadMemoryByAddress.o: $(UDS_DIR)/read_memory_by_address/src/ReadMemoryByAddress.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/read_memory_by_address/src/ReadMemoryByAddress.cpp -o $(OBJ_DIR)/ReadMemoryByAddress.o

$(OBJ_DIR)/MCUModule_ReceiveFrames.o: $(MCU_DIR)/ReceiveFrames.cpp
	$(CXX) $(CFLAGS) -c $(MCU_DIR)/ReceiveFrames.cpp -o $(OBJ_DIR)/MCUModule_ReceiveFrames.o

$(OBJ_DIR)/ReadDtcInformation.o: $(UDS_DIR)/read_dtc_information/src/ReadDtcInformation.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/read_dtc_information/src/ReadDtcInformation.cpp -o $(OBJ_DIR)/ReadDtcInformation.o

$(OBJ_DIR)/RequestUpdateStatus.o: $(OTA_DIR)/request_update_status/src/RequestUpdateStatus.cpp
	$(CXX) $(CFLAGS) -c $(OTA_DIR)/request_update_status/src/RequestUpdateStatus.cpp -o $(OBJ_DIR)/RequestUpdateStatus.o

$(OBJ_DIR)/RequestTransferExit.o: $(OTA_DIR)/request_transfer_exit/src/RequestTransferExit.cpp
	$(CXX) $(CFLAGS) -c $(OTA_DIR)/request_transfer_exit/src/RequestTransferExit.cpp -o $(OBJ_DIR)/RequestTransferExit.o

$(OBJ_DIR)/RequestDownload.o: $(OTA_DIR)/request_download/src/RequestDownload.cpp
	$(CXX) $(CFLAGS) -c $(OTA_DIR)/request_download/src/RequestDownload.cpp -o $(OBJ_DIR)/RequestDownload.o

$(OBJ_DIR)/MemoryManager.o: $(UTILS_DIR)/MemoryManager.cpp
	$(CXX) $(CFLAGS) -c $(UTILS_DIR)/MemoryManager.cpp -o $(OBJ_DIR)/MemoryManager.o

$(OBJ_DIR)/FileManager.o: $(UTILS_DIR)/FileManager.cpp
	$(CXX) $(CFLAGS) -c $(UTILS_DIR)/FileManager.cpp -o $(OBJ_DIR)/FileManager.o
	
$(OBJ_DIR)/TransferData.o: $(OTA_DIR)/transfer_data/src/TransferData.cpp
	$(CXX) $(CFLAGS) -c $(OTA_DIR)/transfer_data/src/TransferData.cpp -o $(OBJ_DIR)/TransferData.o

$(OBJ_DIR)/Logger.o: $(UTILS_DIR)/Logger.cpp
	$(CXX) $(CFLAGS) -c $(UTILS_DIR)/Logger.cpp -o $(OBJ_DIR)/Logger.o

$(OBJ_DIR)/ClearDtc.o: $(UDS_DIR)/clear_dtc/src/ClearDtc.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/clear_dtc/src/ClearDtc.cpp -o $(OBJ_DIR)/ClearDtc.o

$(OBJ_DIR)/RoutineControl.o: $(UDS_DIR)/routine_control/src/RoutineControl.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/routine_control/src/RoutineControl.cpp -o $(OBJ_DIR)/RoutineControl.o

$(OBJ_DIR)/AccessTimingParameter.o: $(UDS_DIR)/access_timing_parameters/src/AccessTimingParameter.cpp
	$(CXX) $(CFLAGS) -c $(UDS_DIR)/access_timing_parameters/src/AccessTimingParameter.cpp -o $(OBJ_DIR)/AccessTimingParameter.o
  
$(OBJ_DIR)/NegativeResponse.o: $(UTILS_DIR)/NegativeResponse.cpp
	$(CXX) $(CFLAGS) -c $(UTILS_DIR)/NegativeResponse.cpp -o $(OBJ_DIR)/NegativeResponse.o


#----------------------------------------------------Clean up--------------------------------------------------------
.PHONY: clean
clean:
	@echo "Cleaning up..."
	rm -f $(FINAL)
	rm -rf $(OBJ_DIR)
