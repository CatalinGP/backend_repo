import os
import sys
import subprocess
import shutil

"""
    STEPS TO DO BEFORE RUNNING THE SCRIPT

1.Create and activate the venv, install dependencies
    cd /src/rest_api
    python3.8 -m venv venv
    source venv/bin/activate
    pip install -r requirements.txt
2.Add key.json to PoC folder
  Copy the key.json from drive PoC folder

    HOW TO USE THE SCRIPT

    command: python3 ReleaseScript.py [version_number] [what_to_create: mcu/ecu/all]
    example: python3 ReleaseScript.py 1 all
"""
# Colors used for printing messages
RED = '\033[91m'
GREEN = '\033[92m'
YELLOW = '\033[93m'
RESET = '\033[0m'

PROJECT_PATH = os.path.abspath(os.path.join(os.getcwd(), "..", ".."))
PATH_SOFTWARE_RELEASES = PROJECT_PATH + '/software_releases'

mcu_size_uncompressed = 0
battery_size_uncompressed = 0
engine_size_uncompressed = 0
doors_size_uncompressed = 0
hvac_size_uncompressed = 0


def create_exec(version: str, sw_to_build: str, extra_args: str):
    dir_name = "release_" + sw_to_build + '_' + version
    directory_path = os.path.expanduser(PATH_SOFTWARE_RELEASES + "/"+dir_name)
    version_integer = validateSoftwareVersion(version, convert=True)
    if subprocess.run(["ls {0}/{1} 2>/dev/null".format(PATH_SOFTWARE_RELEASES, dir_name)], shell=True).returncode == 0:
        print(f"{YELLOW}release_{version}_{sw_to_build} already exists locally{RESET}")
        user_input = input(
            f"{GREEN}Do you want to delete it and create a new one? y/n: {RESET}")
        if user_input.lower() == 'y':
            shutil.rmtree(directory_path)
        else:
            user_input = input(
                f"{GREEN}Upload to Google Drive?(only if the release already exists locally) y/n: {RESET}")
            if user_input.lower() == 'y':
                uploadRelease(directory_path)
                exit(1)
            elif user_input.lower() == 'n':
                print(f"{GREEN}Not uploading.{RESET}")
            else:
                print(f"{RED}Invalid input.{RESET}")
            exit(-1)
    try:
        # Create the directory
        subprocess.run(
            ["mkdir", "-p", "{0}/{1}".format(PATH_SOFTWARE_RELEASES, dir_name)], check=True)
        if sw_to_build == "mcu" or sw_to_build == "all":
            if "clean" in extra_args:
                subprocess.run(
                    ["sudo", "make", "clean", "-C", "{0}/../src/mcu/".format(PATH_SOFTWARE_RELEASES)], check=True)
            # Build the MCU executable
                subprocess.run(
                    ["sudo", "make", f"SOFTWARE_VERSION={version_integer}", "-C", "{0}/../src/mcu/".format(PATH_SOFTWARE_RELEASES)], check=True)
            mcu_executable_path = "{0}/../src/mcu/main_mcu".format(
                PATH_SOFTWARE_RELEASES)
            mcu_archive_name = "{0}/{1}/MCU_SW_VERSION_{2}".format(
                PATH_SOFTWARE_RELEASES, dir_name, version)
            global mcu_size_uncompressed
            mcu_size_uncompressed = createArchive(mcu_executable_path, mcu_archive_name)
        if sw_to_build == "battery" or sw_to_build == "all":
            if "clean" in extra_args:
                subprocess.run(
                    ["sudo", "make", "clean", "-C", "{0}/../src/ecu_simulation/BatteryModule".format(PATH_SOFTWARE_RELEASES)], check=True)
            # Build the Battery Module executable
            subprocess.run(
                ["sudo", "make", f"SOFTWARE_VERSION={version_integer}", "-C", "{0}/../src/ecu_simulation/BatteryModule".format(PATH_SOFTWARE_RELEASES)], check=True)
            battery_executable_path = "{0}/../src/ecu_simulation/BatteryModule/main_battery".format(
                PATH_SOFTWARE_RELEASES)
            battery_archive_name = "{0}/{1}/ECU_BATTERY_SW_VERSION_{2}".format(
                PATH_SOFTWARE_RELEASES, dir_name, version)
            global battery_size_uncompressed
            battery_size_uncompressed = createArchive(
                battery_executable_path, battery_archive_name)
        if sw_to_build == "engine" or sw_to_build == "all":
            if "clean" in extra_args:
                subprocess.run(
                    ["sudo", "make", "clean", "-C", "{0}/../src/ecu_simulation/BatteryModule".format(PATH_SOFTWARE_RELEASES)], check=True)
            # Build the engine Module executable
            subprocess.run(
                ["sudo", "make", f"SOFTWARE_VERSION={version_integer}", "-C", "{0}/../src/ecu_simulation/EngineModule".format(PATH_SOFTWARE_RELEASES)], check=True)
            engine_executable_path = "{0}/../src/ecu_simulation/EngineModule/main_engine".format(
                PATH_SOFTWARE_RELEASES)
            engine_archive_name = "{0}/{1}/ECU_ENGINE_SW_VERSION_{2}".format(
                PATH_SOFTWARE_RELEASES, dir_name, version)
            global engine_size_uncompressed
            engine_size_uncompressed = createArchive(
                engine_executable_path, engine_archive_name)
        if sw_to_build == "doors" or sw_to_build == "all":
            if "clean" in extra_args:
                subprocess.run(
                    ["sudo", "make", "clean", "-C", "{0}/../src/ecu_simulation/DoorsModule".format(PATH_SOFTWARE_RELEASES)], check=True)
            # Build the doors Module executable
            subprocess.run(
                ["sudo", "make", f"SOFTWARE_VERSION={version_integer}", "-C", "{0}/../src/ecu_simulation/DoorsModule".format(PATH_SOFTWARE_RELEASES)], check=True)
            doors_executable_path = "{0}/../src/ecu_simulation/DoorsModule/main_doors".format(
                PATH_SOFTWARE_RELEASES)
            doors_archive_name = "{0}/{1}/ECU_DOORS_SW_VERSION_{2}".format(
                PATH_SOFTWARE_RELEASES, dir_name, version)
            global doors_size_uncompressed
            doors_size_uncompressed = createArchive(
                doors_executable_path, doors_archive_name)
        if sw_to_build == "hvac" or sw_to_build == "all":
            if "clean" in extra_args:
                subprocess.run(
                    ["sudo", "make", "clean", "-C", "{0}/../src/ecu_simulation/HVACModule".format(PATH_SOFTWARE_RELEASES)], check=True)
            # Build the hvac Module executable
            subprocess.run(
                ["sudo", "make", f"SOFTWARE_VERSION={version_integer}", "-C", "{0}/../src/ecu_simulation/HVACModule".format(PATH_SOFTWARE_RELEASES)], check=True)
            hvac_executable_path = "{0}/../src/ecu_simulation/HVACModule/main_hvac".format(
                PATH_SOFTWARE_RELEASES)
            hvac_archive_name = "{0}/{1}/ECU_HVAC_SW_VERSION_{2}".format(
                PATH_SOFTWARE_RELEASES, dir_name, version)
            global hvac_size_uncompressed
            hvac_size_uncompressed = createArchive(
                hvac_executable_path, hvac_archive_name)
        user_input = input(f"{GREEN}Upload to Google Drive? y/n: {RESET}")
        if user_input.lower() == 'y':
            uploadRelease(directory_path)
        elif user_input.lower() == 'n':
            print(f"{GREEN}Not uploading.{RESET}")
        else:
            print(f"{RED}Invalid input.{RESET}")
    except subprocess.CalledProcessError as error:
        print(error)
    except FileNotFoundError as error:
        print(error)
    return directory_path


