#!/bin/bash

# The script will be run as: 1. sudo ./create_sd_card.sh (when there is need to set the virtual environment setup)
# or 2. sudo ./create_sd_card.sh --only-sdcard (when there is no need of virtual environment setup, but only sd card creation). 
# This script can also be run from any directory.

# Determine the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Function to find a target directory by searching up the tree
find_directory_upwards()
{
  local TARGET_DIR="$1"
  local CURRENT_DIR="$SCRIPT_DIR"
  
  while [[ "$CURRENT_DIR" != "/" ]]; do
    if [[ -d "$CURRENT_DIR/$TARGET_DIR" ]]; then
    echo "$CURRENT_DIR/$TARGET_DIR"
    return 0
    fi
  # Go up one level
  CURRENT_DIR="$(dirname "$CURRENT_DIR")"
  done
  
  #Return 1 if not found
  return 1
}

# Ensure the script is run with sudo
if [ "$EUID" -ne 0 ]; then
  echo "Please run with sudo"
  exit
fi

# Check the only sdcard argument - run the script with "--only-sdcard" argument if there is no need of virtual environment setup
ONLY_SDCARD=false
if [ "$1" == "--only-sdcard" ]; then
  ONLY_SDCARD=true
fi

PYTHON_VERSION=$(python3 -c 'import sys; print(".".join(map(str, sys.version_info[:2])))')
REQUIRED_VERSION="3.8"

if [ "$(printf '%s\n' "$REQUIRED_VERSION" "$PYTHON_VERSION" | sort -V | head -n1)" != "$REQUIRED_VERSION" ]; then
  echo "Python 3.8 or higher is required. Please install the appropriate version."
  exit 1
fi

echo "Python version is $PYTHON_VERSION. Proceeding..."

# Skip venv setup if --only-sdcard is provided
if [ "$ONLY_SDCARD" = false ]; then
  # Step 1: Change directory to ../rest_api, but first find the rest_api directory by searching upwards from the script location
  REST_API_DIR=$(find_directory_upwards "rest_api")
 
  if [[ -z "$REST_API_DIR" ]]; then
    echo "rest_api directory not found. Exiting."
    exit 1
  else
    echo "Found rest_api directory at $REST_API_DIR"
  fi

  cd "$REST_API_DIR" || exit 1

  # Step 2
  # Check if the virtual environment exists
  if [ ! -d "venv" ]; then
      echo "Creating virtual environment..."
      python3.8 -m venv venv
      # Activate the virtual environment
      source venv/bin/activate
      echo "Virtual environment activated."
      pip install -r requirements.txt

      # Optional: Check if the activation was successful
      if [ "$VIRTUAL_ENV" != "" ]; then
          echo "Virtual environment activated: $VIRTUAL_ENV"
          pip install -r requirements.txt
          echo "Requirements installed."
      else
          echo "Failed to activate the virtual environment."
      fi
  else
      echo "Virtual environment found."
      # Activate the virtual environment
      source venv/bin/activate
      if [ "$VIRTUAL_ENV" != "" ]; then
          echo "Virtual environment activated: $VIRTUAL_ENV"
      else
          echo "Failed to activate the virtual environment."
      fi
  fi

# Step 4: Change directory back to ../mcu
MCU_DIR=$(find_directory_upwards "mcu")
 
  if [[ -z "$MCU_DIR" ]]; then
    echo "mcu directory not found. Exiting."
    exit 1
  else
    echo "Found mcu directory at $MCU_DIR"
  fi

  cd "$MCU_DIR" || exit 1

fi

#Set IMG_PATH in order to create the sd card image in the user's home directory
USER_HOME=$(eval echo ~"$SUDO_USER")
IMG_PATH="$USER_HOME/sdcard.img"
LOOP_DEVICE=""
PART1_SIZE="+100M"
START_SECTOR=2048
SEEK_OFFSET=118006272
DD_COUNT=17168
MOUNT_DIR="/mnt/sdcard"

# Step 7: Check if the image already exists
if [ -f "$IMG_PATH" ]; then
   echo "Image already exists at $IMG_PATH."
   
  # Step 6: Search and remove any existing loop device associated with sdcard.img
  EXISTING_LOOP_DEVICE=$(losetup -j "$IMG_PATH" | cut -d: -f1)

  if [ -n "$EXISTING_LOOP_DEVICE" ]; then
    echo "Found existing loop device: $EXISTING_LOOP_DEVICE."
    
    # Detach the loop device
    echo "Detaching loop device $EXISTING_LOOP_DEVICE..."
    sudo losetup -d "$EXISTING_LOOP_DEVICE"

    # Verify that the loop device has been successfully detached
    if losetup -a | grep -q "$IMG_PATH"; then
      echo "Loop device $EXISTING_LOOP_DEVICE could not be detached. Forcing detachment..."
      sudo losetup -D  # Force-detach all loop devices
    fi
  fi

  # Use a specific loop /dev/loop20
  LOOP_DEVICE="/dev/loop20"

  # Detach the existing loop device if /dev/loop10 is already in use
  if losetup | grep -q "$LOOP_DEVICE"; then
    echo "Detaching existing $LOOP_DEVICE..."
    sudo losetup -d "$LOOP_DEVICE"
  fi

  # Attach the image file to the specified loop device
  sudo losetup "$LOOP_DEVICE" "$IMG_PATH"
  echo "Loop device set to: $LOOP_DEVICE"

  # Extract the loop number from LOOP_DEVICE
  LOOP_NUMBER=$(basename "$LOOP_DEVICE" | grep -o '[0-9]*')

  # Ensure the loop device has appropriate permissions
  sudo chmod 777 "$LOOP_DEVICE"
  
