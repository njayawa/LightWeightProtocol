#ifndef __Lock__
#define __Lock__

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

class Lock
{
public:
  Lock(const char *name = NULL);
  ~Lock();
#ifdef _WIN32
  void LockIt(void);
  DWORD LockIt(DWORD milliseconds);
  void UnLockIt(void);
#else
  int  LockIt(void) { return pthread_mutex_lock(&theMutex);} 
  void LockIt(long milliseconds);
  int  UnLockIt(void) {return pthread_mutex_unlock(&theMutex);}
#endif
  
private:
#ifdef  _WIN32
  HANDLE hLock;
#else
  pthread_mutex_t theMutex;
#endif
};

#endif
