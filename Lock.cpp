#include "Lock.hpp"

#ifdef _WIN32
Lock::Lock(const char *name)
{
    hLock = CreateMutex(0, 0, name);
}

Lock::~Lock()
{
    CloseHandle(hLock);
}

void Lock::LockIt()
{
    WaitForSingleObject(hLock, INFINITE);
}

DWORD Lock::LockIt(DWORD milliseconds)
{
    return WaitForSingleObject(hLock, milliseconds);
}

void Lock::UnLockIt()
{
    ReleaseMutex(hLock);
}

#else
Lock::Lock(const char *name)
{
    pthread_mutex_init(&theMutex, NULL); 
}

Lock::~Lock()
{
}
#endif
