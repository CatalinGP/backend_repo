import HtmlTestRunner
from routes.api import api_bp
from flask import Flask
from unittest import TestCase
from unittest.mock import *
import unittest
import os
import sys
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..',))
sys.path.append(PROJECT_ROOT)


class TestAPIReads(TestCase):
    def setUp(self):
        self.app = Flask(__name__)
        self.app.register_blueprint(api_bp)
        self.client = self.app.test_client()
        self.app.testing = True

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_battery(self, mock_read_to_battery):

        mock_read_to_battery.return_value = {
            "battery_level": 80,
            "voltage": 10,
            "percentage": 2,
            "battery_state_of_charge": "Fully charged",
            "time_stamp": "2024-10-31T12:34:56.789123"
        }
        fake_json = mock_read_to_battery.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get("/read_info_battery?is_manual_flow=false")
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json["battery_level"],
            fake_json["battery_level"],
            "The battery_level is incorrect")
        self.assertEqual(
            response.json["voltage"],
            fake_json["voltage"],
            "The voltage is not correct")
        self.assertEqual(
            response.json["percentage"],
            fake_json["percentage"],
            "The percentage is not correct")
        self.assertEqual(
            response.json["battery_state_of_charge"],
            fake_json["battery_state_of_charge"],
            "The battery_state_of_charge is not correct")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_battery2(self, mock_read_to_battery):

        mock_read_to_battery.return_value = {
            'error': "The 'is_manual_flow' flag cannot be an empty string. Please provide "
            "the values 'true' or 'false'"}
        fake_json = mock_read_to_battery.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get("/read_info_battery?is_manual_flow=")
        self.assertEqual(
            response.status_code,
            400,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The battery_level is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_battery3(self, mock_read_to_battery):

        mock_read_to_battery.return_value = {
            'error': "The 'is_manual_flow' flag must be either 'true' or 'false'. You "
            "provided: 'ds'"}
        fake_json = mock_read_to_battery.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get("/read_info_battery?is_manual_flow=ds")
        self.assertEqual(
            response.status_code,
            400,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The battery_level is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_battery4(self, mock_read_to_battery):

        mock_read_to_battery.return_value = {
            "battery_level": 80,
            "voltage": 10,
            "percentage": 2,
            "battery_state_of_charge": "Fully charged",
            "time_stamp": "2024-10-31T12:34:56.789123"
        }
        fake_json = mock_read_to_battery.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get("/read_info_battery?is_manual_flow=true")
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json["battery_level"],
            fake_json["battery_level"],
            "The battery_level is incorrect")
        self.assertEqual(
            response.json["voltage"],
            fake_json["voltage"],
            "The voltage is not correct")
        self.assertEqual(
            response.json["percentage"],
            fake_json["percentage"],
            "The percentage is not correct")
        self.assertEqual(
            response.json["battery_state_of_charge"],
            fake_json["battery_state_of_charge"],
            "The battery_state_of_charge is not correct")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_battery5(self, mock_read_to_battery):

        mock_read_to_battery.return_value = {
            'error': "Invalid parameter 'battery_leve'. Use /get_identifiers to see valid "
            'parameters.'}
        fake_json = mock_read_to_battery.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get(
            "/read_info_battery?is_manual_flow=false&item=battery_leve")
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The battery_level is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_battery6(self, mock_read_to_battery):

        mock_read_to_battery.return_value = {
            'battery_level': 80,
            'time_stamp': '2024-11-12T11:07:13.146058'}
        fake_json = mock_read_to_battery.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get(
            "/read_info_battery?is_manual_flow=false&item=battery_level")
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json['battery_level'],
            fake_json['battery_level'],
            "The battery_level is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_doors(self, mock_read_to_doors):

        mock_read_to_doors.return_value = {
            "door": "Closed",
            "passenger": "Unlocked",
            "passenger_lock": "Unlocked",
            "ajar": "No warning",
            "time_stamp": "2024-10-31T12:34:56.789123"
        }
        fake_json = mock_read_to_doors.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_doors?is_manual_flow=false')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json["door"],
            fake_json["door"],
            "The door status is incorrect")
        self.assertEqual(
            response.json["passenger"],
            fake_json["passenger"],
            "The passenger is not correct")
        self.assertEqual(
            response.json["passenger_lock"],
            fake_json["passenger_lock"],
            "The passenger_lock is not correct")
        self.assertEqual(
            response.json["ajar"],
            fake_json["ajar"],
            "The ajar is not correct")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_doors2(self, mock_read_to_doors):

        mock_read_to_doors.return_value = {
            'error': "The 'is_manual_flow' flag cannot be an empty string. Please provide "
            "the values 'true' or 'false'"}
        fake_json = mock_read_to_doors.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_doors?is_manual_flow=')
        self.assertEqual(
            response.status_code,
            400,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json)

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_doors3(self, mock_read_to_doors):

        mock_read_to_doors.return_value = {
            'error': "The 'is_manual_flow' flag must be either 'true' or 'false'. You "
            "provided: 'ds'"}
        fake_json = mock_read_to_doors.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_doors?is_manual_flow=ds')
        self.assertEqual(
            response.status_code,
            400,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json)

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_doors4(self, mock_read_to_doors):

        mock_read_to_doors.return_value = {
            "door": "Closed",
            "passenger": "Unlocked",
            "passenger_lock": "Unlocked",
            "ajar": "No warning",
            "time_stamp": "2024-10-31T12:34:56.789123"
        }
        fake_json = mock_read_to_doors.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_doors?is_manual_flow=true')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        # self.assertEqual(response.json, fake_json, "The door status is incorrect")
        self.assertEqual(
            response.json["door"],
            fake_json["door"],
            "The door status is incorrect")
        self.assertEqual(
            response.json["passenger"],
            fake_json["passenger"],
            "The passenger is not correct")
        self.assertEqual(
            response.json["passenger_lock"],
            fake_json["passenger_lock"],
            "The passenger_lock is not correct")
        self.assertEqual(
            response.json["ajar"],
            fake_json["ajar"],
            "The ajar is not correct")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_doors5(self, mock_read_to_doors):

        mock_read_to_doors.return_value = {
            'error': "Invalid parameter 'soor'. Use /get_identifiers to see valid "
            'parameters.'}
        fake_json = mock_read_to_doors.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_doors?is_manual_flow=true&item=soor')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The door status is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_doors6(self, mock_read_to_doors):

        mock_read_to_doors.return_value = {
            "door": "Closed", "time_stamp": "2024-11-12T10:41:51.457556"}
        fake_json = mock_read_to_doors.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_doors?is_manual_flow=true&item=door')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json["door"],
            fake_json["door"],
            "The door status is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_engine(self, mock_read_to_engine):
        mock_read_to_engine.return_value = {
            "engine_load": 80,
            "engine_rpm": 64,
            "coolant_temperature": 95,
            "fuel_level": 85,
            "fuel_pressure": 50,
            "intake_air_temperature": 25,
            "oil_temperature": 70,
            "throttle_position": 65,
            "vehicle_speed": 120,
            "time_stamp": "2024-10-31T12:34:56.789123"
        }
        fake_json = mock_read_to_engine.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_engine?is_manual_flow=false')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json["engine_rpm"],
            fake_json["engine_rpm"],
            "The engine_rpm is incorrect")
        self.assertEqual(
            response.json["coolant_temperature"],
            fake_json["coolant_temperature"],
            "The voltage is not correct")
        self.assertEqual(
            response.json["fuel_level"],
            fake_json["fuel_level"],
            "The percentage is not correct")
        self.assertEqual(
            response.json["engine_load"],
            fake_json["engine_load"],
            "The engine_load is incorrect")
        self.assertEqual(
            response.json["fuel_pressure"],
            fake_json["fuel_pressure"],
            "The fuel_pressure is not correct")
        self.assertEqual(
            response.json["intake_air_temperature"],
            fake_json["intake_air_temperature"],
            "The intake_air_temperature is not correct")
        self.assertEqual(
            response.json["throttle_position"],
            fake_json["throttle_position"],
            "The engine_rpm is incorrect")
        self.assertEqual(
            response.json["oil_temperature"],
            fake_json["oil_temperature"],
            "The oil_temperature is not correct")
        self.assertEqual(
            response.json["vehicle_speed"],
            fake_json["vehicle_speed"],
            "The speed is not correct")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_engine2(self, mock_read_to_engine):
        mock_read_to_engine.return_value = {
            'error': "The 'is_manual_flow' flag cannot be an empty string. Please provide "
            "the values 'true' or 'false'"}
        fake_json = mock_read_to_engine.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_engine?is_manual_flow=')
        self.assertEqual(
            response.status_code,
            400,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The engine_rpm is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_engine3(self, mock_read_to_engine):
        mock_read_to_engine.return_value = {
            'error': "The 'is_manual_flow' flag must be either 'true' or 'false'. You "
            "provided: 'ds'"}
        fake_json = mock_read_to_engine.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_engine?is_manual_flow=ds')
        self.assertEqual(
            response.status_code,
            400,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The engine_rpm is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_engine4(self, mock_read_to_engine):
        mock_read_to_engine.return_value = {
            'message': 'ssue encountered during Read by ID',
            'negative_response': {
                'error_message': 'Security Access Denied',
                'nrc': '0x33',
                'service_description': 'Read by ID',
                'service_id': '0x22'}}
        fake_json = mock_read_to_engine.return_value
        response = self.client.post('/change_session', json={"sub_funct": 1})
        response = self.client.get('/read_info_engine?is_manual_flow=true')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        # self.assertEqual(response.json, fake_json, "The engine_rpm is incorrect")
        self.assertEqual(
            response.json["engine_rpm"],
            fake_json["engine_rpm"],
            "The engine_rpm is incorrect")
        self.assertEqual(
            response.json["coolant_temperature"],
            fake_json["coolant_temperature"],
            "The voltage is not correct")
        self.assertEqual(
            response.json["fuel_level"],
            fake_json["fuel_level"],
            "The percentage is not correct")
        self.assertEqual(
            response.json["engine_load"],
            fake_json["engine_load"],
            "The engine_load is incorrect")
        self.assertEqual(
            response.json["fuel_pressure"],
            fake_json["fuel_pressure"],
            "The fuel_pressure is not correct")
        self.assertEqual(
            response.json["intake_air_temperature"],
            fake_json["intake_air_temperature"],
            "The intake_air_temperature is not correct")
        self.assertEqual(
            response.json["throttle_position"],
            fake_json["throttle_position"],
            "The engine_rpm is incorrect")
        self.assertEqual(
            response.json["oil_temperature"],
            fake_json["oil_temperature"],
            "The oil_temperature is not correct")
        self.assertEqual(
            response.json["vehicle_speed"],
            fake_json["vehicle_speed"],
            "The speed is not correct")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_engine5(self, mock_read_to_engine):
        mock_read_to_engine.return_value = {
            'error': "Invalid parameter 'engine_rpn'. Use /get_identifiers to see valid "
            'parameters.'}
        fake_json = mock_read_to_engine.return_value
        response = self.client.post('/change_session', json={"sub_funct": 1})
        response = self.client.get(
            '/read_info_engine?is_manual_flow=true&item=engine_rpn')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The engine_rpm is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_engine6(self, mock_read_to_engine):
        mock_read_to_engine.return_value = {
            'engine_rpm': 64, 'time_stamp': '2024-11-12T10:55:37.346299'}
        fake_json = mock_read_to_engine.return_value
        response = self.client.post('/change_session', json={"sub_funct": 1})
        response = self.client.get(
            '/read_info_engine?is_manual_flow=true&item=engine_rpm')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json["engine_rpm"],
            fake_json["engine_rpm"],
            "The engine_rpm is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_hvac(self, test_read_info_hvac):
        test_read_info_hvac.return_value = {'ambient_air_temperature': 50,
                                            'cabin_temperature': 20,
                                            'cabin_temperature_driver_set': 21,
                                            'fan_speed': 10,
                                            'hvac_modes': {'AC Status': True,
                                                           'Air Recirculation': False,
                                                           'Defrost': False,
                                                           'Front': True,
                                                           'Legs': False},
                                            'mass_air_flow': 5,
                                            'time_stamp': '2024-11-05T16:05:36.240350'}
        fake_json = test_read_info_hvac.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_hvac?is_manual_flow=false')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json['ambient_air_temperature'],
            fake_json['ambient_air_temperature'],
            "The data are incorrect")
        self.assertEqual(
            response.json['cabin_temperature'],
            fake_json['cabin_temperature'],
            "The data are incorrect")
        self.assertEqual(
            response.json['cabin_temperature_driver_set'],
            fake_json['cabin_temperature_driver_set'],
            "The data are incorrect")
        self.assertEqual(
            response.json['fan_speed'],
            fake_json['fan_speed'],
            "The data are incorrect")
        self.assertEqual(
            response.json['mass_air_flow'],
            fake_json['mass_air_flow'],
            "The data are incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_hvac2(self, mock_read_to_hvac):
        mock_read_to_hvac.return_value = {
            'error': "The 'is_manual_flow' flag cannot be an empty string. Please provide "
            "the values 'true' or 'false'"}
        fake_json = mock_read_to_hvac.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_hvac?is_manual_flow=')
        self.assertEqual(
            response.status_code,
            400,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The engine_rpm is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_hvac3(self, mock_read_to_hvac):
        mock_read_to_hvac.return_value = {
            'error': "The 'is_manual_flow' flag must be either 'true' or 'false'. You "
            "provided: 'ds'"}
        fake_json = mock_read_to_hvac.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_hvac?is_manual_flow=ds')
        self.assertEqual(
            response.status_code,
            400,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The engine_rpm is incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_hvac4(self, mock_read_info_hvac):
        mock_read_info_hvac.return_value = {'ambient_air_temperature': 50,
                                            'cabin_temperature': 20,
                                            'cabin_temperature_driver_set': 21,
                                            'fan_speed': 10,
                                            'hvac_modes': {'AC Status': True,
                                                           'Air Recirculation': False,
                                                           'Defrost': False,
                                                           'Front': True,
                                                           'Legs': False},
                                            'mass_air_flow': 5,
                                            'time_stamp': '2024-11-05T16:05:36.240350'}
        fake_json = mock_read_info_hvac.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get('/read_info_hvac?is_manual_flow=true')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json['ambient_air_temperature'],
            fake_json['ambient_air_temperature'],
            "The data are incorrect")
        self.assertEqual(
            response.json['cabin_temperature'],
            fake_json['cabin_temperature'],
            "The data are incorrect")
        self.assertEqual(
            response.json['cabin_temperature_driver_set'],
            fake_json['cabin_temperature_driver_set'],
            "The data are incorrect")
        self.assertEqual(
            response.json['fan_speed'],
            fake_json['fan_speed'],
            "The data are incorrect")
        self.assertEqual(
            response.json['mass_air_flow'],
            fake_json['mass_air_flow'],
            "The data are incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_hvac5(self, mock_read_info_hvac):
        mock_read_info_hvac.return_value = {
            'error': "Invalid parameter 'cabin_temperatu'. Use /get_identifiers to see valid "
            'parameters.'}
        fake_json = mock_read_info_hvac.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get(
            '/read_info_hvac?is_manual_flow=true&item=cabin_temperatu')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The data are incorrect")

    @patch('actions.read_info_action.ReadInfo')
    def test_read_info_hvac6(self, mock_read_info_hvac):
        mock_read_info_hvac.return_value = {
            "cabin_temperature": 19,
            "time_stamp": "2024-11-12T10:38:52.189470"}
        fake_json = mock_read_info_hvac.return_value
        response = self.client.post('/change_session', json={"sub_funct": 2})
        response = self.client.get(
            '/read_info_hvac?is_manual_flow=true&item=cabin_temperature')
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(
            response.json['cabin_temperature'],
            fake_json['cabin_temperature'],
            "The data are incorrect")

    @patch('actions.access_timing_action.ReadAccessTiming')
    def test_read_access_timing(self, mock_read_access_timing):
        mock_read_access_timing.return_value = {
            'message': 'Timing parameters accessed successfully',
            'timing_values': {
                'P2_MAX_TIME_DEFAULT': '100 seconds',
                'P2_STAR_MAX_TIME_DEFAULT': '200 milliseconds'}}
        fake_json = mock_read_access_timing.return_value
        response = self.client.post('/read_access_timing', json={"sub_funct": 1})
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The door status is incorrect")

    @patch('actions.access_timing_action.ReadAccessTiming')
    def test_read_access_timing2(self, mock_read_access_timing):
        mock_read_access_timing.return_value = {
            'message': 'Timing parameters accessed successfully',
            'timing_values': {
                'p2_max_time': '100 seconds',
                'p2_star_max_time': '200 milliseconds'}}
        fake_json = mock_read_access_timing.return_value
        response = self.client.post('/read_access_timing', json={"sub_funct": 3})
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The door status is incorrect")

    @patch('actions.access_timing_action.ReadAccessTiming')
    def test_read_access_timing3(self, mock_read_access_timing):
        mock_read_access_timing.return_value = {
            'message': 'Unexpected or insufficient response length while reading timing '
            'parameters', 'status': 'error'}
        fake_json = mock_read_access_timing.return_value
        response = self.client.post('/read_access_timing', json={"sub_funct": 2})
        self.assertEqual(
            response.status_code,
            200,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The door status is incorrect")

    @patch('actions.access_timing_action.ReadAccessTiming')
    def test_read_access_timing4(self, mock_read_access_timing):
        mock_read_access_timing.return_value = {
            'message': "Missing 'sub_funct' parameter", 'status': 'error'}
        fake_json = mock_read_access_timing.return_value
        response = self.client.post('/read_access_timing', json={"sub_funct": None})
        self.assertEqual(
            response.status_code,
            400,
            "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The door status is incorrect")


if __name__ == '__main__':
    runner = HtmlTestRunner.HTMLTestRunner(output="test_report")
    unittest.main(testRunner=runner)