else

  echo "Creating image..."
  sudo truncate -s +300M "$IMG_PATH"

  # Use a specific loop /dev/loop20
  LOOP_DEVICE="/dev/loop20"

  # Step 8: Create a loop device and attach it to the specified loop device
  sudo losetup "$LOOP_DEVICE" "$IMG_PATH"
  echo "Loop device created and attached to $LOOP_DEVICE"

  # Extract the loop number from LOOP_DEVICE
  LOOP_NUMBER=$(basename "$LOOP_DEVICE" | grep -o '[0-9]*')

  # Step 10: Create partitions
  echo "Creating partitions..."
  (
  echo n # New partition
  echo p # Primary
  echo 1 # Partition number
  echo $START_SECTOR # First sector
  echo $PART1_SIZE # Last sector
  echo n # New partition
  echo p # Primary
  echo 2 # Partition number
  echo   # First sector (Accept default)
  echo   # Last sector (Accept default)
  echo w # Write changes
  ) | sudo fdisk "$LOOP_DEVICE"

  sudo partprobe "$LOOP_DEVICE"

  # Step 11: Formatting the partitions
  echo "Formatting partitions..."
  sudo mkfs.fat -F 32 "${LOOP_DEVICE}p1"
  sudo mkfs.fat -F 16 "${LOOP_DEVICE}p2"

  # Step 12: Change permissions of the loop device
  sudo chmod 777 "$LOOP_DEVICE"

  # Step 13: Create mount directory
  echo "Creating mount directory..."
  mkdir -p "$MOUNT_DIR"

  # Step 14: Mount partitions
  echo "Mounting partitions..."
  sudo mount -o rw,uid=1000,gid=1000 "${LOOP_DEVICE}p1" "$MOUNT_DIR"
  sudo mount -o rw,uid=1000,gid=1000 "${LOOP_DEVICE}p2" "$MOUNT_DIR"

  # Step 15: Display partition data
  # echo "Reading data from offset $SEEK_OFFSET..."
  # xxd -l $DD_COUNT -s $SEEK_OFFSET "$LOOP_DEVICE"

  # Step 16: Zero out the data at specified offset
  # echo "Zeroing out data at offset $SEEK_OFFSET..."
  # sudo dd if=/dev/zero of="$LOOP_DEVICE" bs=1 seek=$SEEK_OFFSET count=$DD_COUNT conv=notrunc

  # Display the created partitions
  echo "Partitions created:"
  sudo lsblk -lno NAME,TYPE,SIZE "$LOOP_DEVICE"
fi

# Step 17: Navigate back to initial location
cd "$SCRIPT_DIR"

# Step 18: Navigate backwards to find the 'src' directory
while [[ "$PWD" != "/" && "${PWD##*/}" != "src" ]]; do
  cd ..
done

# Check if we are in the 'src' directory
if [[ "${PWD##*/}" != "src" ]]; then
  echo "'src' directory not found. Exiting."
  exit 1
fi

echo "Found 'src' directory at $PWD"
SRC_DIR="$PWD"

# Step 19: Replace loop numbers in file MemoryManager.h
echo "Replacing loop numbers in file MemoryManager.h"
find ./backend/utils/include -type f -name "MemoryManager.h" -exec sed -i "s/\/dev\/loop[0-9]*/\/dev\/loop${LOOP_NUMBER}/g" {} +

# Navigate to SRC_DIR, go downwards into mcu folder, and run make commands
cd "$SRC_DIR" || { echo "'src' directory not found. Exiting."; exit 1; }

# Go downwards into mcu folder
cd ./backend/mcu || { echo "'mcu' directory not found. Exiting."; exit 1; }
# Run make clean command for mcu
echo "Running make command for mcu..."
make clean

# Go downwards into battery folder
cd ./../ecu_simulation/BatteryModule || { echo "'BaterryModule' directory not found. Exiting."; exit 1; }
# Run make clean command for battery
echo "Running make command for battery..."
make clean

# Go downwards into doors folder
cd ./../DoorsModule || { echo "'DoorsModule' directory not found. Exiting."; exit 1; }
# Run make clean command for doors
echo "Running make command for doors..."
make clean

# Go downwards into engine folder
cd ./../EngineModule || { echo "'EngineModule' directory not found. Exiting."; exit 1; }
# Run make clean command for engine
echo "Running make command for engine..."
make clean

# Go downwards into HVAC folder
cd ./../HVACModule || { echo "'HVACodule' directory not found. Exiting."; exit 1; }
# Run make clean command for HVAC
echo "Running make command for HVAC..."
make clean

echo "Make clean done for mcu and ECUs"

# Info message:
YELLOW='\033[0;33m'
NC='\033[0m'
echo -e "${YELLOW}MCU and ECUs need to be compiled. Run the command 'make'.${NC}"

echo "Script completed successfully."