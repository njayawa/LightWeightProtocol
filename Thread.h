#ifndef _Thread_
#define _Thread_



#define INFINITE_WAIT 0

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    
    #define SLEEP(n) ::Sleep(n)
	#define DllExport   __declspec( dllexport )

#else
    #include <tk.h>
	#define DllExport

    #define SLEEP(n) Tk_Sleep(n)
#endif

class DllExport	Runnable
{
public:
    virtual void run() = 0;
    virtual bool isRunning() = 0;
};


class DllExport	AbstractRunnable : public Runnable
{
public:
    AbstractRunnable() : _bIsRunning(true) {}; //TODO: this is a hack.  isrunning should be false
    virtual ~AbstractRunnable(){};
    virtual void run();
    virtual bool isRunning() {return _bIsRunning;}
protected:
    virtual void runBody() = 0;
private:
    bool    _bIsRunning;

};

class DllExport	Thread
{
public:
    Thread(Runnable* runnable);
    virtual ~Thread();
    void start();
    static void sleep(int nMilliSeconds);
    static bool waitForRunnable(Runnable* pRunnable, int nMilliSeconds = INFINITE_WAIT);
    static bool waitForRunnables(int iSize, Runnable* pRunnables[], int nMilliSeconds = INFINITE_WAIT);
protected:

#ifdef _WIN32
    unsigned createThread(void (*start)(void*), void *arglst);
    static void threadFunction(void *pVoid);

#else
    unsigned createThread(void *(*start)(void*), void *arglst);
    static void *threadFunction(void *pVoid);
#endif
private:
    Runnable* _pRunnable;
};

#endif // _Thread_