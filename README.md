# Steps to Set Up and Run the Backend
1. Install Required Programs  
   Ensure all necessary programs are installed in the specified versions.  

2. Clone the Project  
   Clone the repository to your local machine.  

3. Create the Key File  
   - Create a JSON file at `PoC/key.json`.  
   - Add the key from the shared drive into this file.  

4. Configure `PROJECT_PATH`  
   - Open `PoC/src/backend/config/config.ini`.  
   - Add the absolute path to your `PoC/src` directory in this file to correctly set the `PROJECT_PATH`.  

5. Create the Environment and Attach the SD Card Image  
   - Run the script `PoC/src/backend/autoscripts/create_sd_card.sh`.  
   - This will create the environment and attach the SD card image to `/dev/loop1000`.  

6. Run the Backend  
   - Launch `./main_mcu` and `rest_api` within the created environment.  
