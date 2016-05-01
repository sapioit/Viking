/*
Copyright (C) 2015 Voinea Constantin Vladimir

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
#ifndef DEBUG_H
#define DEBUG_H

#define LOGGING
#ifdef LOGGING
#include <iostream>
#define debug(x) std::cout << x << std::endl;
#else
#define debug(x)
#endif

void dump_backtrace();

#define BUG_ON(x, t)                                                                                                   \
    if (x)                                                                                                             \
        debug(t);                                                                                                      \
    exit(EXIT_FAILURE);

#define WARN_ON(x, t)                                                                                                  \
    if (x) {                                                                                                           \
        debug(t);                                                                                                      \
        dump_backtrace();                                                                                              \
    }

#endif // DEBUG_H
