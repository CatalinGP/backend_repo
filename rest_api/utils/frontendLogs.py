import os
import sys
import subprocess
import shutil
PROJECT_SRC_PATHY = os.path.abspath(os.path.join(os.getcwd(), "..", ".."))
google_drive_api_path = PROJECT_SRC_PATHY + '/backend/ota/google_drive_api'
sys.path.append(google_drive_api_path)


from GoogleDriveApi import gDrive, FRONTEND_LOG_FILE_LOCATION, DRIVE_MCU_SW_VERSIONS_FILE


log_size_uncompressed = 0

def uploadFrontendLogs():
    
    directory_path = os.path.expanduser(PROJECT_SRC_PATHY + '/backend/rest_api/utils/log' )
    for file in os.listdir(directory_path):

        if "logs" in file:
            gDrive.uploadFile(file, os.path.join(
                directory_path, file), log_size_uncompressed, FRONTEND_LOG_FILE_LOCATION)
        
        
