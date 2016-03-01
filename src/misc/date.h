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
#ifndef DATE
#define DATE

#include <stdio.h>
#include <time.h>
#include <string>

struct Date {
    time_t _time;
    struct tm tm;

    public:
    Date(time_t time) : _time(time), tm(*gmtime(&_time)) {}

    bool operator<(const Date &other) { return _time < other._time; }

    static Date Now() { return Date(time(0)); }

    std::string ToString() {
        std::string text;
        text.resize(100);
        auto size = strftime(&text.front(), text.size(), "%a, %d %b %Y %H:%M:%S %Z", &tm);
        text.resize(size);
        return text;
    }
};

#endif // DATE
