#ifndef ASYNCBUFFER
#define ASYNCBUFFER

#include <io/buffers/datasource.h>
#include <future>

template<typename T>
struct AsyncBuffer : public DataSource {
    std::future<T> future;

public:
    AsyncBuffer(std::future<T> future) : future(std::move(future)) {}
    bool IsReady() const noexcept {
    return false;
    }
    operator bool() const noexcept { return true; }
    bool Intact() const noexcept { return true; }
};


#endif // ASYNCBUFFER

