#ifndef STORAGE_H
#define STORAGE_H

#include "settings.h"

class Storage {
    static Settings settings_;

    public:
    static const Settings &GetSettings();
    static void SetSettings(const Settings &);
};

#endif // STORAGE_H
