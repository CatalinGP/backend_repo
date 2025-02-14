#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <iostream>

extern std::string PROJECT_PATH;

/**
 * @brief find the config.ini file and load project path from it
 */
void v_loadProjectPath();

/**
 * @brief Convert all the letters to lowercase
 * 
 */
std::string to_lowercase(const std::string& str);

/**
 * @brief Count the number of digits in a number
 * 
 * @param number The integer whose digits are to be counted.
 * @return The number of digits for a given integer.
 */
int countDigits(int number);
/**
 * @brief Method to stop all the processes with the given name, the current one.
 * 
 * @param process_name Process name to be stopped.
 */
void stopProcess(std::string process_name);

/* Enumeration for frame types */
enum FrameType {
    DATA_FRAME,
    REMOTE_FRAME,
    ERROR_FRAME,
    OVERLOAD_FRAME
};

#endif
