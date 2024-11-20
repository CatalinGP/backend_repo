import os
import sys
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__),'..',))
sys.path.append(PROJECT_ROOT)
import unittest
from unittest.mock import *
from unittest import TestCase, main
from flask import Flask
from flask import request, jsonify, Blueprint 
from routes.api import api_bp
from datetime import datetime
import HtmlTestRunner
import time
import json

class BatteryInfoTestSuite(TestCase):

    def setUp(self):
        self.app = Flask(__name__)
        self.app.register_blueprint(api_bp)
        self.client = self.app.test_client()
        self.app.testing=True

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_battery(self,mock_write_to_battery):
        payload={
            "is_manual_flow": "false",
            "battery_level": 75,
            "voltage": 10,
            "battery_state_of_charge": 2,
            "percentage": 2
        }
        mock_write_to_battery.return_value={
            "message": "Successfully written values to Battery ECU.",
            "written_values": {
                "battery_level": 75,
                "voltage": 10,
                "percentage": 2
            },
            "time_stamp": "2024-10-31T12:34:56.789123"
        }
        fake_json= mock_write_to_battery.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 2 })
        response = self.client.post('/write_info_battery', json=payload)
        
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

    #When battery level is out of range 1-100
    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_battery2(self,mock_write_to_battery):
        payload={
            "is_manual_flow": "false",
            "battery_level": "Psasa",
            "voltage": "Psasa",
            "battery_state_of_charge": "Psasa",
            "percentage": "Psasa"
        }
        mock_write_to_battery.return_value={
            "message": "Successfully written values to Battery ECU.",
            "written_values": {
            }
        }
        fake_json= mock_write_to_battery.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 2 })
        response = self.client.post('/write_info_battery', json=payload)
        
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

    
    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_battery3(self,mock_write_to_battery):
        payload={
        }
        mock_write_to_battery.return_value={'error': "The 'is_manual_flow' flag is required but was not provided in the "
           'request.'}
        fake_json= mock_write_to_battery.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 2 })
        response = self.client.post('/write_info_battery', json=payload)
        
        self.assertEqual(response.status_code,400, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json, "The json is not the expected one")
       
    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_battery4(self,mock_write_to_battery):
        payload={
            "is_manual_flow": "",
            "battery_level": 75,
            "voltage": 10,
            "battery_state_of_charge": 2,
            "percentage": 2
        }
        mock_write_to_battery.return_value={'error': "The 'is_manual_flow' flag cannot be an empty string. Please provide "
           "the values 'true' or 'false'"}
        fake_json= mock_write_to_battery.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 2 })
        response = self.client.post('/write_info_battery', json=payload)
        
        self.assertEqual(response.status_code,400, "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The message is incorrect")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_battery5(self,mock_write_to_battery):
        payload={
            "is_manual_flow": "true",
            "battery_level": 75,
            "voltage": 10,
            "battery_state_of_charge": 2,
            "percentage": 2
        }


        mock_write_to_battery.return_value={
            'message': 'Issue encountered during Write by ID',
            'negative_response': {'error_message': 'Security Access Denied',
            'nrc': '0x33',
            'service_description': 'Write Data by Identifier',
            'service_id': '0x2e'}}
        fake_json= mock_write_to_battery.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 1 })
        response = self.client.post('/write_info_battery', json=payload)
        
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The message is incorrect")
        
