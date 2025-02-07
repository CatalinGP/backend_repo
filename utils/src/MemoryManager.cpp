#include <unistd.h>
#include <sys/stat.h>
#include <iomanip>
#include <fstream>
#include <fcntl.h>

#include "MemoryManager.h"

MemoryManager* MemoryManager::instance = nullptr;
bool MemoryManager::dev_loop_path_configured = false;

MemoryManager* MemoryManager::getInstance(Logger& logger)
{
    if (instance == nullptr)
    {
        instance = new MemoryManager(DEV_LOOP_PARTITION_1_ADDRESS_START, DEV_LOOP, logger);
        LOG_INFO(logger.GET_LOGGER(), "Memory manager instance created with default values: addres {} path {}", DEV_LOOP_PARTITION_1_ADDRESS_START, DEV_LOOP);
    }

    return instance;
}

MemoryManager* MemoryManager::getInstance(off_t address, std::string path, Logger& logger)
{
    if (instance == nullptr)
    {
        instance = new MemoryManager(address, path, logger);
    }
    return instance;
}

MemoryManager::MemoryManager(off_t address, std::string path, Logger& logger) : logger(logger)
{
    setPath(path);
    setAddress(address);
} 

void MemoryManager::setAddress(off_t address)
{
    if(availableAddress(address) == 0)
    {
        return;
    }

    LOG_INFO(logger.GET_LOGGER(), "Address {} valid and set.", address);
    this->address = address;
    this->address_continue_to_write = address;
}

void MemoryManager::setPath(std::string path)
{
    if(path == this->path)
    {
        LOG_INFO(logger.GET_LOGGER(), "Path {} already set.", path);
        return;
    }
    if(path == "" || path == "-a")
    {
        LOG_INFO(logger.GET_LOGGER(), "Path {} invalid", path);
        return;  
    }
    std::string command = "losetup " + std::string(path) + " | grep -q 'sdcard.img'";
    if(system(command.c_str()) != 0)
    {
        LOG_WARN(logger.GET_LOGGER(), "Path {} not linked to sdcard.", path);
        return;
    }

    this->path = path;
    LOG_INFO(logger.GET_LOGGER(), "Path {} exists and is assigned. Granting 777 permisions..", path);
    command = "sudo chmod 777 " + std::string(path) + " >/dev/null 2>&1";
    if(system(command.c_str()) != 0)
    {
        LOG_WARN(logger.GET_LOGGER(), "777 permisions could not be granted for {}", path);
        return;
    }

    LOG_INFO(logger.GET_LOGGER(), "777 permisions granted for {}", path);
    MemoryManager::dev_loop_path_configured = true;
}

off_t MemoryManager::getAddress()
{
    return this->address;
}

std::string MemoryManager::getPath()
{
    return this->path;
}

void MemoryManager::resetInstance()
{
    delete instance;
    instance = nullptr;
}

