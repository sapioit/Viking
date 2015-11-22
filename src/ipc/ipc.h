#ifndef IPC_H
#define IPC_H
#include <unistd.h>
namespace IPC {
    bool IsParent = false;
    int PID() { return getpid(); }
}

#endif // IPC_H

