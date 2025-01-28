from actions.base_actions import *
from configs.data_identifiers import *
import time

OTA_UPDATE_STATES = {
    0x00: "IDLE",				# Initial state of the FOTA Handler after the ECU startup procedure
    0x10: "INIT",				# The FOTA Handler is initialized, and Dcm is set into the correct state (in Dcm FOTA session and security access has been granted) */
	0x20: "WAIT",				# The FOTA Handler has successfully processed the last received data chunk, returned the Dcm callout function, and is waiting for the next data chunk */
	0x21: "WAIT_DOWNLOAD_COMPLETED",	# The FOTA Handler has successfully processed the last received data chunk, returned the Dcm callout function, and is waiting for the next data chunk */
	0x22: "WAIT_DOWNLOAD_FAILED",		# The FOTA Handler has successfully processed the last received data chunk, returned the Dcm callout function, and is waiting for the next data chunk */
	0x30: "PROCESSING",					# The FOTA Handler is triggered by the Dcm callout since a new chunk has been received and is processed in the callout context */
	0x31: "PROCESSING_TRANSFER_COMPLETE",	# The FOTA Handler is triggered by the Dcm callout since a new chunk has been received and is processed in the callout context */
	0x32: "PROCESSING_TRANSFER_FAILED",		# The FOTA Handler is triggered by the Dcm callout since a new chunk has been received and is processed in the callout context */
	0x40: "READY",							# All FOTA data chunks have been installed, activation procedure can be triggered */
	0x50: "VERIFY",						# Optional and implementer specific step, since the FOTA Target does not specify any details on the verification process */
	0x51: "VERIFY_COMPLETE",			# Optional and implementer specific step, since the FOTA Target does not specify any details on the verification process */
	0x52: "VERIFY_FAILED",				# Optional and implementer specific step, since the FOTA Target does not specify any details on the verification process */
	0x60: "ACTIVATE",					# FOTA installation has finished and received a respective service job from the FOTA Master that indicates the partition switch during the next boot process */
	0x61: "ACTIVATE_INSTALL_COMPLETE",	# FOTA installation has finished and received a respective service job from the FOTA Master that indicates the partition switch during the next boot process */
	0x62: "ACTIVATE_INSTALL_FAILED",	# FOTA installation has finished and received a respective service job from the FOTA Master that indicates the partition switch during the next boot process */
	0xFF: "ERROR"						# Optional and implementer specific. Reserved state for e.g., implementer specific error handling, which is not (yet) covered by the FOTA Target */
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

    def update_to(self, type, version, id, address):
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
            "update_file_version": "1.3",
            "ecu_id": "0x11"
        }'
        """
        try:
            # CAN ID used for OTA Initialisation Routine
            self.id = (int(id, 16) << 16) + (self.my_id << 8) + self.id_ecu[0]
            log_info_message(
                logger, "Changing MCU to session to extended diagonstic mode")
            self.session_control(self.id, sub_funct=0x03)
            self._passive_response(SESSION_CONTROL, "Error changing session control")

            ses_id = (0x00 << 16) + (self.my_id << 8) + int(id, 16)
            if int(id,16) != self.id_ecu[0]:
                log_info_message(
                    logger, f"Changing ECU {int} to session to extended diagonstic mode")
                self.session_control(ses_id, sub_funct=0x03)
                self._passive_response(SESSION_CONTROL, "Error changing session control")

            log_info_message(logger, "Authenticating...")
            # -> security only to MCU
            self._authentication(self.my_id * 0x100 + self.id_ecu[0])
            log_info_message(logger, f"Version to be updated {version}")
            current_version = self._verify_version(ses_id)
            log_info_message(logger, f"Curent version {current_version}")
            if current_version == version.replace(".", ""):
                response_json = ToJSON()._to_json(
                    f"Version {version} already installed", 0)
                log_info_message(logger, "Version {version} already installed")
                return response_json

            # Check if another OTA update is in progress ( OTA_STATE is not IDLE)

            log_info_message(logger, "Downloading... Please wait")
            self._download_data(type, version, id, address)
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

    def _download_data(self, type, version, id, address):
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
        -> search/change "/dev/loop13XX" in RequestDownload.cpp, MemoryManager.cpp; (Depends which partition is attributed)
        """
        # self.id is: target(1b)-api(1b)-mcu(1b)
        self.init_ota_routine(self.id, version=version)
        frame_response = self._passive_response(
            ROUTINE_CONTROL, "Error initialzing OTA")

        mem_size = frame_response.data[5]

        api_target_id = (0x00 << 16) + (0xFA << 8) + int(id, 16)
        hex_address = ''.join(address)
        
        self.request_download(api_target_id,
                              data_format_identifier=type,
                              memory_address=int(hex_address, 16),
                              memory_size=mem_size,
                              version=version)
        frame_response = self._passive_response(
            REQUEST_DOWNLOAD, "Error requesting download")
        time.sleep(1)
        if (api_target_id & 0xFF) != 0x10:

            transfer_data_counter = 0x01
            while True:
                self.transfer_data(api_target_id, transfer_data_counter)
                frame_response = self._passive_response(
                    TRANSFER_DATA, "Error transfering data")
                if frame_response.data[1] != 0x76:
                    log_info_message(logger, "Transfer data failed")
                    break
                else:
                    ota_state = frame_response.data[3]
                    if ota_state == 0x31:
                        log_info_message(
                            logger, "Data has been transferred succesfully")
                        break
                if transfer_data_counter == 255:
                    transfer_data_counter = 0
                transfer_data_counter += 0x01

            self.request_transfer_exit(api_target_id, True)
            frame_response = self._passive_response(REQUEST_TRANSFER_EXIT, "Error at transfer exit.")
            if frame_response.data[1] != 0x77:
                log_info_message(logger, "Update failed at transfer exit step.")
                return
            
        self.control_frame_verify_data(api_target_id)
        frame_response = self._passive_response(
            ROUTINE_CONTROL, "Error at verify data routine.")
        if frame_response.data[1] != 0x71:
            log_info_message(logger, "Update failed at verification step.")
            return
        self.control_frame_write_file(api_target_id)
        frame_response = self._passive_response(
            ROUTINE_CONTROL, "Error at writting file routine.")
        if frame_response.data[1] != 0x71:
            log_info_message(logger, "Update failed at writting file step.")
            return
        self.control_frame_install_updates(api_target_id)
        frame_response = self._passive_response(
            ROUTINE_CONTROL, "Error at install routine.")
        if frame_response.data[1] != 0x71:
            log_info_message(logger, "Update failed at install step.")
            return

    def _verify_version(self, ses_id):
        """
        Private method to verify if the current software version matches the desired version.

        Args:
        - version: Desired version of the software.

        Raises:
        - CustomError: If the current software version matches the desired version,
          indicating that the latest version is already installed.
        """
        log_info_message(logger, "Reading current version")

        current_version = self._read_by_identifier(
            ses_id, IDENTIFIER_SYSTEM_SUPPLIER_ECU_SOFTWARE_VERSION_NUMBER)
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
            log_info_message(
                logger, f"There are {number_of_dtc} errors found after download")
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
                log_info_message(
                    logger, f"Timeout reached. State {hex(value)} is still invalid.")
                return "INVALID_STATE_TIMEOUT"

            time.sleep(1)


    def get_ota_status(self, ecu_id):
        try:
            # Convert the hex string (e.g., "0x10") to an integer
            hex_value = int(ecu_id, 16)
            self.request_update_status(hex_value)
            frame_response = self._passive_response(REQUEST_UPDATE_STATUS, "Request OTA state failed.")

            return self.get_ota_update_state(frame_response.data[2])
        
        except ValueError:
            return ToJSON()._to_json(f"Request OTA state failed", 0)


            
    def change_ota_state(self, ecu_id, ota_status_value):
    
        id = (0x00 << 16) + (0xFA << 8) + 0x10
        self.write_data_by_identifier(id, 0XE001, [int(ota_status_value, 16)])
        self._passive_response(WRITE_BY_IDENTIFIER, f"Error writing {ota_status_value}")

        id = (0x00 << 16) + (0xFA << 8) + int(ecu_id, 16)
        if ecu_id != self.id_ecu[0]:
            self.write_data_by_identifier(id, 0XE001, [int(ota_status_value, 16)])
            self._passive_response(WRITE_BY_IDENTIFIER, f"Error writing {ota_status_value}")
        
        return True
        
    def transfer_data_to_ecu(self, ecu_id, address, data):
        id = (0x00 << 16) + (0xFA << 8) + int(ecu_id, 16)
        hex_address = ''.join(address)

        self.request_download(id,
                              data_format_identifier=0x00,
                              memory_address=int(hex_address, 16),
                              memory_size=0x01,
                              version=0x00)
        self._passive_response(REQUEST_DOWNLOAD, "Error requesting download")

        data = data[2:]
        transfer_data_counter = 0x01
        if len(data) <= 10:
            self.transfer_data(id, transfer_data_counter, int(data[:2], 16))
            self._passive_response(TRANSFER_DATA, "Error transfering data")
            data = data[2:]
            transfer_data_counter += 0x01

        while data:
            max_bytes = min(5, len(data) // 2)
            current_chunk = int(data[:max_bytes * 2], 16)
            
            if max_bytes < 5 or len(data) == 5:
                if self.change_ota_state(ecu_id, '0x31') == False:
                    log_info_message(logger, f"Transfer Data to Ecu failed at changing ota status to transfer complete")
                    return
                time.sleep(1)

            self.transfer_data(id, transfer_data_counter, current_chunk)
            self._passive_response(TRANSFER_DATA, "Error transfering data")

            if transfer_data_counter == 255:
                transfer_data_counter = 0
            transfer_data_counter += 0x01

            data = data[max_bytes * 2:]