class EngineInfoTestSuite(TestCase):

    def setUp(self):
        self.app = Flask(__name__)
        self.app.register_blueprint(api_bp)
        self.client = self.app.test_client()
        self.app.testing=True

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_engine(self,mock_write_to_engine):
        payload={
        "is_manual_flow": "false",
        "engine_rpm": 4000,
        "coolant_temperature": 95,
        "throttle_position": 65,
        "vehicle_speed": 120,
        "engine_load": 80,
        "fuel_level": 85,
        "oil_temperature": 70,
        "fuel_pressure": 50,
        "intake_air_temperature": 25,
        "transmission_status": 3
    }
        self.maxDiff=None
        mock_write_to_engine.return_value={
        "message": "Successfully written values to Engine ECU.",
        "written_values": {
            "coolant_temperature": 95,
            "throttle_position": 65,
            "vehicle_speed": 120,
            "engine_load": 80,
            "fuel_level": 85,
            "oil_temperature": 70,
            "fuel_pressure": 50,
            "intake_air_temperature": 25
        },
        "time_stamp": "2024-10-31T12:34:56.789123"
    }

        fake_json= mock_write_to_engine.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 2 })
        response = self.client.post('/write_info_engine', json=payload)
        
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_engine2(self,mock_write_to_engine):
        payload={
        "is_manual_flow": "false",
        "engine_rpm": "Psasa",
        "coolant_temperature": "Psasa",
        "throttle_position": "Psasa",
        "vehicle_speed": "Psasa",
        "engine_load": "Psasa",
        "fuel_level": "Psasa",
        "oil_temperature": "Psasa",
        "fuel_pressure": "Psasa",
        "intake_air_temperature": "Psasa",
        "transmission_status": "Psasa"
    }
        self.maxDiff=None
        mock_write_to_engine.return_value={
        "message": "Successfully written values to Engine ECU.",
        "written_values": {

        }
    }

        fake_json= mock_write_to_engine.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 2 })
        response = self.client.post('/write_info_engine', json=payload)
        
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_engine3(self,mock_write_to_engine):
        payload={
    }
        mock_write_to_engine.return_value={'error': "The 'is_manual_flow' flag is required but was not provided in the "
           'request.'}

        fake_json= mock_write_to_engine.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 2 })
        response = self.client.post('/write_info_engine', json=payload)
        
        self.assertEqual(response.status_code,400, "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The message is incorrect")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_engine4(self,mock_write_to_engine):
        payload={
        "is_manual_flow": "",
        "engine_rpm": 4000,
        "coolant_temperature": 95,
        "throttle_position": 65,
        "vehicle_speed": 120,
        "engine_load": 80,
        "fuel_level": 85,
        "oil_temperature": 70,
        "fuel_pressure": 50,
        "intake_air_temperature": 25,
        "transmission_status": 3
    }
        self.maxDiff=None
        mock_write_to_engine.return_value={'error': "The 'is_manual_flow' flag cannot be an empty string. Please provide "
           "the values 'true' or 'false'"}
        fake_json= mock_write_to_engine.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 2 })
        response = self.client.post('/write_info_engine', json=payload)
        
        self.assertEqual(response.status_code,400, "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The message is incorrect")
    
    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_engine5(self,mock_write_to_engine):
        payload={
        "is_manual_flow": "true",
        "engine_rpm": 4000,
        "coolant_temperature": 95,
        "throttle_position": 65,
        "vehicle_speed": 120,
        "engine_load": 80,
        "fuel_level": 85,
        "oil_temperature": 70,
        "fuel_pressure": 50,
        "intake_air_temperature": 25,
        "transmission_status": 3
    }
        self.maxDiff=None
        mock_write_to_engine.return_value={
            'message': 'Issue encountered during Write by ID',
            'negative_response': {'error_message': 'Security Access Denied',
            'nrc': '0x33',
            'service_description': 'Write Data by Identifier',
            'service_id': '0x2e'}}
    

        fake_json= mock_write_to_engine.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 1 })
        response = self.client.post('/write_info_engine', json=payload)
        
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The message is incorrect")

