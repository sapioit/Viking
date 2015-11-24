#ifndef MEMBUFFER_H
#define MEMBUFFER_H

#include <io/buffers/datasource.h>

#include <vector>

struct MemoryBuffer : public DataSource {
    std::vector<char> data;
    std::size_t initial_size;

    MemoryBuffer(const std::vector<char> &data) : data(data), initial_size(data.size()) {}
    virtual operator bool() const noexcept { return data.size() != 0; }
    virtual bool Intact() const noexcept { return data.size() == initial_size; }
};

#endif // MEMBUFFER_H
