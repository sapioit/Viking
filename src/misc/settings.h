#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>

struct Settings {
    Settings();
    ~Settings() = default;
    Settings(const Settings &) = default;
    Settings(Settings &&) = default;
    Settings &operator=(const Settings &) = default;

    std::string root_path;
    std::uint32_t max_connections;
    std::uint32_t default_max_age;
    bool allow_directory_listing;
};

#endif // SETTINGS_H
