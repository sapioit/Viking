#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
//#define LOGGING
#ifdef LOGGING
#define debug(x) std::cout << x << std::endl;
#else
#define debug(x)
#endif

//void debug(const std::string &message) noexcept;

#endif // DEBUG_H
