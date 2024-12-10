#include "Globals.h"
#include <fstream>
#include <stdexcept>

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
