#include "MCUModule.h"

Logger* MCULogger = nullptr;
namespace MCU
{
    MCUModule* mcu = nullptr;
    std::map<uint8_t, double> MCUModule::timing_parameters;
    std::map<uint8_t, std::future<void>> MCUModule::active_timers;
    std::map<uint8_t, std::atomic<bool>> MCUModule::stop_flags;
    std::unordered_map<uint16_t, std::vector<uint8_t>> MCUModule::default_DID_MCU = 
        {
            /** Vehicle Identification Number (VIN) */
            {0xF190, {0x31, 0x48, 0x47, 0x43, 0x4D, 0x38, 0x32, 0x36, 0x33, 0x3A, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36}},
            /** ECU Serial Number */
            {0xF17F, {0x45, 0x43, 0x55, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32}},
            /** System Supplier ECU Hardware Number */
            {0xF18C, {0x48, 0x57, 0x30, 0x30, 0x31, 0x37, 0x38, 0x35, 0x32, 0x30, 0x32, 0x32}},
            #ifdef SOFTWARE_VERSION
                /** System Supplier ECU Software Version Number */
                {0xF1A2, {static_cast<uint8_t>(SOFTWARE_VERSION)}},
            #else
                /** System Supplier ECU Software Version Number */
                {0xF1A2, {0x00}},
            #endif
            /** System Supplier ECU Software Number */
            {0xF1A0, {0x53, 0x57, 0x45, 0x43, 0x55, 0x34, 0x35, 0x36, 0x37}},
            /** System Name or Engine Type */
            {0xF187, {0x32, 0x30, 0x4C, 0x20, 0x54, 0x75, 0x72, 0x62, 0x6F, 0x20, 0x49, 0x34}},
            /** System Supplier ECU Hardware Version Number */
            {0xF1A1, {0x48, 0x56, 0x34, 0x2E, 0x35, 0x2E, 0x36}},
            /** System Supplier ECU Manufacturing Date */
            {0xF1A4, {0x32, 0x30, 0x32, 0x32, 0x2D, 0x30, 0x35, 0x2D, 0x31, 0x35}},
            /** System Supplier ECU Coding/Configuration Part Number */
            {0xF1A5, {0x43, 0x46, 0x47, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34}},
            /** System Calibration Identification Number */
            {0xF1A8, {0x43, 0x41, 0x4C, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37}},
            /** System Calibration Verification Number (CVN) */
            {0xF1A9, {0x43, 0x56, 0x4E, 0x31, 0x31, 0x32, 0x32, 0x33, 0x33}},
            /** System ECU Boot Software Identification Number */
            {0xF1AA, {0x42, 0x4F, 0x4F, 0x54, 0x33, 0x33, 0x34, 0x34, 0x35}},
            /** System ECU Application Software Identification Number */
            {0xF1AB, {0x41, 0x50, 0x50, 0x35, 0x35, 0x36, 0x36, 0x37, 0x37}},
            /** System ECU Data Set Identification Number */
            {0xF1AC, {0x44, 0x41, 0x54, 0x41, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32}},
            /** System ECU Flash Software Version Number */
            {0xF1AD, {0x46, 0x4C, 0x41, 0x53, 0x48, 0x32, 0x2E, 0x33, 0x2E, 0x34}}
        };
    const std::vector<uint16_t> MCUModule::writable_MCU_DID =
    {
        /* System Supplier ECU Software Number */
        0xF1A0,
        /* System Supplier ECU Software Version Number */
        0xF1A2,
        /* System Supplier ECU Coding/Configuration Part Number */
        0xF1A5,
        /* System Calibration Identification Number */
        0xF1A8,
        /* System Calibration Verification Number (CVN) */
        0xF1A9
    };
    /* Constructor */
    MCUModule::MCUModule(uint8_t interfaces_number) : 
                    is_running(false),
                    create_interface(CreateInterface::getInstance(interfaces_number, *MCULogger)),
                    receive_frames(nullptr),
                    mcu_api_socket(create_interface->createSocket(interfaces_number)),
                    mcu_ecu_socket(create_interface->createSocket(interfaces_number >> 4))
                    {
        writeDataToFile();
        receive_frames = new ReceiveFrames(mcu_ecu_socket, mcu_api_socket);
    }

    /* Default constructor */
    MCUModule::MCUModule() : is_running(false),
                         create_interface(CreateInterface::getInstance(0x01, *MCULogger)),
                         receive_frames(nullptr)
    {
        writeDataToFile();
    }

    /* Destructor */
    MCUModule::~MCUModule() 
    {
        create_interface->stopInterface();
        delete receive_frames;
        if(system("pkill main_mcu") != 0)
        {
            LOG_ERROR(MCULogger->GET_LOGGER(), "Error when trying to kill main_mcu process");
        }
    }

