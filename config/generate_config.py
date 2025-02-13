import os
import configparser

config_file = "config.ini"

# Move two directories up
project_path = os.path.abspath(os.path.join(os.getcwd(), "..", ".."))

config = configparser.ConfigParser()
config.optionxform = str

config["Paths"] = {"PROJECT_PATH": project_path}

# Overwrite or create config.ini
with open(config_file, "w") as file:
    for section in config.sections():
        file.write(f"[{section}]\n")
        for key, value in config[section].items():
            file.write(f"{key}={value}\n")
