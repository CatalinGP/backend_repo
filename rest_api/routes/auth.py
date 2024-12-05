from flask import Blueprint, request, jsonify
from models import User, db, TokenList
from flask_jwt_extended import create_access_token, get_jwt_identity , jwt_required
from utils.decorators import role_required
import uuid

auth_bp = Blueprint('auth', __name__)

@auth_bp.route('/login', methods=['POST'])
def login():
    data = request.get_json()
    username = data.get('username')
    password = data.get('password')

    user = User.query.filter_by(username=username).first()
    if user and user.check_password(password):
        token_identifier = str(uuid.uuid4())
        access_token = create_access_token(
            identity={
            'id': user.id,
            'role': user.role,
            'token_identifier': token_identifier
        })
        new_token = TokenList(user_id=user.id, token_identifier=token_identifier)
        db.session.add(new_token)
        db.session.commit()

        return jsonify({'token': access_token, 'role': user.role}), 200

    return jsonify({'message': 'Invalid credentials'}), 401

@auth_bp.route('/register', methods=['POST'])
def register():
    data = request.get_json()
    username = data.get('username')
    password = data.get('password')
    role = data.get('role', 'member') 

    if not username or not password:
        return jsonify({'message': 'Username and password are required'}), 400

    existing_user = User.query.filter_by(username=username).first()
    if existing_user:
        return jsonify({'message': 'Username already exists'}), 409

    new_user = User(username=username, role=role)
    new_user.set_password(password)

    db.session.add(new_user)
    db.session.commit()

    return jsonify({'message': f'User {username} registered successfully'}), 201

@auth_bp.route('/logout', methods=['POST'])
@jwt_required()
def logout():
    identity = get_jwt_identity()
    token_identifier = identity.get('token_identifier')
    
    token_entry = TokenList.query.filter_by(token_identifier=token_identifier).first()
    
    if token_entry:
        db.session.delete(token_entry)
        db.session.commit()
        return jsonify({'message': 'Successfully logged out'}), 200

    return jsonify({'message': 'Token not found or already revoked'}), 400