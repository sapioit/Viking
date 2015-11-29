#ifndef ASYNCBUFFER
#define ASYNCBUFFER

#include <io/buffers/datasource.h>
#include <future>

template <typename T> struct AsyncBuffer : public DataSource {
    std::future<T> future;

    public:
    AsyncBuffer(std::future<T> future) : future(std::move(future)) {}

    operator bool() const noexcept { return true; }
    bool Intact() const noexcept { return true; }
    bool IsReady() {
        auto result = future.wait_for(std::chrono::seconds(0));
        return (result == std::future_status::ready || result == std::future_status::deferred);
    }
};

#endif // ASYNCBUFFER
