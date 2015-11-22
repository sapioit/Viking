#include <ipc/fork.h>
#include <ipc/ipc.h>

void IPC::Fork()
{
    auto pid = fork();
    if(pid == 0)
        IPC::IsParent = false;
    if(pid > 0)
        IPC::IsParent = true;
}
