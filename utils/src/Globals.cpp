#include "Globals.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>

#define __DIR__ (std::string(__FILE__).substr(0, std::string(__FILE__).find_last_of("/")))

std::string PROJECT_PATH;

void v_loadProjectPath() {
    std::string configIniFilePath = __DIR__ + std::string("/../../config/config.ini");
    std::ifstream file(configIniFilePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config.ini");
    }

    std::string line, key = "PROJECT_PATH=";
    while (std::getline(file, line)) {
        if (line.rfind(key, 0) == 0) {
            PROJECT_PATH = line.substr(key.size());
            return;
        }
    }

    throw std::runtime_error("PROJECT_PATH not found in config.ini");
}

std::string to_lowercase(const std::string& str) {
	std::string lower_str = str;
	std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), [](unsigned char c){ return std::tolower(c); });
	return lower_str;
}

int countDigits(int number) {
    int digits = 0;
    if (number < 0) 
        return 0;
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}

void stopProcess(std::string process_name)
{
    /* Use popen to capture the output of the command */
    FILE* fp = popen(("pgrep -f '" + process_name + "' | wc -l").c_str(), "r");
    if (fp == nullptr) {
        std::cerr << "Failed to run command" << std::endl;
        return;
    }

    int count = 0;
    /* Read the output of the command and check if it is valid */
    if (fscanf(fp, "%d", &count) != 1) {
        std::cerr << "Failed to read process count" << std::endl;
        pclose(fp);
        return;
    }
    pclose(fp);

    /* Loop while there are more than 2 processes running */
    while (count > 2) {
        std::cout << "More than one process found. Killing old instances..." << std::endl;

        /* Kill the oldest process */
        int result = system(("pkill -o -f '" + process_name + "'").c_str());
        if (result == 0) {
            std::cout << "Old process terminated successfully." << std::endl;
        } else {
            std::cerr << "Failed to terminate old process." << std::endl;
        }

        /* Reload the process count after killing one process */
        fp = popen(("pgrep -f '" + process_name + "' | wc -l").c_str(), "r");
        if (fp == nullptr) {
            std::cerr << "Failed to run command" << std::endl;
            return;
        }
        if (fscanf(fp, "%d", &count) != 1) {
            std::cerr << "Failed to read process count" << std::endl;
            pclose(fp);
            return;
        }
        pclose(fp);
    }
}
