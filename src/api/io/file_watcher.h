#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <vector>
#include <memory>
#include <algorithm>

namespace IO {
template <class T> class FileWatcher {
protected:
  std::vector<std::shared_ptr<T>> watched_files_;

public:
  FileWatcher() = default;
  virtual ~FileWatcher() = default;

  template <typename... FileArgs> void Add(FileArgs &&... params) noexcept {
    watched_files_.emplace_back(std::forward<FileArgs>(params)...);
  }
  void Add(std::shared_ptr<T> file) noexcept { watched_files_.push_back(file); }

  template <class Predicate> void Remove(Predicate p) noexcept {
    std::remove_if(watched_files_.begin(), watched_files_.end(), p);
  }

  void Remove(const T &file) {
    for (auto it = watched_files_.cbegin(); it != watched_files_.end(); ++it) {
      if ((*(*it)) == file) {
        watched_files_.erase(it);
        break;
      }
    }
  }

  // template <class Callback>
  virtual void Run(std::function<void(std::vector<std::shared_ptr<T>>)>) = 0;
};
}

#endif