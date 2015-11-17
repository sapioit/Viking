#ifndef UTILS_H
#define UTILS_H

#include <memory>
#include <io/buffers/mem_buffer.h>
#include <io/buffers/unix_file.h>

std::unique_ptr<MemoryBuffer> From(const UnixFile &);

#endif // UTILS_H
