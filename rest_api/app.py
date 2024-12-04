from flask import Flask
from flasgger import Swagger
from config import *
from routes.main import main_bp
from flask_cors import CORS
from routes.auth import auth_bp
from routes.api import api_bp
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address
from models import db
from flask_jwt_extended import JWTManager
from utils.utils import cleanup_expired_tokens


app = Flask(__name__)

limiter = Limiter(
    get_remote_address,
    app=app,
    default_limits=["100 per 1 seconds"]
)

app.config.from_object(Config)
db.init_app(app)
jwt = JWTManager(app)

swagger = Swagger(app, template_file=YAML_FILE_PATH)

app.register_blueprint(auth_bp, url_prefix='/auth')
app.register_blueprint(api_bp, url_prefix='/api')
app.register_blueprint(main_bp)
CORS(app)

with app.app_context():
    db.create_all()
    cleanup_expired_tokens()

if __name__ == '__main__':
    app.run(port=5000, debug=True)