class DoorsInfoTestSuite(TestCase):

    def setUp(self):
        self.app = Flask(__name__)
        self.app.register_blueprint(api_bp)
        self.client = self.app.test_client()
        self.app.testing=True

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_doors(self,mock_write_to_doors):
        payload={
            "is_manual_flow": "false",
            "door": 0,
            "passenger": 0,
            "driver": 1,
            "passenger_lock": 0,
            "ajar": 0
        }
        
        mock_write_to_doors.return_value={
        "message": "Successfully written values to Doors ECU.",
        "written_values": {
            "door": 0,
            "passenger": 0,
            "driver": 1,
            "passenger_lock": 0,
            "ajar": 0
        },
        "time_stamp": "2024-10-31T12:34:56.789123"
    }
        fake_json= mock_write_to_doors.return_value
        response = self.client.post('/write_info_doors', json=payload)
        
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_doors2(self,mock_write_to_doors):
        payload={
            "is_manual_flow": "false",
            "door": "dsa",
            "passenger": "dsa",
            "driver": "dsa",
            "passenger_lock": "dsa",
            "ajar": "dsa"
        }
        
        mock_write_to_doors.return_value={
        "message": "Successfully written values to Doors ECU.",
        "written_values": {
        }
    }
        fake_json= mock_write_to_doors.return_value
        response = self.client.post('/write_info_doors', json=payload)
        
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_doors3(self,mock_write_to_doors):
        payload={
        }
        
        mock_write_to_doors.return_value={'error': "The 'is_manual_flow' flag is required but was not provided in the "
           'request.'}
        fake_json= mock_write_to_doors.return_value
        response = self.client.post('/write_info_doors', json=payload)
        
        self.assertEqual(response.status_code,400, "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The message is incorrect")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_doors4(self,mock_write_to_doors):
        payload={
            "is_manual_flow": "",
            "door": 0,
            "passenger": 0,
            "driver": 1,
            "passenger_lock": 0,
            "ajar": 0
        }
        
        mock_write_to_doors.return_value={'error': "The 'is_manual_flow' flag cannot be an empty string. Please provide "
           "the values 'true' or 'false'"}
        fake_json= mock_write_to_doors.return_value
        response = self.client.post('/write_info_doors', json=payload)
        
        self.assertEqual(response.status_code,400, "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The message is incorrect")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_doors5(self,mock_write_to_doors):
        payload={
            "is_manual_flow": "true",
            "door": 0,
            "passenger": 0,
            "driver": 1,
            "passenger_lock": 0,
            "ajar": 0
        }
        
        mock_write_to_doors.return_value={
            'message': 'Issue encountered during Write by ID',
            'negative_response': {'error_message': 'Security Access Denied',
            'nrc': '0x33',
            'service_description': 'Write Data by Identifier',
            'service_id': '0x2e'}}
        fake_json= mock_write_to_doors.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 1 })
        response = self.client.post('/write_info_doors', json=payload)
        
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

