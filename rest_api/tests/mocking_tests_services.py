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

class TestAPISendFrame(TestCase):

    def setUp(self):
        self.app = Flask(__name__)
        self.app.register_blueprint(api_bp)
        self.client = self.app.test_client()
        self.app.testing=True

    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_success_clear_diagnostic(self,mock_manual_send_frame):
        # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA11",
            "can_data": "0x02, 0x14, 0x00, 0x0A, 0xAA"
        }
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value ={"response":[{"can_data":["0x1","0x54"],"can_id":"0X11FA"}]}

        fake_json=mock_manual_send_frame.return_value
        
        # Act: Send a POST request to the /send_frame API
        response = self.client.post('/send_frame', json=payload)

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_success_security_access(self,mock_manual_send_frame):
        # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA10",
            "can_data": "0x02, 0x27, 0x03"
        }
        payload2={ "sub_funct": 2 } 
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value = {
        "response": [
            {
                "can_data": [
                    "0x2",
                    "0x10",
                    "0x2"
                ],
                "can_id": "0XFA10"
            },
            {
                "can_data": [
                    "0x2",
                    "0x50",
                    "0x2"
                ],
                "can_id": "0X10FA"
            },
            {
                "auth_status": "failed",
                "can_data": [
                    "0x3",
                    "0x7f",
                    "0x27",
                    "0x12"
                ],
                "can_id": "0X10FA",
                "error_text": {
                    "error_message": "SubFunction Not Supported",
                    "nrc": "0x12",
                    "service_description": "Security Access",
                    "service_id": "0x27"
                }
            }
        ]
    }
        fake_json=mock_manual_send_frame.return_value
        
        # Act: Send a POST request to the /send_frame API
        response = self.client.post('/change_session', json=payload2)
        response = self.client.post('/send_frame', json=payload)

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_diagnostic_session(self,mock_manual_send_frame):
        # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA10",
            "can_data": "0x02, 0x10, 0x01"
        }
        payload2={ "sub_funct": 1 } 
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value = {
        "response": [
            {
                "can_data": [
                    "0x2",
                    "0x10",
                    "0x1"
                ],
                "can_id": "0XFA10"
            },
            {
                "can_data": [
                    "0x2",
                    "0x50",
                    "0x1"
                ],
                "can_id": "0X10FA"
            },
            {
                "can_data": [
                    "0x2",
                    "0x50",
                    "0x1"
                ],
                "can_id": "0X10FA"
            }
        ]
    }
        fake_json=mock_manual_send_frame.return_value
        
        
        # Act: Send a POST request to the /send_frame API
        response = self.client.post('/change_session', json=payload2)
        time.sleep(5)
        response = self.client.post('/send_frame', json=payload)

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")
    
    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_tester_present(self,mock_manual_send_frame):
        # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA10",
            "can_data": "0x02, 0x3E, 0x00"
        }
        payload2={ "sub_funct": 1 } 
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value = {
        "response": [
            {
                "can_data": [
                    "0x2",
                    "0x10",
                    "0x1"
                ],
                "can_id": "0XFA10"
            },
            {
                "can_data": [
                    "0x2",
                    "0x50",
                    "0x1"
                ],
                "can_id": "0X10FA"
            },
            {
                "can_data": [
                    "0x2",
                    "0x7e",
                    "0x0"
                ],
                "can_id": "0X10FA"
            }
        ]
    }
        fake_json=mock_manual_send_frame.return_value
        
        # Act: Send a POST request to the /send_frame API
        response = self.client.post('/change_session', json=payload2)
        response = self.client.post('/send_frame', json=payload)

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    
    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_routine_control(self,mock_manual_send_frame):
        # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA10",
            "can_data": "0x03, 0x31, 0x01"
        }
        payload2={ "sub_funct": 1 } 
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value = {
        "response": [
            {
                "can_data": [
                    "0x2",
                    "0x10",
                    "0x1"
                ],
                "can_id": "0XFA10"
            },
            {
                "can_data": [
                    "0x2",
                    "0x50",
                    "0x1"
                ],
                "can_id": "0X10FA"
            },
            {
                "auth_status": "failed",
                "can_data": [
                    "0x3",
                    "0x7f",
                    "0x31",
                    "0x13"
                ],
                "can_id": "0X10FA",
                "error_text": {
                    "error_message": "Incorrect Message Length Or Invalid Format",
                    "nrc": "0x13",
                    "service_description": "Routine Control",
                    "service_id": "0x31"
                }
            }
        ]
    }
        fake_json=mock_manual_send_frame.return_value
        
        # Act: Send a POST request to the /send_frame API
        response = self.client.post('/change_session', json=payload2)
        response = self.client.post('/send_frame', json=payload)

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")


    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_success_ecu_reset(self,mock_manual_send_frame):
        # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA10",
            "can_data": "0x02, 0x11, 0x01"
        }
        payload2={ "sub_funct": 1 } 
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value = {
        "response": [
            {
                "can_data": [
                    "0x2",
                    "0x10",
                    "0x1"
                ],
                "can_id": "0XFA10"
            },
            {
                "can_data": [
                    "0x2",
                    "0x50",
                    "0x1"
                ],
                "can_id": "0X10FA"
            },
            {
                "auth_status": "failed",
                "can_data": [
                    "0x3",
                    "0x7f",
                    "0x11",
                    "0x33"
                ],
                "can_id": "0X10FA",
                "error_text": {
                    "error_message": "Security Access Denied",
                    "nrc": "0x33",
                    "service_description": "Ecu Reset",
                    "service_id": "0x11"
                }
            }
        ]
    }
        fake_json=mock_manual_send_frame.return_value
        
        # Act: Send a POST request to the /send_frame API
        response = self.client.post('/change_session', json=payload2)
        response = self.client.post('/send_frame', json=payload)

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_write_data_by_id(self,mock_manual_send_frame):
             # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA10",
            "can_data": "0x07, 0x2E, 0xF1, 0x90, 0x55,0x44,0x33,0x22 "
        }
        payload2={ "sub_funct": 2 } 
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value = {
        "response": [
            {
            "can_data": [
                "0x2",
                "0x10",
                "0x2"
            ],
            "can_id": "0XFA10"
            },
            {
            "can_data": [
                "0x2",
                "0x50",
                "0x2"
            ],
            "can_id": "0X10FA"
            },
            {
            "auth_status": "failed",
            "can_data": [
                "0x3",
                "0x7f",
                "0x2e",
                "0x33"
            ],
            "can_id": "0X10FA",
            "error_text": {
                "error_message": "Security Access Denied",
                "nrc": "0x33",
                "service_description": "Write Data by Identifier",
                "service_id": "0x2e"
            }
            }
        ]
        }
        fake_json=mock_manual_send_frame.return_value
        
        # Act: Send a POST request to the /send_frame API
        response = self.client.post('/change_session', json=payload2)
        response = self.client.post('/send_frame', json=payload)
     
        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_read_by_id(self,mock_manual_send_frame):
             # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA10",
            "can_data": "0x02, 0x23, 0x000010, 0x0010"
        }
        payload2={ "sub_funct": 2 } 
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value = {
    "response": [
        {
        "can_data": [
            "0x2",
            "0x10",
            "0x2"
        ],
        "can_id": "0XFA10"
        },
        {
        "can_data": [
            "0x2",
            "0x50",
            "0x2"
        ],
        "can_id": "0X10FA"
        },
        {
        "auth_status": "failed",
        "can_data": [
            "0x3",
            "0x7f",
            "0x23",
            "0x13"
        ],
        "can_id": "0XFA10",
        "error_text": {
            "error_message": "Incorrect Message Length Or Invalid Format",
            "nrc": "0x13",
            "service_description": "Unknown service",
            "service_id": "0x23"
        }
        },
        {
        "auth_status": "failed",
        "can_data": [
            "0x3",
            "0x7f",
            "0x23",
            "0x78"
        ],
        "can_id": "0X2300",
        "error_text": {
            "error_message": "Request Correctly Received-Response Pending",
            "nrc": "0x78",
            "service_description": "Unknown service",
            "service_id": "0x23"
        }
        }
    ]
    }
        fake_json=mock_manual_send_frame.return_value
        
        # Act: Send a POST request to the /send_frame API
        response = self.client.post('/change_session', json=payload2)
        response = self.client.post('/send_frame', json=payload)
        
        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_access_timing_parameter(self,mock_manual_send_frame):
        # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA10",
            "can_data": "0x02, 0x83, 0x01"
        }
        payload2={ "sub_funct": 1 } 
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value = {
        "response": [
            {
                "can_data": [
                    "0x2",
                    "0x10",
                    "0x1"
                ],
                "can_id": "0XFA10"
            },
            {
                "can_data": [
                    "0x2",
                    "0x50",
                    "0x1"
                ],
                "can_id": "0X10FA"
            },
            {
                "can_data": [
                    "0x6",
                    "0xc3",
                    "0x1",
                    "0x0",
                    "0x28",
                    "0x1",
                    "0x90"
                ],
                "can_id": "0X10FA"
            }
        ]
    }
        fake_json=mock_manual_send_frame.return_value
        
        # Act: Send a POST request to the /send_frame API
        response = self.client.post('/change_session', json=payload2)
        response = self.client.post('/send_frame', json=payload)

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    @patch('actions.manual_send_frame.manual_send_frame')
    def test_send_frame_read_dtc_information(self,mock_manual_send_frame):
             # Arrange: Define the payload for the POST request
        payload = {
            "can_id": "0xFA10",
            "can_data": "0x03, 0x19, 0x01, 0xFF"
        }
        payload2={ "sub_funct": 1 } 
        self.maxDiff = None  # Allow full diff output for assert failures
        
        mock_manual_send_frame.return_value = {
        "response": [
            {
                "can_data": [
                    "0x2",
                    "0x10",
                    "0x1"
                ],
                "can_id": "0XFA10"
            },
            {
                "can_data": [
                    "0x2",
                    "0x50",
                    "0x1"
                ],
                "can_id": "0X10FA"
            },
            {
                "auth_status": "failed",
                "can_data": [
                    "0x3",
                    "0x7f",
                    "0x19",
                    "0x94"
                ],
                "can_id": "0X10FA",
                "error_text": {
                    "error_message": "Unable to read DTCs",
                    "nrc": "0x94",
                    "service_description": "Read DTC Information",
                    "service_id": "0x19"
                }
            }
        ]
    }

        fake_json=mock_manual_send_frame.return_value
        response = self.client.post('/change_session', json=payload2)
        response = self.client.post('/send_frame', json=payload)

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")
        
