#include <http/cachemanager.h>
#include <http/components.h>
#include <io/filesystem.h>
#include <misc/storage.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace Http::Components;

std::map<std::string, Resource> CacheManager::_resources;
std::mutex CacheManager::_cacheLock;

Resource CacheManager::GetItem(const std::string &path) {
        std::lock_guard<std::mutex> lock(_cacheLock);
        auto item = CacheManager::_resources.find(path);
        if (item != CacheManager::_resources.end())
                return item->second;

        return {};
}

void CacheManager::PutItem(const std::pair<std::string, Resource> &&item) {
        std::lock_guard<std::mutex> lock(_cacheLock);
        CacheManager::_resources.insert(item);
}

void CacheManager::ReplaceItem(const std::string &path, const Resource &res) {
        std::lock_guard<std::mutex> lock(_cacheLock);
        _resources[path] = res;
}
Resource CacheManager::GetResource(const std::string &path) {
        std::string fpath(Storage::settings().root_path + path);

        auto item = CacheManager::GetItem(fpath);

        if (item) {
                // The item is in cache
                struct stat st;
                auto st_res = stat(fpath.c_str(), &st);
                if (st_res != 0) {
                        // The item has been removed from the disk but it is
                        // still in cache, it
                        // should be removed
                        throw StatusCode::NotFound;
                } else {
                        // The item has been updated on disk
                        if (st.st_mtime > item.stat().st_mtime) {
                                try {
                                        Resource res(fpath);
                                        CacheManager::ReplaceItem(fpath, res);
                                        return res;
                                } catch (IO::fs_error &ex) {
                                        throw StatusCode::NotFound;
                                } catch (std::system_error &ex) {
                                        throw StatusCode::InternalServerError;
                                }
                        } else
                                return item;
                }
        } else {
                // Fetch the item from disk and cache it
                // TODO make some decision on whether or not it should be cached
                try {
                        Resource res(fpath);
                        CacheManager::PutItem(std::make_pair(fpath, res));
                        return res;
                } catch (IO::fs_error &ex) {
                        throw StatusCode::NotFound;
                } catch (std::system_error &ex) {
                        throw StatusCode::InternalServerError;
                }
        }
}