class HVACInfoTestSuite(TestCase):

    def setUp(self):
        self.app = Flask(__name__)
        self.app.register_blueprint(api_bp)
        self.client = self.app.test_client()
        self.app.testing=True

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_hvac(self,mock_write_to_hvac):
        payload={
        "is_manual_flow": "false",
        "mass_air_flow": 5,
        "ambient_air_temperature": 5,
        "cabin_temperature": 20,
        "cabin_temperature_driver_set": 21,
        "fan_speed": 10,
        "hvac_modes": 5
    }
        
        mock_write_to_hvac.return_value={
        "message": "Successfully written values to HVAC ECU.",
        "written_values": {
            "mass_air_flow": 5,
            "ambient_air_temperature": 5,
            "cabin_temperature": 20,
            "cabin_temperature_driver_set": 21,
            "fan_speed": 10,
            "hvac_modes": 5
        },
        "time_stamp": "2024-10-31T12:34:56.789123"
    }
        fake_json= mock_write_to_hvac.return_value
        response = self.client.post('/write_info_hvac', json=payload)
   
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_hvac2(self,mock_write_to_hvac):
        payload={
        "is_manual_flow": "false",
        "mass_air_flow": "sdadas",
        "ambient_air_temperature": "sdadas",
        "cabin_temperature": "sdadas",
        "cabin_temperature_driver_set": "sdadas",
        "fan_speed": "sdadas",
        "hvac_modes": "sdadas"
    }
        
        mock_write_to_hvac.return_value={
        "message": "Successfully written values to HVAC ECU.",
        "written_values": {

        }
        }
        fake_json= mock_write_to_hvac.return_value
        response = self.client.post('/write_info_hvac', json=payload)
   
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_hvac3(self,mock_write_to_hvac):
        payload={
        }
        
        mock_write_to_hvac.return_value={'error': "The 'is_manual_flow' flag is required but was not provided in the "
           'request.'}
        fake_json= mock_write_to_hvac.return_value
        response = self.client.post('/write_info_hvac', json=payload)
   
        self.assertEqual(response.status_code,400, "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The message is incorrect")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_hvac4(self,mock_write_to_hvac):
        payload={
        "is_manual_flow": "",
        "mass_air_flow": 5,
        "ambient_air_temperature": 5,
        "cabin_temperature": 20,
        "cabin_temperature_driver_set": 21,
        "fan_speed": 10,
        "hvac_modes": 5
    }
        
        mock_write_to_hvac.return_value={'error': "The 'is_manual_flow' flag cannot be an empty string. Please provide "
           "the values 'true' or 'false'"}
        fake_json= mock_write_to_hvac.return_value
        response = self.client.post('/write_info_hvac', json=payload)
   
        self.assertEqual(response.status_code,400, "The HTTP response is not the expected one")
        self.assertEqual(response.json, fake_json, "The message is incorrect")

    @patch('actions.write_info_action.WriteInfo')
    def test_write_info_hvac5(self,mock_write_to_hvac):
        payload={
        "is_manual_flow": "true",
        "mass_air_flow": 5,
        "ambient_air_temperature": 5,
        "cabin_temperature": 20,
        "cabin_temperature_driver_set": 21,
        "fan_speed": 10,
        "hvac_modes": 5
    }
        
        mock_write_to_hvac.return_value={
            'message': 'Issue encountered during Write by ID',
            'negative_response': {'error_message': 'Security Access Denied',
            'nrc': '0x33',
            'service_description': 'Write Data by Identifier',
            'service_id': '0x2e'}}
        fake_json= mock_write_to_hvac.return_value
        response = self.client.post('/change_session', json={ "sub_funct": 1 })
        response = self.client.post('/write_info_hvac', json=payload)
   
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json["message"], fake_json["message"], "The message is incorrect")
        self.assertEqual(response.json["written_values"], fake_json["written_values"],"The written values aren't correct")

class TimingInfoTestSuite(TestCase):

    def setUp(self):
        self.app = Flask(__name__)
        self.app.register_blueprint(api_bp)
        self.client = self.app.test_client()
        self.app.testing=True

    @patch('actions.access_timing_action.WriteAccessTiming')
    def test_write_access_timing(self,mock_write_access_timing):

        payload={
        "p2_max": 50,
        "p2_star_max": 100
        }
        mock_write_access_timing.return_value={
        "message": "Timing parameters written successfully",
        "written_values": {
            "New P2 Max Time": 50,
            "New P2 Star Max": 100
            }
        }
        fake_json=mock_write_access_timing.return_value
        response = self.client.post('/write_timing', json=payload)
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json, "The data are not correct")

    @patch('actions.access_timing_action.WriteAccessTiming')
    def test_write_access_timing2(self,mock_write_access_timing):

        payload={
        "p2_max": "50",
        "p2_star_max": "100"
        }
        mock_write_access_timing.return_value={'message': "unsupported operand type(s) for >>: 'str' and 'int'"}
        fake_json=mock_write_access_timing.return_value
        response = self.client.post('/write_timing', json=payload)
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json, "The data are not correct")

    @patch('actions.access_timing_action.WriteAccessTiming')
    def test_write_access_timing3(self,mock_write_access_timing):

        payload={

        }
        mock_write_access_timing.return_value={'message': 'Missing required parameters'}
        fake_json=mock_write_access_timing.return_value
        response = self.client.post('/write_timing', json=payload)
        self.assertEqual(response.status_code,400, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json, "The data are not correct")

    @patch('actions.access_timing_action.WriteAccessTiming')
    def test_write_access_timing4(self,mock_write_access_timing):

        payload={
        "p2_max": 1.20,
        "p2_star_max": 3.20
        }
        mock_write_access_timing.return_value={'message': "unsupported operand type(s) for >>: 'float' and 'int'"}
        fake_json=mock_write_access_timing.return_value
        response = self.client.post('/write_timing', json=payload)
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json, "The data are not correct")

if __name__ == '__main__':
    unittest.main()