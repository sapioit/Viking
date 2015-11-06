#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <vector>
#include <memory>
#include <algorithm>
#include <misc/debug.h>

namespace IO {
template <class T> class FileWatcher {
      protected:
        std::vector<T> watched_files_;

      public:
        struct FileNotFound {
                int fd;
                FileNotFound(int file_descriptor) : fd(file_descriptor) {}
        };
        FileWatcher() = default;
        virtual ~FileWatcher() = default;

        T &Add(T file) noexcept {
                watched_files_.emplace_back(std::move(file));
                return watched_files_.front();
        }

        void Remove(const T &file) {
                for (auto it = watched_files_.cbegin();
                     it != watched_files_.end(); ++it) {
                        if ((*it) == file) {
                                watched_files_.erase(it);
                                return;
                        }
                }
                debug("FileWatcher could not remove the file with fd = " +
                      std::to_string(file.get_fd()));
                // throw FileNotFound {file.get_fd()};
        }

        virtual void
        Run(std::function<void(std::vector<std::reference_wrapper<T>>)>) = 0;
};
}

#endif