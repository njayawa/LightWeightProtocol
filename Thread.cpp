
#include "Thread.h"


////////////////////////////////////////////////////////////
// Structure to send to ThreadFunction
////////////////////////////////////////////////////////////

//typedef struct tagThreadStruct
//{
//   Runnable* runnable;
//   bool shouldDelete;
//} ThreadStruct;















////////////////////////////////////////////////////////////
// class Thread
////////////////////////////////////////////////////////////

// Constructor
Thread::Thread(Runnable* runnable) :
_pRunnable(runnable)

{
}


// Destructor
Thread::~Thread()
{
}

void Thread::start()
{
    Thread::createThread(threadFunction, _pRunnable);
}

void Thread::sleep(int nMilliSeconds)
{
    SLEEP(nMilliSeconds);
}


bool Thread::waitForRunnable(Runnable* pRunnable, int nMilliSeconds /*= INFINITE_WAIT*/)
{
    if(!pRunnable->isRunning())
        return true;
    
    if(nMilliSeconds != INFINITE_WAIT)
    {
        Thread::sleep(nMilliSeconds);
        return !pRunnable->isRunning();
    }

    do
    {
        Thread::sleep(10); //let the CPU run other threads    
    }
    while(pRunnable->isRunning());
    return true;

}

bool Thread::waitForRunnables(int iSize, Runnable* pRunnables[], int nMilliSeconds /*= INFINITE_WAIT*/)
{
    bool bRtn = true;
    for(int i = 0; i < iSize; i++)
    {
        bRtn = bRtn && Thread::waitForRunnable(pRunnables[i], nMilliSeconds);
    }
    return bRtn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function:    createThread  
//
// Purpose:     This method provides a platform-agnostic way of starting a thread
// 
// Parameters:  . start - this is the function pointer which is used by the begin_thread method 
//                  to route all messages to this function.
//              . arglist - a pointer to the list of arguments passed to the function denoted by
//                  the "start" parameter
//
// Returns:     the thread id of the thread that is created
//
// Notes:       . 
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
unsigned Thread::createThread( void (*start)(void*), void *arglst)
#else
unsigned Thread::createThread( void *(*start)(void*), void *arglst)
#endif
{
#ifdef _WIN32
   return _beginthread( start, 0, arglst );
#else
   pthread_t tid;
   int th;
#ifdef _SOLARIS_
   th = pthread_create(&tid, NULL, start, arglst);
#else
   pthread_attr_t pattr;         
   pthread_attr_create(&pattr);  
   th = pthread_create(&tid, pattr, start, arglst);   
   th = pthread_detach(&tid); file:
#endif

   if (th == 0) th = 1;
   return (unsigned) th;
#endif
}

////////////////////////////////////////////////////////////
// Thread Function to be passed to CreateThread
////////////////////////////////////////////////////////////
#ifdef _WIN32
void Thread::threadFunction(void* pVoid)
#else
void Thread::*threadFunction(void* pVoid)
#endif
{
   Runnable* pRunnable = (Runnable*)pVoid;
   try
   {
        pRunnable->run();
   }
   catch(...)
   {
      throw;
   }
}



void AbstractRunnable::run()
{
    try
    {
        _bIsRunning = true;
        runBody();
    }
    catch(...)
    {
        _bIsRunning = false;
#ifndef _WIN32
        pthread_exit(NULL);
#endif
        throw;
    }
    _bIsRunning = false;
#ifndef _WIN32
        pthread_exit(NULL);
#endif
}


