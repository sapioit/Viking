#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <vector>
#include <memory>

namespace IO {
template <class T> class FileWatcher {
  std::vector<std::shared_ptr<T>> watched_files_;

  FileWatcher() = default;
  virtual ~FileWatcher() = default;

  template <typename... FileArgs>
  virtual void Add(FileArgs &&... params) noexcept {
    watched_files_.emplace_back(std::forward<FileArgs>(params)...);
  }

  template <class Predicate> virtual void Remove(Predicate p) noexcept {
    std::remove_if(watched_files_.begin(), watched_files_.end(), p);
  }

  template <class Callback> virtual void Run(Callback) = 0;
};
}

#endif