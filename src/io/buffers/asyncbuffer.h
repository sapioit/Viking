#ifndef ASYNCBUFFER
#define ASYNCBUFFER

#include <io/buffers/datasource.h>
#include <future>

template<typename T>
struct AsyncBuffer : public DataSource {
    std::future<T> future;

    bool IsReady() const noexcept {
    return false;
    }
};


#endif // ASYNCBUFFER

