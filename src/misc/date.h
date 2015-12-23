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
