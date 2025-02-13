from flask import jsonify
from flask_jwt_extended import get_jwt_identity, verify_jwt_in_request
from models import TokenList
from functools import wraps


def role_required(required_role):
    """
    """
    def wrapper(fn):
        @wraps(fn)
        def decorator(*args, **kwargs):
            verify_jwt_in_request()
            claims = get_jwt_identity()

            user_role = claims.get('role')
            token_identifier = claims.get('token_identifier')

            token_entry = TokenList.query.filter_by(
                token_identifier=token_identifier).first()
            if not token_entry:
                return jsonify({'message': 'Token has been revoked'}), 403

            if user_role != required_role and user_role != 'admin':
                return jsonify(
                    {'message': 'Access forbidden: insufficient privileges'}), 403

            return fn(*args, **kwargs)
        return decorator
    return wrapper
