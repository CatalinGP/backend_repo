from actions.base_actions import *
from configs.data_identifiers import *
from actions.read_info_action import *


class Routine(Action):

    def _to_json_status(self, message: str, no_errors):
        response = {
            "Message": message,
            "errors": no_errors,
            "time_stamp": datetime.datetime.now().isoformat()
        }
        return response

    def verify_software(self, id):
        """
        Method to verify the software of the ECU/MCU.

        Args:
        - id: Desired MCU/ECU id.

        Returns:
        - JSON response indicating the status of the software and any errors encountered.

        Raises:
        - CustomError: If the Software Verify is completed successfully

        curl -X POST http://127.0.0.1:5000/api/verify_software -H "Content-Type: application/json" -d '{
            "ecu_id": "0x10"
        }'
        """

        api_target_id = (0x00 << 16) + (0xFA << 8) + int(id, 16)

        self.request_update_status(REQUEST_UPDATE_STATUS)
        frame_response = self._collect_response_raw(
            api_target_id, REQUEST_UPDATE_STATUS)

        if frame_response is not None:
            if frame_response[1] != 0x7F:
                if frame_response[2] != 0x40:
                    log_error_message(logger, "[Verify Software] Not Ready.")
                    return self._to_json_status("Not Ready", 1)

                self.control_frame_verify_data(api_target_id)
                frame_response = self._collect_response_raw(
                    api_target_id, ROUTINE_CONTROL)

                if frame_response is not None:
                    if frame_response[1] != 0x7F:
                        log_info_message(
                            logger, "[Verify Software] Verify Software is Successful")
                        return self._to_json_status("Succes", 0)
                    else:
                        log_error_message(
                            logger, "[Verify Software] Verify data not succesful, check logs")
                        return self._to_json_status(
                            "Verify data not succesful, check logs", 1)
            else:
                log_error_message(logger, "[Verify Software] Not Security Check")
                return self._to_json_status("Not Security Check", 1)

        log_error_message(logger, "[Verify Software] No Message received from CAN ")

        return self._to_json_status("The MCU/ECU is not responding", 1)

    def erase_memory_from_address(self, id, address, nrBytes):
        """
        Erases a specific memory region starting from a given address on the ECU/MCU.

        Args:
        - id (int): The ID of the target MCU/ECU to which the erase command will be sent.
        - address (int): The starting address of the memory region to be erased.
        - nrBytes (int): The number of bytes to erase starting from the provided address.


        Returns:
        - dict: A JSON-like dictionary response containing:
        - `status` (str): The result of the erase operation ("success" or "failure").
        - `message` (str): A detailed message about the operation's outcome.
        - `time_stamp` (str): The timestamp when the operation was completed.
        - `errors` (int): The number of errors encountered during the operation.

        Raises:
        - CustomError: If the erase operation fails due to communication issues, invalid parameters, or an error returned by the ECU/MCU.

        curl -X POST http://127.0.0.1:5000/api/erase_memory -H "Content-Type: application/json" -d '{
            "ecu_id": "0x10",
            "address": "0x0800",
            "nrBytes": "0x4"
        }'
    """

        api_target_id = (0x00 << 16) + (0xFA << 8) + int(id, 16)
        address = int(address, 16)
        nrBytes = int(nrBytes, 16)

        self._erase_data(api_target_id, address, nrBytes, True)
        frame_response = self._collect_response_raw(api_target_id, ROUTINE_CONTROL)

        if frame_response is not None:
            if frame_response[1] == ROUTINE_CONTROL + 0x40:  # frame response == 71

                self._erase_data(api_target_id, address, nrBytes, False)
                frame_response = self._collect_response_raw(
                    api_target_id, ROUTINE_CONTROL)

                if frame_response is not None:
                    if frame_response[1] == ROUTINE_CONTROL + \
                            0x40:   # frame response == 71
                        log_info_message(
                            logger, f"[Erase Memory] Erase Memory from {address}, nr bytes: {nrBytes} is Successful")
                        return self._to_json_status("Succes Erase Memory", 0)
                    else:
                        log_error_message(
                            logger, "[Erase Memory] Erase Memory not successful, nr bytes is too large")
                        return self._to_json_status(
                            "Erase Memory not successful, nr bytes is too large", 1)
            else:
                log_error_message(logger, "[Erase Memory] The address is incorrect")
                return self._to_json_status("Address Incorrect", 1)

        log_error_message(logger, "[Erase Memory] No Message received from CAN ")
        return self._to_json_status("The MCU/ECU is not responding", 1)

    def rollback_software(self, ecu_id):
        """
        Handles the CAN frame sending for rolling back software.

        Args:
        - ecu_id: ID of the ECU in hexadecimal (e.g., "0x10").
        """
        try:
            rollback_flag = self._read_by_identifier(0xFA * 0x100 + int(ecu_id, 16), 0xEEEE)
            if int(rollback_flag, 16) == 0:
                return {
                    "status": "success",
                    "message": f"Rollback not available for ECU ID: {ecu_id}"
                }

            # Compute CAN ID from ECU ID
            ecu_numeric_id = int(ecu_id, 16)  # Convert hex string to integer
            can_id = 0xFA00 + ecu_numeric_id  # Compute CAN ID (e.g., FA10 for 0x10)

            # Construct and send the CAN frame
            frame_data = [0x05, 0x31, 0x01, 0x05, 0x01, 0x00]
            log_info_message(
                logger, f"Sending rollback CAN frame with ID: {hex(can_id)} and data: {frame_data}")
            self.write_data_by_identifier(0xFA * 0x100 + int(ecu_id, 16), 0xEEEE, [0])
            self._passive_response(
                        WRITE_BY_IDENTIFIER, f"Error writing {0xEEEE}")
            self.send_frame(can_id, frame_data)

            # Wait for response and handle it
            frame_response = self._passive_response(
                ROUTINE_CONTROL, "Error during rollback.")
            if frame_response.data[1] != 0x71:
                log_info_message(logger, f"Rollback failed for ECU ID: {ecu_id}")
                raise CustomError(f"Rollback failed for ECU ID: {ecu_id}")

            log_info_message(logger, f"Rollback successful for ECU ID: {ecu_id}")

            # Return positive response
            return {
                "status": "success",
                "message": f"Rollback completed successfully for ECU ID: {ecu_id}"
            }

        except ValueError:
            raise CustomError(f"Invalid ECU ID: {ecu_id}")

    def activate_software(self, ecu_id):
        """
        Handles the CAN frame sending for activating software.

        Args:
        - ecu_id: ID of the ECU in hexadecimal (e.g., "0x10").
        """
        try:
            # Compute CAN ID from ECU ID
            ecu_numeric_id = int(ecu_id, 16)  # Convert hex string to integer
            can_id = 0xFA00 + ecu_numeric_id  # Compute CAN ID (e.g., FA10 for 0x10)

            # Construct and send the CAN frame
            frame_data = [0x05, 0x31, 0x01, 0x06, 0x01, 0x00]
            log_info_message(
                logger, f"Sending CAN frame with ID: {hex(can_id)} and data: {frame_data}")
            self.send_frame(can_id, frame_data)

            # Wait for response and handle it
            frame_response = self._passive_response(
                ROUTINE_CONTROL, "Error during activation.")
            if frame_response.data[1] != 0x71:
                log_info_message(logger, f"Activation failed for ECU ID: {ecu_id}")
                raise CustomError(f"Activation failed for ECU ID: {ecu_id}")

            log_info_message(logger, f"Activation successful for ECU ID: {ecu_id}")

            # Return positive response
            return {
                "status": "success",
                "message": f"Activation completed successfully for ECU ID: {ecu_id}"
            }

        except ValueError:
            raise CustomError(f"Invalid ECU ID: {ecu_id}")

    def ota_init(self, target, version):
        """
        Handles the CAN frame sending for initialise OTA
        curl -X POST http://127.0.0.1:5000/api/ota_init -H "Content-Type: application/json" -d '{
            "target": "0x11",
            "version": "1.2"
        }'


        Args:
        - target: ID of the ECU or MCU in hexadecimal (e.g., "0x10").
        - version: version of software in hexadecimal("00" to "FF")
        """
        try:
            # Compute CAN ID from ECU ID
            can_id = (int(target, 16) << 16) + (self.my_id << 8) + self.id_ecu[0]
            if isinstance(version, str):
                if '.' not in version:
                    version += '.0'
                major, minor = map(int, version.split('.'))
                major -= 1
                if major < 0 or major > 15 or minor < 0 or minor > 15:
                    raise ValueError(
                        f"Invalid version: {version}. Major and minor must be between 0 and 15."
                    )
                version_byte = (major << 4) | minor  # Encode directly without reduction
            elif isinstance(version, int):
                # Assume the int is already in the correct format
                version_byte = version
            else:
                raise ValueError(f"Invalid version format: {version}")

            # Construct and send the CAN frame
            frame_data = [0x05, 0x31, 0x01, 0x02, 0x01, version_byte]
            log_info_message(
                logger,
                f"Sending initialise OTA CAN frame with ID: {hex(can_id)} and data: {frame_data}")
            self.send_frame(can_id, frame_data)

            # Wait for response and handle it
            frame_response = self._passive_response(
                ROUTINE_CONTROL, "Error during initialise.")
            if frame_response.data[1] != 0x71:
                log_info_message(
                    logger, f"Initialise failed for ECU ID: {target} with version {version}")
                raise CustomError(
                    f"Initialise failed for ECU ID: {target} with version {version}")

            log_info_message(
                logger, f"Initialise OTA successful for ECU ID: {target} with version {version}")

            # Return positive response
            return {
                "status": "success",
                "message": f"Initialise OTA completed successfully for ECU ID: {target} with version {version}"
            }

        except ValueError:
            raise CustomError(f"Invalid ECU ID: {target} with version {version}")

    def write_to_file(self, ecu_id):
        """
         This routine reads the data from a memory address and write the data in zip
         file “main_battery_new.zip” and unzip it. This is called after the verify routine
         is successfully.

        curl -X POST http://127.0.0.1:5000/api/write_to_file -H "Content-Type: application/json" -d '{
            "ecu_id": "0x11"
        }'
        Args:
        - target: ID of the ECU or MCU in hexadecimal (e.g., "0x10").
        """
        try:
            # Compute CAN ID from ECU ID
            ecu_numeric_id = int(ecu_id, 16)
            can_id = 0xFA00 + ecu_numeric_id
            # Construct and send the CAN frame
            frame_data = [0x05, 0x31, 0x01, 0x03, 0x01, 0x00]
            log_info_message(
                logger, f"Sending CAN frame with ID: {hex(can_id)} and data: {frame_data}")
            self.send_frame(can_id, frame_data)

            # Wait for response and handle it
            frame_response = self._passive_response(
                ROUTINE_CONTROL, "Error during write to file.")
            if frame_response.data[1] != 0x71:
                log_info_message(logger, f"Write to file failed for ECU ID: {ecu_id}")
                raise CustomError(f"Write to file failed for ECU ID: {ecu_id}")

            log_info_message(logger, f"Write to file successful for ECU ID: {ecu_id}")

            # Return positive response
            return {
                "status": "success",
                "message": f"Write to file completed successfully for ECU ID: {ecu_id}"
            }
        except ValueError:
            raise CustomError(f"Invalid ECU ID: {ecu_id}")