def createArchive(executable_path, archive_name):
    size_uncompressed = int((os.path.getsize(executable_path) / 1000000)) + 1
    shutil.make_archive(archive_name, 'zip', root_dir=os.path.dirname(
        executable_path), base_dir=os.path.basename(executable_path))
    return size_uncompressed


def uploadRelease(directory_path: str):
    google_drive_api_path = PROJECT_PATH + '/src/ota/google_drive_api'
    sys.path.append(google_drive_api_path)
    from GoogleDriveApi import gDrive, DRIVE_MCU_SW_VERSIONS_FILE, DRIVE_ECU_BATTERY_SW_VERSIONS_FILE, DRIVE_ECU_ENGINE_SW_VERSIONS_FILE, DRIVE_ECU_DOORS_SW_VERSIONS_FILE, DRIVE_ECU_HVAC_SW_VERSIONS_FILE

    for file in os.listdir(directory_path):

        if "MCU" in file:
            gDrive.uploadFile(file, os.path.join(
                directory_path, file), mcu_size_uncompressed, DRIVE_MCU_SW_VERSIONS_FILE)
        elif "BATTERY" in file:
            gDrive.uploadFile(file, os.path.join(
                directory_path, file), battery_size_uncompressed, DRIVE_ECU_BATTERY_SW_VERSIONS_FILE)
        elif "ENGINE" in file:
            gDrive.uploadFile(file, os.path.join(
                directory_path, file), engine_size_uncompressed, DRIVE_ECU_ENGINE_SW_VERSIONS_FILE)
        elif "DOORS" in file:
            gDrive.uploadFile(file, os.path.join(
                directory_path, file), doors_size_uncompressed, DRIVE_ECU_DOORS_SW_VERSIONS_FILE)
        elif "HVAC" in file:
            gDrive.uploadFile(file, os.path.join(
                directory_path, file), hvac_size_uncompressed, DRIVE_ECU_HVAC_SW_VERSIONS_FILE)


def validateSoftwareVersion(software_version, convert=False):
    if "." not in software_version:
        print(f"{RED}Version not valid, must look like 1.1, 1.6 etc{RESET}")
        exit(-1)

    major_version, minor_version = software_version.split('.')
    major_version = int(major_version)
    minor_version = int(minor_version)

    if major_version < 1 or major_version > 16 or minor_version < 0 or minor_version > 15:
        print(f"{RED}Version not valid. Valid versions are between 1.0 and 16.15{RESET}")
        exit(-1)

    if convert :
        return ((major_version - 1) << 4 | minor_version)

    return software_version


def validateSoftwareToBuild(software_to_build):
    # TODO
    return software_to_build


def main():
    version = "1.0"
    sw_to_build = "all"
    extra_args = ""

    if len(sys.argv) < 2:
        print(f"{RED}Please provide as first argument the version of the release using format 1.0 -> 1.15 and as second argument the software to build mcu/ecu/all{RESET}")
        exit(-1)
    else:
        version = validateSoftwareVersion(sys.argv[1], convert=False)
        sw_to_build = validateSoftwareToBuild(sys.argv[2])
        extra_args = sys.argv[3:]

    create_exec(version, sw_to_build, extra_args)


if __name__ == "__main__":
    main()
