#include <http/cachemanager.h>
#include <http/components.h>
#include <io/filesystem.h>
#include <misc/storage.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

std::map<std::string, Resource> CacheManager::_resources;

Resource CacheManager::GetItem(const std::string &path) {
    auto item = CacheManager::_resources.find(path);
    if (item != CacheManager::_resources.end())
        return item->second;

    return {};
}

void CacheManager::PutItem(const std::pair<std::string, Resource> &&item) { CacheManager::_resources.insert(item); }

void CacheManager::ReplaceItem(const std::string &path, const Resource &res) { _resources[path] = res; }

Resource CacheManager::GetResource(const std::string &path) {
    std::string fpath(Storage::GetSettings().root_path + path);

    auto item = CacheManager::GetItem(fpath);

    if (item) {
        struct stat st;
        auto st_res = stat(fpath.c_str(), &st);
        if (st_res != 0) {
            // The item has been removed from the disk but it is
            // still in cache, it
            // should be removed
            throw Http::StatusCode::NotFound;
        } else {
            if (st.st_mtime > item.stat().st_mtime) {
                // The item has been updated on disk
                try {
                    Resource res(fpath);
                    CacheManager::ReplaceItem(fpath, res);
                    return res;
                } catch (IO::fs_error &ex) {
                    throw Http::StatusCode::NotFound;
                } catch (std::system_error &ex) {
                    throw Http::StatusCode::InternalServerError;
                }
            } else {
                return item;
            }
        }
    } else {
        // Fetch the item from disk and cache it
        // TODO make some decision on whether or not it should be cached
        try {
            Resource res(fpath);
            CacheManager::PutItem(std::make_pair(fpath, res));
            return res;
        } catch (IO::fs_error &ex) {
            throw Http::StatusCode::NotFound;
        } catch (std::system_error &ex) {
            throw Http::StatusCode::InternalServerError;
        }
    }
}
