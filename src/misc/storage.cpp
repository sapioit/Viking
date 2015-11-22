#include <misc/storage.h>
#include <http/cachemanager.h>
#include <io/filesystem.h>

#include <system_error>
#include <utility>
#include <thread>

Settings Storage::settings_;

const Settings &Storage::GetSettings() { return Storage::settings_; }

void Storage::SetSettings(const Settings &settings) { Storage::settings_ = settings; }
