#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <set>
#include <memory>
#include <algorithm>
#include <misc/debug.h>

namespace IO {
template <class T> class FileWatcher {
      protected:
        std::set<T> watched_files_;

      public:
        struct FileNotFound {
                int fd;
                FileNotFound(int file_descriptor) : fd(file_descriptor) {}
        };
        FileWatcher() = default;
        virtual ~FileWatcher() = default;

        void Add(T file) noexcept { watched_files_.emplace(std::move(file)); }

        void Remove(const T &file) {
                auto elements_removed = watched_files_.erase(file);
                if (elements_removed == 0)
                        debug(
                            "FileWatcher could not remove the file with fd = " +
                            std::to_string(file.GetFD()));
        }

        // virtual void Run(std::function<bool(const T &)>) noexcept = 0;
};
}

#endif
