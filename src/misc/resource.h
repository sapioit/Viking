#ifndef RESOURCE_H
#define RESOURCE_H

#include <cstdint>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class Resource {
    std::string _path;
    std::vector<char> _content;
    // TODO Modify to use the stat64 version
    struct stat _stat;

    public:
    Resource() = default;
    Resource(const Resource &) = default;
    Resource(const std::string &, const std::vector<char> &, const struct stat &);
    Resource(const std::string &);
    ~Resource() = default;
    operator bool();
    bool operator<(const Resource &);

    const std::uint64_t &hits() const;
    const std::vector<char> &content() const;
    const std::string &path() const;
    struct stat stat() const;
};

#endif // RESOURCE_H
