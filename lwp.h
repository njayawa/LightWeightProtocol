#ifndef __LWP__
#define __LWP__

#ifdef _WIN32

    #include <windows.h>
    #include <winsock.h>
	#define DllExport   __declspec( dllexport )

#else

	#define DllExport

    #include <sys/types.h>

    #ifdef _SOLARIS_

        #include <sys/filio.h>
        #include <sys/socket.h>

    #else

        #include <``.h>
    #endif

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

#include "root.h"
#include "Lock.hpp"
#include "Thread.h"
#include <exception>
#include <map>
#include "assert.h"

using namespace std;


#ifdef _WIN32

    #define SEND_TIMEOUT                    500 * 1000             // send timeout     
    #define SELECT_STATUS_VALID             1
    #define CLOSESOCKET(a) closesocket(a)
    #define IOCTL(a, b, c) ioctlsocket(a, b, c) 
   


#else

    #define SEND_TIMEOUT                    500             // send timeout
    #define SELECT_STATUS_VALID             0
    #define CLOSESOCKET(a) close(a) 
    #define IOCTL(a, b, c) ioctl(a, b, c) 

    #define INVALID_SOCKET                  -1
    #define SOCKET_ERROR                    -1
    typedef int      SOCKET;


#endif


#define MAX_OPEN_SOCKETS                100              // maximum open sockets
//Maximum msg size
#define MAX_MSG_SIZE                    65534            // maximum message size
#define MAX_MUTEX_WAIT_TIME             10000           // mutex wait max




//Forward declarations
//
class IllegalMSGLengthException;
class LWP;
class NULLMSGException;
class Synchronization;
class SynchronizationData;
class MSG_DATA;


class DllExport IllegalMSGLengthException : public exception
{
public:
    virtual const char* what() const throw() { return "Illegal message length received";    }
};


class DllExport NULLMSGException : public exception
{
public:
    virtual const char* what() const throw() { return "Message received is NULL";    }
};

typedef DllExport long SYNC_HANDLE;


static const char *Status[] ={ "None", "LWP_INIT", "LWP_START", "LWP_CONNECT", "LWP_DISCONNECT", "PTP_STOP", "PTP_SHUTDOWN", "PTP_MISSEDHB"};



typedef DllExport struct tagSYNCHRONIZABLEDATA
{
    tagSYNCHRONIZABLEDATA() { memset(this, 0, sizeof(SYNCHRONIZABLEDATA)); }
    SYNC_HANDLE sync_id;
} SYNCHRONIZABLEDATA;

////////////////////////////////////////////////////////////////////////////////////////////////////
//OBJECT:   LWP
//
//PURPOSE:  This object is the base class for all lightweight protocol objects.  It defines standard
//            behavior such as message receiving and control messages.  
//
//NOTES:    None
////////////////////////////////////////////////////////////////////////////////////////////////////

//
//class LWP : public Root
//{
//public:
//
//                    LWP();
//    virtual         ~LWP();
//
//protected:
//
//private:
//
//};



class DllExport Communicator
{
public:
    virtual                 ~Communicator() {};
    virtual bool            sendMessage(SYNCHRONIZABLEDATA* pData, unsigned nLength) = 0;
    virtual bool            postMessage(SYNCHRONIZABLEDATA* pData, unsigned nLength) = 0;
    virtual int				setAsyncronousMode() = 0;
    virtual void            terminateConnection() = 0;
protected:
    //virtual void            setDataSynchronization(void* pData, SYNC_HANDLE h) = 0;
    virtual void            processData(SYNCHRONIZABLEDATA* pData, unsigned nLength) = 0; 
    virtual void            preProcessData(SYNCHRONIZABLEDATA* pData, unsigned nLength) = 0 ; 
    virtual void            postProcessData(SYNCHRONIZABLEDATA* pData, unsigned nLength) = 0; 
    virtual void            translateData(SYNCHRONIZABLEDATA* pData, unsigned nLength) = 0;
    virtual void            control(unsigned id) = 0;
    virtual void            receiverStarted() = 0;
    virtual void            receiverStopped() = 0;
    virtual SOCKET&         getSocket() = 0;
    virtual void            setSocket(SOCKET s) = 0;
    virtual Lock*           getSendLock() = 0;

};

