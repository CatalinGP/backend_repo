#include "Globals.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>

std::string PROJECT_PATH;

void loadProjectPathForMCU() {
    std::ifstream file("../config/config.ini");
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

void loadProjectPathForECU() {
    std::ifstream file("../../config/config.ini");
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
