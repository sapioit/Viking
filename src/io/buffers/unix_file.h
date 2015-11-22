#ifndef UNIX_FILE_H
#define UNIX_FILE_H

#include <io/buffers/datasource.h>

#include <sys/sendfile.h>
#include <sys/types.h>
#include <functional>
#include <string>

struct UnixFile : public DataSource {
    int fd = -1;
    off64_t offset = 0;
    off64_t size = 0;

    public:
    typedef std::function<int(const std::string &)> AquireFunction;
    typedef std::function<void(int)> ReleaseFunction;

    private:
    std::function<int(const std::string &)> aquire_func_;
    std::function<void(int)> release_func_;
    void Close();

    public:
    struct Error {
        std::string path;
        Error() = default;
        virtual ~Error() = default;
        Error(const std::string &path) : path(path) {}
    };
    struct BadFile {
        const UnixFile *ptr;
    };

    struct DIY {
        const UnixFile *ptr;
    };
    struct BrokenPipe {
        const UnixFile *ptr;
    };

    UnixFile() = default;
    virtual ~UnixFile();
    UnixFile(const std::string &, AquireFunction a, ReleaseFunction r);
    UnixFile(UnixFile &&);
    UnixFile &operator=(UnixFile &&);
    UnixFile(const UnixFile &) = delete;
    UnixFile &operator=(const UnixFile &) = delete;
    virtual operator bool() const noexcept;
    bool SendTo(int);
};

#endif // UNIX_FILE_H
