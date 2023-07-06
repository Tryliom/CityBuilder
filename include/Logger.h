#pragma once

#include <string>
#include <iostream>

#define LOG(message) std::cout << message << " from " << __FILE__ << ":" << __LINE__ << std::endl
#define LOG_ERROR(message) std::cout << "[ERROR] " << message << " from " << __FILE__ << ":" << __LINE__ << std::endl