class TestAPIs(unittest.TestCase):

    def setUp(self):
        self.app = Flask(__name__)
        self.app.register_blueprint(api_bp)
        self.client = self.app.test_client()
        self.app.testing=True
    
    #Test for request_ids
    @patch('actions.request_id_action.RequestIdAction')
    def test_request_ids_success(self,mock_get_request_id):
        mock_get_request_id.return_value = {'ecus': [{'ecu_id': '11', 'version': '0.0'}], 
                'mcu_id': '10',
                'status': 'Success',
                "time_stamp": datetime.now().isoformat()
            }
        fake_json=mock_get_request_id.return_value
        payload={ "sub_funct": 2 }

        # Act: Simulate a GET request to the /request_ids API endpoint
        response = self.client.post('/change_session',json=payload) 
        response = self.client.get('/request_ids')
        
        # Assert: Verify the response status and data
        self.assertEqual(response.status_code, 200, "The HTTP response is not the expected one")
        self.assertEqual(response.json['ecus'][0]['ecu_id'], fake_json['ecus'][0]['ecu_id'], "ECU ID is incorrect")
        self.assertEqual(response.json['ecus'][0]['version'], fake_json['ecus'][0]['version'], "ECU version is incorrect")
        self.assertEqual(response.json['mcu_id'], fake_json['mcu_id'], "MCU ID is incorrect")
        self.assertEqual(response.json['status'], fake_json['status'], "Status is incorrect")

        #Assert using not Equal because it is not possible to assert time_stamp with the same values in isoformat"
        self.assertNotEqual(response.json['time_stamp'], fake_json['time_stamp'], "The obtained data are not correct")

    @patch('actions.diag_session.SessionManager')
    def test_change_session(self,mock_diag_session):
        payload={ "sub_funct": 1 }      
        mock_diag_session.return_value={
            "message": "Session changed to DEFAULT successfully"
            }
        fake_json= mock_diag_session.return_value

        # Act: Send a POST request to the /change_session API
        response = self.client.post('/change_session', json=payload)
        self.assertEqual(response.status_code,200 , "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    @patch('actions.diag_session.SessionManager')
    def test_change_session2(self,mock_diag_session):
        payload={ "sub_funct": 2 }      
       
        mock_diag_session.return_value={
            "message": "Session changed to PROGRAMMING successfully"
            }
        fake_json= mock_diag_session.return_value

        # Act: Send a POST request to the /change_session API
        response = self.client.post('/change_session', json=payload)
        self.assertEqual(response.status_code, 200 , "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")        

    # Test for successful authentication
    @patch('actions.secure_auth.Auth')
    def test_authenticate_success(self,mock_auth_class):
        payload={ "sub_funct": 2 } 

        # Arrange: Mock the Auth class and its _auth_to method
        mock_auth_class.return_value= {"message": "Authentication successful"}
        fake_json=mock_auth_class.return_value

        # Act: Send a POST request to the /change_session API
        # Act: Simulate a GET request to /authenticate
        response = self.client.post('/change_session', json=payload)
        response = self.client.get('/authenticate')

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code, 200 , "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    # Test for handling CustomError
    @patch('actions.secure_auth.Auth')
    def test_authenticate_error(self,mock_auth_class):
        payload={ "sub_funct": 1 } 

        # Arrange: Set up the mock to raise a CustomError
        mock_auth_class.return_value = {
            "message": "Error during authentication",
            "negative_response": {
            "error_message": "SubFunction Not Supported In Active Session",
            "nrc": "0x7e",
            "service_description": "Security Access",
            "service_id": "0x27"
            }}

        fake_json=mock_auth_class.return_value

        # Act: Simulate a GET request to /authenticate
        response = self.client.post('/change_session', json=payload)
        response = self.client.get('/authenticate')
              
        # Assert: Verify the response status and error message
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    #Test for clear DTC info
    @patch('actions.dtc_info.DiagnosticTroubleCode')
    def test_clear_dtc_info(self,mock_clear_dtc_info):
        mock_clear_dtc_info.return_value={
        "message": "Clearing all DTCs information with positive response succeded"
        }
        fake_json=mock_clear_dtc_info.return_value
        response = self.client.get('/clear_dtc_info')
        # Assert: Verify the response status and data
        self.assertEqual(response.status_code, 200,"The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")
            
    #Test for tester present
    @patch('actions.tester_present.Action')
    def get_tester_present(self,mock_is_present):
      
        # Arrange: Set up the mock response for is_present()
        mock_is_present.return_value = {
                "can_id": "0xFA10",
                "message": "Tester is present"
            }
        fake_json=mock_is_present.return_value

        response = self.client.get('/tester_present')
        self.assertEqual(response.status_code, 200,"The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    #Test for reset_ecu when is failed
    @patch('actions.ecu_reset.Reset')
    def test_reset_ecu_error(self,mock_reset_ecu):
        # Create the payload for the POST request
        payload = {
            "ecu_id": "8",
            "type_reset": "soft" #or soft
        }
    
        # Arrange: Set up the mock response for reset_ecu
        mock_reset_ecu.return_value = {
            "message": "Invalid ECU ID: " + payload["ecu_id"],
            "status": "error"
        }
        fake_json=mock_reset_ecu.return_value

        # Act: Simulate a POST request to the /reset_ecu API endpoint
        response = self.client.post('/change_session', json=payload)
        response = self.client.post('/reset_ecu', json=payload)

        # Assert: Verify the response status and data
        self.assertEqual(response.status_code, 200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    @patch('actions.ecu_reset.Reset')
    def test_reset_ecu(self,mock_reset_ecu):
        # Create the payload for the POST request
        payload = {
            "ecu_id": "11",
            "type_reset": "soft" #or hard
        }
    
        # Arrange: Set up the mock response for reset_ecu
        mock_reset_ecu.return_value = {
        "can_id": "0xFA11",
        "message": "ECU reset successful"
        }
                
        fake_json=mock_reset_ecu.return_value

        # Act: Simulate a POST request to the /reset_ecu API endpoint
        response = self.client.post('/change_session', json={ "sub_funct": 2 })
        response = self.client.get('/authenticate')
        response = self.client.post('/reset_ecu', json=payload)
        # Assert: Verify the response status and data
        self.assertEqual(response.status_code, 200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")

    @patch('actions.diag_session.SessionManager')
    def test_change_session3(self,mock_diag_session):
        payload={ "sub_funct": 3 }      
       
        mock_diag_session.return_value={
            "message": "Session changed to unknown successfully"
            }
        fake_json= mock_diag_session.return_value

        # Act: Send a POST request to the /change_session API
        response = self.client.post('/change_session', json=payload)
        self.assertEqual(response.status_code,200, "The HTTP response is not the expected one")
        self.assertEqual(response.json,fake_json,"The data are not correct")     


if __name__ == '__main__':
    runner = HtmlTestRunner.HTMLTestRunner(output="test_report")
    unittest.main(testRunner=runner)