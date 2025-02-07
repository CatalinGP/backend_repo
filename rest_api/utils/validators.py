import re
from flask import jsonify
from functools import wraps
from configs.data_identifiers import data_identifiers  # noqa: F401
import requests  # noqa: F401

# Define allowed contexts for status interpretation
ALLOWED_CONTEXTS = {"door", "lock", "ajar", "unknown"}


def validate_item_exists(module_identifiers):
    def decorator(f):
        @wraps(f)
        def wrapper(*args, **kwargs):
            item = kwargs.get('item') or request.args.get('item') or request.json.get('item')  # noqa: F821
            if item is None:
                return jsonify({"error": "Missing 'item' parameter"}), 400
            if item not in module_identifiers:
                return jsonify({"error": f"Unrecognized identifier '{item}'"}), 400
            return f(*args, **kwargs)
        return wrapper
    return decorator


def validate_hex_binary(value):
    hex_pattern = re.compile(r'^0x[0-9A-Fa-f]+$')
    binary_pattern = re.compile(r'^0b[01]+$')
    if not (hex_pattern.match(value) or binary_pattern.match(value)):
        raise ValueError(
            f"Value '{value}' is not in a valid hexadecimal or binary format.")


def validate_context(context):
    if context not in ALLOWED_CONTEXTS:
        raise ValueError(
            f"Unsupported context '{context}'. Allowed contexts: {ALLOWED_CONTEXTS}")


def validate_data_types(expected_types):
    def decorator(f):
        @wraps(f)
        def wrapper(*args, **kwargs):
            data = request.get_json()  # noqa: F821
            for param, param_type in expected_types.items():
                if param not in data:
                    return jsonify({"error": f"Missing parameter '{param}'"}), 400
                if not isinstance(data[param], param_type):
                    return jsonify(
                        {"error": f"Parameter '{param}' must be of type {param_type.__name__}."}), 400
            return f(*args, **kwargs)
        return wrapper
    return decorator


def validate_interpreted_values(mapping_dict):
    def decorator(f):
        @wraps(f)
        def wrapper(*args, **kwargs):
            value = kwargs.get('value') or request.args.get('value') or request.json.get('value')  # noqa: F821
            if value not in mapping_dict:
                return jsonify(
                    {"error": f"Unsupported value '{value}' for interpretation."}), 400
            return f(*args, **kwargs)
        return wrapper
    return decorator
