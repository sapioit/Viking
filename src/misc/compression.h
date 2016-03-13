#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <vector>
#include <zlib.h>

namespace compression {
template <typename T> T deflate(const T &data) {
    T deflated;
    deflated.resize(compressBound(data.size()));

    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;

    defstream.avail_in = (uInt)data.size();
    defstream.next_in = (Bytef *)&data.front();
    defstream.avail_out = (uInt)deflated.size();
    defstream.next_out = (Bytef *)&deflated.front();

    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);
    deflated.resize(defstream.total_out);

    return deflated;
}

template <typename T> T gzip(const T &data) {
    T gzipped;
    gzipped.resize(compressBound(data.size()));

    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;

    defstream.avail_in = (uInt)data.size();
    defstream.next_in = (Bytef *)&data.front();
    defstream.avail_out = (uInt)gzipped.size();
    defstream.next_out = (Bytef *)&gzipped.front();

    constexpr auto windowsBits = 15;
    constexpr auto GZIP_ENCODING = 16;
    deflateInit2(&defstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowsBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);
    gzipped.resize(defstream.total_out);

    return gzipped;
}
}

#endif // COMPRESSION_H
