from actions.base_actions import *
from configs.data_identifiers import *


class ReadAccessTiming(Action):
    """ curl -X POST http://127.0.0.1:5000/api/read_access_timing \
    -H "Content-Type: application/json" \
    -d '{
        "sub_funct": "timing"
    }'
    """
    def _read_timing_info(self, ecu_id, sub_funct=1):
        """
        Reads timing parameters of the ECUs.
        Args:
        - id: The identifier of the target ECU or device.
        - sub_funct: The sub-function code to specify which timing parameters to read.
        Returns:
        - A dictionary with the status, message, and timing parameters (if successful).
        """

        try:
            id = self.my_id * 0x100 + ecu_id

            log_info_message(logger, "Changing session to programming")
            self.access_timing_parameters(id, sub_funct)
            frame_response = self._passive_response(ACCESS_TIMING_PARAMETERS, "Error reading timing parameters")

            if len(frame_response.data) < 4:
                return {
                    "status": "error",
                    "message": "Unexpected or insufficient response length while reading timing parameters"
                }

            if frame_response.data[1] == 0x7F:
                nrc_msg = frame_response.data[3]
                sid_msg = frame_response.data[2]
                negative_response = self.handle_negative_response(nrc_msg, sid_msg)
                return {
                    "status": "error",
                    "message": "Negative response received while reading timing parameters",
                    "negative_response": negative_response
                }

            if frame_response.data[1] == 0xC3:
                log_info_message(logger, "Timing parameters read successfully")

                timing_values = list(frame_response.data[3:])

                if len(timing_values) >= 4:
                    value1_decimal = (timing_values[0] << 8) + timing_values[1]
                    value2_decimal = (timing_values[2] << 8) + timing_values[3]

                    if sub_funct == 1:
                        timing_values_dict = {
                            "P2_MAX_TIME_DEFAULT": f"{value1_decimal} milliseconds",
                            "P2_STAR_MAX_TIME_DEFAULT": f"{value2_decimal} milliseconds"
                        }
                    elif sub_funct == 3:
                        timing_values_dict = {
                            "p2_max_time": f"{value1_decimal} milliseconds",
                            "p2_star_max_time": f"{value2_decimal} milliseconds"
                        }
                    else:
                        return {
                            "message": "Sub-function value not recognized"
                        }

                    return {
                        "message": "Timing parameters accessed successfully",
                        "timing_values": timing_values_dict
                    }
                else:
                    return {
                        "message": "Insufficient data length to read timing parameters"
                    }
            else:
                return {
                    "message": "Unexpected response while reading timing parameters"
                }
        except Exception as e:
            logger.error(f"Exception accessing timing parameters: {e}")
            return {"message": str(e)}


class WriteAccessTiming(Action):
    """ curl -X POST http://127.0.0.1:5000/api/write_timing \
    -H "Content-Type: application/json" \
    -d '{
        "p2_max": "50",
        "p2_star_max": "100"
    }'
    """
    def _write_timing_info(self, sub_function, ecu_id, timing_values={}):
        """
        Writes timing parameters to the ECU.

        Args:
        - id: The identifier of the target ECU or device.
        - timing_values: A dictionary containing the timing values to be written.

        Returns:
        - A dictionary with the status and message.
        """
        try:
            id = (self.my_id << 8) + ecu_id
            if sub_function == 2:
                self.write_timming_parameters(id, sub_function)
            else:
                p2_max = timing_values.get("p2_max", 0)
                p2_star_max = timing_values.get("p2_star_max", 0)
                self.write_timming_parameters(id, sub_function, p2_max, p2_star_max)

            frame_response = self._passive_response(ACCESS_TIMING_PARAMETERS, "Error writing timing parameters")

            if frame_response.data[1] == 0xC3:
                log_info_message(logger, "Timing parameters written successfully")

                if sub_function == 2:
                    return {
                        "message": "Timing parameters reset successfully",
                    }
                else:
                    return {
                        "message": "Timing parameters written successfully",
                        "written_values": {
                            "New P2 Max Time": p2_max,
                            "New P2 Star Max": p2_star_max
                        }
                    }
            else:
                return {
                    "message": "Unexpected response while writing timing parameters"
                }
        except Exception as e:
            logger.error(f"Exception writing timing parameters: {e}")
            return {"message": str(e)}
