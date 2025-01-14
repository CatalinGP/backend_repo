from actions.base_actions import *
from configs.data_identifiers import *
import time

OTA_UPDATE_STATES = {
    0x00: "IDLE",
    0x10: "INIT",
    0x20: "READY",
    0x30: "PROCESSING",
    0x31: "PROCESSING_TRANSFER_COMPLETE",
    0x32: "PROCESSING_TRANSFER_FAILED",
    0x40: "WAIT",
    0x41: "WAIT_DOWNLOAD_COMPLETED",
    0x42: "WAIT_DOWNLOAD_FAILED",
    0x50: "VERIFY",
    0x51: "VERIFY_COMPLETE",
    0x52: "VERIFY_FAILED",
    0x60: "ACTIVATE",
    0x61: "ACTIVATE_INSTALL_COMPLETE",
    0x62: "ACTIVATE_INSTALL_FAILED",
    0x70: "ERROR"
}


class ToJSON():
    def _to_json(self, status: str, no_errors):
        response = {
            "status_download": status,
            "errors": no_errors,
            "time_stamp": datetime.datetime.now().isoformat()
        }
        return response


class Updates(Action):
    """
    Update class for managing software updates on an Electronic Control Unit (ECU).

    Attributes:
    - my_id: Identifier for the device initiating the update.
    - id_ecu: Identifier for the specific ECU being updated.
    - g: Instance of GenerateFrame for generating CAN bus frames.
    """

    def update_to(self, type, version, id):
        """
        Method to update the software of the ECU to a specified version.

        Args:
        - version: Desired version of the software.
        - data: Optional data to be used in the update process (default is an empty list).

        Returns:
        - JSON response indicating the status of the update and any errors encountered.

        Raises:
        - CustomError: If the current software version matches the desired version,
          indicating that the latest version is already installed.

        curl -X POST http://127.0.0.1:5000/api/update_to_version -H "Content-Type: application/json" -d '{
            "update_file_type": "zip",
            "update_file_version": "2.3",
            "ecu_id": "0x11"
        }'
        """
        try:
            # CAN ID used for OTA Initialisation Routine
            self.id = (int(id, 16) << 16) + (self.my_id << 8) + self.id_ecu[0]
            log_info_message(logger, "Changing MCU to session to extended diagonstic mode")
            self.session_control(self.id, sub_funct=0x03)
            self._passive_response(SESSION_CONTROL, "Error changing session control")

            log_info_message(logger, f"Changing ECU {int} to session to extended diagonstic mode")
            ses_id = (0x00 << 16) + (self.my_id << 8) + int(id, 16)
            self.session_control(ses_id, sub_funct=0x03)
            self._passive_response(SESSION_CONTROL, "Error changing session control")

            log_info_message(logger, "Authenticating...")
            self._authentication(self.my_id * 0x100 + self.id_ecu[0])  # -> security only to MCU

            log_info_message(logger, "Reading data from battery")
            current_version = self._verify_version()
            if current_version == version:
                response_json = ToJSON()._to_json(f"Version {version} already installed", 0)
                return response_json

            # Check if another OTA update is in progress ( OTA_STATE is not IDLE)

            log_info_message(logger, "Downloading... Please wait")
            self._download_data(type, version, id)
            log_info_message(logger, "Download finished, restarting ECU...")

            # Reset the ECU to apply the update
            # self.id = (self.my_id * 0x100) + int(ecu_id, 16)
            # self.ecu_reset(self.id)
            # self._passive_response(RESET_ECU, "Error trying to reset ECU") # ToDo reactivate when using real hardware

            # Add a delay to wait until the ECU completes the reset process
            # log_info_message(logger, "Waiting until ECU is up")
            time.sleep(1)

            # Check for errors in the updated ECU
            # log_info_message(logger, "Checking for errors..")
            # no_errors = self._check_errors()
            no_errors = "No errors."

            # Generate a JSON response indicating the success of the update
            response_json = ToJSON()._to_json("downloaded", no_errors)

            log_info_message(logger, "Sending JSON")
            return response_json

        except CustomError as e:
            return e.message

    def _download_data(self, type, version, id):
        """
        Request Sid = 0x34
        Response Sid = 0x74

        Request frame format:
        { pci, sid, data_format_identifier, adress_and_length_format_identifier, memory_adress, memory_size, version}

        Pci = 1 byte
        Sid = 1 byte
        Data_format_identifier = 1 byte
        0x00 means that no compression/encryption method is used
        0x01 means that only encryption is used
        0x10 means that only compression is used
        0x11 means that both encryption and compression are used
            -> for now use 0x00 because compression/encryption are not defined
            -> we can define more values if needed
        Address and Length format identifier 1-byte
        (bit 4- bit 7) denotes the number of bytes of the memory size parameter and the lower nibble
        (bit 0- bit 3) denotes the number of bytes of the memory address parameter.
        Memory address = min 1 byte -  max 16 bytes
        Memory size = min 1 byte - max 16 bytes
        Version = 1 byte
        0x00 => 0b 0000 0000
                                    ^^^^ these bits are used to determine update
                    iteration(ranges between 0 and 15)
                            ^^^^ these bits are used to determine update
            version(ranges between 0 and 15)
            -> 0000 0000 -> version 0.0
            -> 0000 0001 -> version 0.1
            …
            -> 0010 0000 -> version 2.0
            -> 0010 0001 -> version 2.1
            …

        Important note: This action requires to have the mcu module running in python virtual env and
        create locally a virtual partition used for download.
        -> search/change "/dev/loopXX" in RequestDownload.cpp, MemoryManager.cpp; (Depends which partition is attributed)
        """
        # self.id is: target(1b)-api(1b)-mcu(1b)
        self.init_ota_routine(self.id, version=version)
        frame_response = self._passive_response(ROUTINE_CONTROL, "Error initialzing OTA")

        mem_size = frame_response.data[5]

        api_target_id = (0x00 << 16) + (0xFA << 8) + int(id, 16)
        self.request_download(api_target_id,
                              data_format_identifier=type,
                              memory_address=0x0800,
                              memory_size=mem_size,
                              version=version)
        frame_response = self._passive_response(REQUEST_DOWNLOAD, "Error requesting download")
        time.sleep(1)
        if (api_target_id & 0xFF) != 0x10:

            transfer_data_counter = 0x01
            while True:
                self.transfer_data(api_target_id, transfer_data_counter)
                frame_response = self._passive_response(TRANSFER_DATA, "Error transfering data")
                if frame_response.data[1] != 0x76:
                    log_info_message(logger, "Transfer data failed")
                    break
                else:
                    ota_state = frame_response.data[3]
                    if ota_state == 0x31:
                        log_info_message(logger, "Data has been transferred succesfully")
                        break
                if transfer_data_counter == 255:
                    transfer_data_counter = 0
                transfer_data_counter += 0x01

            # self.request_transfer_exit(api_target_id)
        self.control_frame_verify_data(api_target_id)
        frame_response = self._passive_response(ROUTINE_CONTROL, "Error at verify data routine.")
        if frame_response.data[1] != 0x71:
            log_info_message(logger, "Update failed at verification step.")
            return
        self.control_frame_write_file(api_target_id)
        frame_response = self._passive_response(ROUTINE_CONTROL, "Error at writting file routine.")
        if frame_response.data[1] != 0x71:
            log_info_message(logger, "Update failed at writting file step.")
            return
        self.control_frame_install_updates(api_target_id)
        frame_response = self._passive_response(ROUTINE_CONTROL, "Error at install routine.")
        if frame_response.data[1] != 0x71:
            log_info_message(logger, "Update failed at install step.")

    def _verify_version(self):
        """
        Private method to verify if the current software version matches the desired version.

        Args:
        - version: Desired version of the software.

        Raises:
        - CustomError: If the current software version matches the desired version,
          indicating that the latest version is already installed.
        """
        log_info_message(logger, "Reading current version")

        current_version = self._read_by_identifier(self.id, IDENTIFIER_SYSTEM_SUPPLIER_ECU_SOFTWARE_VERSION_NUMBER)
        return current_version

    def _check_errors(self):
        """
        Private method to check for Diagnostic Trouble Codes (DTCs) in the updated ECU.

        Returns:
        - Number of errors (DTCs) found in the ECU.

        Raises:
        - CustomError: If any error occurs during the error checking process.
        """
        self.request_read_dtc_information(self.id, 0x01, 0x01)
        response = self._passive_response(READ_DTC, "Error reading DTC")

        if response is not None:
            number_of_dtc = response.data[5]
            log_info_message(logger, f"There are {number_of_dtc} errors found after download")
            return number_of_dtc

    def get_ota_update_state(self, value):
        """
        Takes a hexadecimal value as input, looks it up in the OTA_UPDATE_STATES dictionary,
        and returns the corresponding state. If the state is READY, it continues execution.

        :param value: Hexadecimal value (e.g., 0x10)
        :return: The state name if found, otherwise an error message.
        """
        start_time = time.time()

        while True:
            state = OTA_UPDATE_STATES.get(value)

            if state is not None:
                log_info_message(logger, f"State found: {state}.")
                return state

            elapsed_time = time.time() - start_time
            if elapsed_time >= 5:
                log_info_message(logger, f"Timeout reached. State {hex(value)} is still invalid.")
                return "INVALID_STATE_TIMEOUT"

            time.sleep(1)
