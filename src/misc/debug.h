#ifndef DEBUG_H
#define DEBUG_H

//#define LOGGING
#ifdef LOGGING
#include <iostream>
#define debug(x) std::cout << x << std::endl;
#else
#define debug(x)
#endif

// void debug(const std::string &message) noexcept;

#endif // DEBUG_H
