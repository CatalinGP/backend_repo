#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>

extern std::string PROJECT_PATH;

void loadProjectPathForMCU();
void loadProjectPathForECU();

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
#endif
