#ifndef MEMBUFFER_H
#define MEMBUFFER_H

#include <io/buffers/datasource.h>

#include <vector>

struct MemoryBuffer : public DataSource {
    std::vector<char> data;

    MemoryBuffer() = default;
    MemoryBuffer(const std::vector<char> &data) : data(data) {}
    virtual operator bool() const noexcept { return data.size() != 0; }
};

#endif // MEMBUFFER_H
