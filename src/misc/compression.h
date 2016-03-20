/*
Copyright (C) 2015 Voinea Constantin Vladimir

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
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