    /* Start the module */
    void MCUModule::StartModule() 
    { 
        is_running = true;
        create_interface->setSocketBlocking(mcu_api_socket);
        create_interface->setSocketBlocking(mcu_ecu_socket);
    }

    int MCUModule::getMcuApiSocket() const 
    {
        return mcu_api_socket;
    }
    int MCUModule::getMcuEcuSocket() const 
    {
        return mcu_ecu_socket;
    }

    void MCUModule::setMcuApiSocket(uint8_t interface_number)
    {
        this->mcu_api_socket = this->create_interface->createSocket(interface_number);
    }
    
    void MCUModule::setMcuEcuSocket(uint8_t interface_number)
    {
        this->mcu_ecu_socket = this->create_interface->createSocket(interface_number >> 4);
    }

    /* Stop the module */
    void MCUModule::StopModule() 
    { 
        is_running = false;
        receive_frames->stopProcessingQueue();            
        receive_frames->stopListenAPI();
        receive_frames->stopListenCANBus(); 
    }

    /* Receive frames */
    void MCUModule::recvFrames() 
    {
        while (is_running)
        {
            receive_frames->startListenAPI();
            receive_frames->startListenCANBus();
            /* Start a thread to process the queue */
            std::thread queue_thread_process(&ReceiveFrames::processQueue, receive_frames);

            /* Start a thread to listen on API socket */
            std::thread queue_thread_listen(&ReceiveFrames::receiveFramesFromAPI, receive_frames);

            /* Receive frames from the CAN bus */
            receive_frames->receiveFramesFromCANBus();

            receive_frames->stopListenAPI();
            receive_frames->stopListenCANBus();

            /* Wait for the threads to finish */
            queue_thread_process.join();
            queue_thread_listen.join();
        }
    }

    void MCUModule::fetchMCUData()
    {
        /* Path to mcu data file */
        std::string file_path = "mcu_data.txt";

        /* Generate random values for each DID */
        std::unordered_map<uint16_t, std::string> updated_values;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 255);

        for (auto& [did, data] : default_DID_MCU)
        {
            if(did != 0xe001 && did != 0xf1a2)
            {
                std::stringstream data_ss;
                for (auto& byte : data)
                {
                    byte = dist(gen);  
                    /* Generate a random value between 0 and 255 */
                    data_ss << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << static_cast<int>(byte) << " ";
                }
                updated_values[did] = data_ss.str();
            }
            /* For OTA status DIDs don t generate random values */
            else
            {
                std::stringstream data_ss;
                for (auto& byte : data)
                {
                    data_ss << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << static_cast<int>(byte) << " ";
                }
                updated_values[did] = data_ss.str();
            }

        }

        /* Read the current file contents into memory */
        std::ifstream infile(file_path);
        std::stringstream buffer;
        buffer << infile.rdbuf();
        infile.close();

        std::string file_contents = buffer.str();
        std::istringstream file_stream(file_contents);
        std::string updated_file_contents;
        std::string file_line;

        /* Update the relevant DID values in the file contents */
        while (std::getline(file_stream, file_line))
        {
            for (const auto& pair : updated_values)
            {
                std::stringstream did_ss;
                did_ss << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << pair.first;
                if (file_line.find(did_ss.str()) != std::string::npos)
                {
                    updated_file_contents += did_ss.str() + " " + pair.second + "\n";
                    break;
                }
            }
        }

        /* Write the updated contents back to the file */
        std::ofstream outfile(file_path);
        outfile << updated_file_contents;
        outfile.close();

        LOG_INFO(MCULogger->GET_LOGGER(), "MCU data file updated with random values.");
    }


    void MCUModule::writeDataToFile()
    {
        std::string file_path = std::string(PROJECT_PATH) + "/backend/mcu/mcu_data.txt";
        /* Insert the default DID values in the file */
        std::ofstream outfile(file_path);
        if (!outfile.is_open())
        {
            throw std::runtime_error("Failed to open file: mcu_data.txt");
        }

        /* Check if old_mcu_data.txt exists */
        std::string old_file_path = "old_mcu_data.txt";
        std::ifstream infile(old_file_path);

        if (infile.is_open())
        {
            /* Read the current file contents into memory */
            std::stringstream buffer;
            /* Read the entire file into the buffer */
            buffer << infile.rdbuf();
            infile.close();

            /* Store the original content */
            std::string original_file_contents = buffer.str();

            /* Write the content of old_mcu_data.txt into mcu_data.txt */
            outfile << original_file_contents;

            /* Delete the old file after reading its contents */
            std::remove(old_file_path.c_str());
        }
        else
        {
            /* Write the default DID values to mcu_data.txt */
            for (const auto& [data_identifier, data] : default_DID_MCU)
            {
                outfile << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << data_identifier << " ";
                for (uint8_t byte : data)
                {
                    outfile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                }
                outfile << "\n";
            }
        }
        outfile.close();
        fetchMCUData();
    }
}