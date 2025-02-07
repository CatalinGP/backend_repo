import os
import sys
import shutil
from datetime import datetime



log_size_uncompressed = 0

def uploadFrontendLogs():
    PROJECT_SRC_PATHY = os.path.abspath(os.path.join(os.getcwd(), "..", ".."))
    google_drive_api_path = PROJECT_SRC_PATHY + '/backend/ota/google_drive_api'
    sys.path.append(google_drive_api_path)
    from GoogleDriveApi import gDrive, FRONTEND_LOG_FILE_LOCATION
    current_date = datetime.now().strftime("%d-%m-%Y_%H:%M")
    

    directory_path = os.path.expanduser(PROJECT_SRC_PATHY + '/backend/rest_api/utils/log' )
    for file in os.listdir(directory_path):

        if "logs" in file:
            new_filename = f"log-{current_date}.txt"
            new_file_path = os.path.join(directory_path, new_filename)
            old_file_path = os.path.join(directory_path, file)
            shutil.move(old_file_path, new_file_path)
            gDrive.uploadFile(new_filename, new_file_path, log_size_uncompressed, FRONTEND_LOG_FILE_LOCATION)