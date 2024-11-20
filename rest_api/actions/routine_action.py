from actions.base_actions import *
from configs.data_identifiers import *
import time

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
        frame_response = self._collect_response_raw(api_target_id, REQUEST_UPDATE_STATUS)

        if frame_response is not None:
            if frame_response[1] != 0x7F:  
                if frame_response[2] != 0x40 :
                    log_error_message(logger, "[Verify Software] Not Ready.")
                    return self._to_json_status("Not Ready", 1)
                
                self.control_frame_verify_data(api_target_id)
                frame_response = self._collect_response_raw(api_target_id, ROUTINE_CONTROL)

                if frame_response is not None:
                    if frame_response[1] != 0x7F: 
                        log_info_message(logger, "[Verify Software] Verify Software is Successful")
                        return self._to_json_status("Succes", 0)
                    else:
                        log_error_message(logger, "[Verify Software] Verify data not succesful, check logs")
                        return self._to_json_status("Verify data not succesful, check logs", 1) 
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
            address = int(address,16)
            nrBytes = int(nrBytes,16)

            self._erase_data(api_target_id, address, nrBytes, True)
            frame_response = self._collect_response_raw(api_target_id,ROUTINE_CONTROL)

            if frame_response is not None:
                if frame_response[1] == ROUTINE_CONTROL+0x40:  # frame response == 71 
                    
                    self._erase_data(api_target_id, address, nrBytes, False)
                    frame_response = self._collect_response_raw(api_target_id,ROUTINE_CONTROL)

                    if frame_response is not None:
                        if frame_response[1] == ROUTINE_CONTROL+0x40:   # frame response == 71 
                            log_info_message(logger, f"[Erase Memory] Erase Memory from {address}, nr bytes: {nrBytes} is Successful")
                            return self._to_json_status("Succes Erase Memory", 0)
                        else:
                            log_error_message(logger, "[Erase Memory] Erase Memory not successful, nr bytes is too large")
                            return self._to_json_status("Erase Memory not successful, nr bytes is too large", 1) 
                else:
                    log_error_message(logger, "[Erase Memory] The address is incorrect")
                    return self._to_json_status("Address Incorrect", 1)

            log_error_message(logger, "[Erase Memory] No Message received from CAN ")
            return self._to_json_status("The MCU/ECU is not responding", 1)