std::string MemoryManager::runCommand(char command[])
{
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(command, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL)
        {
            result += buffer;
        }
    } catch (...)
    {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

bool MemoryManager::availableAddress(off_t address)
{
    if(dev_loop_path_configured == false)
    {
        LOG_WARN(logger.GET_LOGGER(), "Error: DEV_LOOP not configured.");
        return false;
    }
    if (address == -1)
    {
        LOG_ERROR(logger.GET_LOGGER(), "Error: the address was not initialized correctly.");
        return false;
    }

    char verify_address_command[256];
    sprintf(verify_address_command, "sudo fdisk -l %s | grep '^/dev/' | awk '{print $2,$3}'", path.c_str());

    std::string result = runCommand(verify_address_command);
    if (result.length() == 0)
    {
        LOG_WARN(logger.GET_LOGGER(), "Error at reading addresses of the path {}", path);
        return false;
    }
    uint32_t start_p1, end_p1, start_p2, end_p2;
    std::istringstream stream_result(result);
    stream_result >> start_p1 >> end_p1 >> start_p2 >> end_p2;
    
    if(address < start_p1 || address > end_p2)
    {
        LOG_WARN(logger.GET_LOGGER(), "Address {} is not in valid range.", address);
        return false;
    }
    return true;
}

bool MemoryManager::availableMemory(off_t size_of_data)
{
    if(dev_loop_path_configured == false)
    {
        LOG_WARN(logger.GET_LOGGER(), "Error: DEV_LOOP not configured.");
        return false;
    }
    char verify_memory_command[256];
    sprintf(verify_memory_command, "sudo fdisk -l %s | grep '^/dev/' | grep -v '*' | awk '{print $3}'", path.c_str());
    std::string result = runCommand(verify_memory_command);
    if (result.length() < 3)
    {
        LOG_WARN(logger.GET_LOGGER(), "No partition found");
        return false;
    }
    off_t last_memory_address = std::stoi(result) * SECTOR_SIZE;
    if ( address + size_of_data >= last_memory_address)
    {
        LOG_ERROR(logger.GET_LOGGER(), "Error: Not enough memory.");
        return false;
    }
    return true;
}

bool MemoryManager::writeToAddress(std::vector<uint8_t>& data) 
{
    if(dev_loop_path_configured == false)
    {
        LOG_WARN(logger.GET_LOGGER(), "Error: DEV_LOOP not configured. Could not writeToAddress");
        return false;
    }

    if(!availableAddress(this->address_continue_to_write) || !availableMemory(data.size()))
    {
        LOG_ERROR(logger.GET_LOGGER(), "Error: Aborting.");
        return false;
    }
    int sd_fd = open(path.c_str(), O_RDWR );
    if (sd_fd < 0)
    {
        perror("Error opening SD card device");
        LOG_ERROR(logger.GET_LOGGER(), "Error opening SD card device: {} ", path);
        return false;
    }

    if (lseek(sd_fd, address_continue_to_write, SEEK_SET) < 0)
    {
       LOG_ERROR(logger.GET_LOGGER(), "Error seeking to address: " + std::to_string(address_continue_to_write));
        close(sd_fd);
        return false;
    }

    ssize_t bytes_written = write(sd_fd, data.data(), data.size());
    std::cout << "\nbytes written in memory: " << bytes_written << std::endl;
    if (bytes_written != static_cast<ssize_t>(data.size()))
    {
        LOG_ERROR(logger.GET_LOGGER(), "Error writing data to address: " + std::to_string(address));
        close(sd_fd);
        return false;
    }

    close(sd_fd);
    LOG_INFO(logger.GET_LOGGER(), "Data successfully written to address " + std::to_string(address) + " on "+path);
    address_continue_to_write += data.size();
    return true;
}

bool MemoryManager::writeToFile(std::vector<uint8_t> &data, std::string path_file, Logger& logger)
{
    std::ofstream sd_card(path_file, std::ios::out | std::ios::binary | std::ios::app);
    if (!sd_card.is_open())
    {
        LOG_ERROR(logger.GET_LOGGER(), "Error opening file: " + path_file);
        return false;
    }

    sd_card.write(reinterpret_cast<const char*>(data.data()), data.size());

    if (sd_card.fail())
    {
        LOG_ERROR(logger.GET_LOGGER(), "Failed to write to file: " + path_file);
        sd_card.close();
        return false;
    }

    sd_card.close();
    LOG_INFO(logger.GET_LOGGER(), "Successfully written to file: " + path_file);
    return true;
}

std::vector<uint8_t> MemoryManager::readBinary(std::string path_to_binary, Logger& logger)
{
    std::fstream sd_card;
    sd_card.open(path_to_binary, std::fstream::in | std::fstream::binary);
    if (!sd_card.is_open())
    {
        LOG_ERROR(logger.GET_LOGGER(), "Error opening SD card device: {} ", path_to_binary);
        return {};
    }
    sd_card.seekg(0, std::ios::end);
    if (sd_card.fail())
    {   
        LOG_ERROR(logger.GET_LOGGER(), "Error: Could not seek the address");
        sd_card.close();
        return {};
    }
    std::streamsize size = sd_card.tellg();
    sd_card.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!sd_card.read(buffer.data(),size))
    {
        LOG_ERROR(logger.GET_LOGGER(), "Failed to read from file " + path_to_binary);
        return {};
    }
    sd_card.close();
    std::vector<uint8_t> uint8Buffer(buffer.begin(), buffer.end());
    LOG_INFO(logger.GET_LOGGER(), "Successfully read from file: " + path_to_binary);
    return uint8Buffer;
}

std::vector<uint8_t> MemoryManager::readFromAddress(std::string path, off_t address_start, off_t size, Logger& logger)
{
    if(dev_loop_path_configured == false)
    {
        throw std::runtime_error("Error: DEV_LOOP not configured. Could not readFromAddress");
    }
    if (! instance->availableAddress(address_start))
    {
        LOG_ERROR(logger.GET_LOGGER(), "Error trying to read from address: " + std::to_string(address_start));
        return {};
    }
    int sd_fd = open(path.c_str(), O_RDWR );
    if (sd_fd < 0)
    {
        LOG_ERROR(logger.GET_LOGGER(), "Error opening SD card device: " + path);
        return {};
    }

    if (lseek(sd_fd, address_start, SEEK_SET) < 0)
    {
        LOG_ERROR(logger.GET_LOGGER(), "Error seeking to address: " + std::to_string(address_start));
        close(sd_fd);
        return {};
    }
    std::vector<char> buffer(size);
    auto bytes_readed = read(sd_fd, buffer.data(),size);
    if (bytes_readed != size)
    {
         LOG_ERROR(logger.GET_LOGGER(), "Failed to read the file " + path);
        close(sd_fd);
        return {};
    }

    close(sd_fd);
    LOG_INFO(logger.GET_LOGGER(), "Data successfully readed from address " + std::to_string(address_start) + " on "+path );
    std::vector<uint8_t> uint8Buffer(buffer.begin(), buffer.end());
    return uint8Buffer;
}

/*
void print_bin_data(std::vector<uint8_t> &data)
{
    int i=0;
    for(auto element : data)
    {
        std::cout<<std::hex<< std::setw(2) << std::setfill('0') <<(unsigned int)element<< " ";
        if ((i + 1) % 16 == 0)
        {
            std::cout << std::endl;
        }
        i++;
    }
    std::cout<<"\n";
}
*/