class DllExport AbstractCommunicator : public Communicator, public AbstractRunnable
{
public:
    //The following are LWP control messages
    enum {LWP_INIT=1, LWP_START, LWP_CONNECT, LWP_DISCONNECT, LWP_STOP, LWP_SHUTDOWN, LWP_MISSEDHB};

                            AbstractCommunicator() {initObject();};
                            AbstractCommunicator(const SOCKET& s);
    virtual                 ~AbstractCommunicator();
    virtual bool            sendMessage(SYNCHRONIZABLEDATA* pData, unsigned nLength);
    virtual bool            postMessage(SYNCHRONIZABLEDATA* pData, unsigned nLength);
    void                    terminateConnection();
protected:
    virtual void            processData(SYNCHRONIZABLEDATA* pData, unsigned nLength) = 0; 
    virtual void            preProcessData(SYNCHRONIZABLEDATA* pData, unsigned nLength); 
    virtual void            postProcessData(SYNCHRONIZABLEDATA* pData, unsigned nLength); 
    virtual void            translateData(SYNCHRONIZABLEDATA* pData, unsigned nLength);
    virtual void            control(unsigned id) = 0;
    virtual void            receiverStarted() = 0;
    virtual void            receiverStopped() = 0;
    virtual SOCKET&         getSocket() {return _s;}
    virtual void            setSocket(SOCKET s) {_s = s;}
    virtual Lock*           getSendLock() {return _sendLock;}
    virtual void            setSendLock(Lock* l) {_sendLock = l; } 
    virtual void            runBody();
    virtual int				setAsyncronousMode();
private:
    void                    initObject();
    SOCKET                  _s;
    Lock*                   _sendLock;
};


class DllExport MSG_DATA
{
};

class DllExport SynchronizationData
{
public:
                            SynchronizationData() {_pData = NULL, _bDelete = false;};
    virtual                 ~SynchronizationData() {if(_bDelete) free(_pData);}
    void                    setMsgData(SYNCHRONIZABLEDATA* pData, bool bDelete = true) {_pData = pData; _bDelete = bDelete;};
    SYNCHRONIZABLEDATA*     getMsgData() const volatile {return _pData;} 
private:
    mutable SYNCHRONIZABLEDATA*       _pData;
    bool                    _bDelete;    
};





class DllExport Synchronization
{
public:
                            Synchronization();
    SYNC_HANDLE             getSynchronizationId() const volatile { return _handle; } 

    virtual                 ~Synchronization()
                                {
                                    deleteSync();
                                };

    void                    waitForSynchronization(SYNCHRONIZABLEDATA* pData = NULL, int nLength = 0) const volatile;
    static void             synchronize(SYNCHRONIZABLEDATA* pData);

protected:
    bool                    deleteSync();
    mutable volatile SYNC_HANDLE         _handle;
private:
};

//class Synchronizer
//{
//public:
//    static Synchronizer*    getInstance() 
//                                {
//                                    if(!_synch)
//                                        _synch = new Synchronizer();
//                                    return _synch;
//                                }
//    SYNC_HANDLE             createSync()
//                                {
//                                }
//    bool                    setSyncHandle(SYNC_HANDLE s, MSG_DATA* pData)
//                            {
//                                    _theMap::iterator i = map.find(s);
//                                    if(i != map.end)
//                                    {
//                                        i->setMsgData(pData);
//                                        return true;
//                                    }
//                                    return false;
//                            }
//
//    void                    waitForSynchronization() const volatile;
//    virtual                 ~Synchronizer(){};
//
//protected:
//
//    Synchronizer();
//private:
//
//
//    static Synchronizer*    _synch;
//                  
//}; 




#endif
