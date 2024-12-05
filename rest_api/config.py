import os
from flask import request
from flask_limiter import Limiter
import json


# For Swagger Doc
YAML_FILE_PATH = os.path.abspath('utils/docs/api_doc.yaml')


limiter = Limiter(key_func=lambda: request.remote_addr)


class Config:
    with open('../../../key.json', 'r') as file:
        key_data = json.load(file)

    CAN_CHANNEL = 'vcan1'
    BUS_RECEIVE_TIMEOUT = 99
    SECRET_KEY = key_data.get('private_key_id')
    SQLALCHEMY_DATABASE_URI = 'sqlite:///db.sqlite'
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    JWT_SECRET_KEY =  key_data.get('private_key_id')
    JWT_ACCESS_TOKEN_EXPIRES = False
