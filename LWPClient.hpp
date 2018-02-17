#ifndef __LWPClient__
#define __LWPClient__

#include "LWP.h"
#include "assert.h"
//
//class LWPClient : public LWP
//{
//public :
//    //
//    // Methods
//    //
//      LWPClient();
//      virtual ~LWPClient();
//      bool connectServer(const char *name);
//      void disconnectServer() {CLOSESOCKET(getSocket());};
//      bool connectionOpen() {return bConnected;};
//
//      bool sendToServer(const void *p, unsigned len);
//
//
//protected:
//      virtual void receiverStarted(SOCKET id) {lwpControl(id,LWP_START);};
//      virtual void receiverStopped(SOCKET id) {lwpControl(id,LWP_STOP); bConnected = false;};
//
//private:
//};

class DllExport ClientCommunicator : public AbstractCommunicator
{
public:
                            ClientCommunicator  ();
    virtual                 ~ClientCommunicator ();
    bool                    connectServer       (const char* pszName, unsigned nPortNumber);
    void                    disconnectServer    () {terminateConnection();};
    bool                    connectionOpen      () {return bConnected;};
protected:
    virtual void            processData         (SYNCHRONIZABLEDATA* pData, unsigned nLength) = 0; 
    virtual void            control             (unsigned code) = 0;
    virtual void            receiverStarted     (){control(LWP_START);};
    virtual void            receiverStopped     (){control(LWP_STOP); bConnected = false;};
private:
    bool                    connectTCP          (const char* pszHostName, const char* pszService);
    bool                    bConnected;
};



#define QLEN 5


class DllExport AbstractServerDaemon
{
};

class DllExport AbstractServerConnectionFactory
{
public:
    virtual AbstractCommunicator* createServerCommunicator(const SOCKET& s, AbstractServerDaemon* pComm) = 0;
};


class DllExport ServerCommunicator : public AbstractCommunicator
{
public:
                            ServerCommunicator(const SOCKET& s) { setSocket(s); }
    virtual                 ~ServerCommunicator(){};
protected:
    virtual void            processData(SYNCHRONIZABLEDATA* pData, unsigned nLength) = 0; 
    virtual void            control(unsigned id) = 0;
    virtual void            receiverStarted() { control(LWP_CONNECT);};
    virtual void            receiverStopped() { control(LWP_DISCONNECT);};
    //void                    setSocket(const SOCKET& s) { _sock = s; }
    //const SOCKET&           getSocket() const   {return _sock; }
private:
    //SOCKET                  _sock;
};



class DllExport ServerDaemon : public AbstractRunnable, public AbstractServerDaemon
{
public:
                            ServerDaemon(AbstractServerConnectionFactory* pFactory, bool bDeleteFactory = true)
                            {
                                assert(pFactory != NULL);
                                _s = 0;
                                _pFactory = pFactory;
                                _bDeleteFactory = bDeleteFactory;
                                _bRun = false;
                            }
    virtual                 ~ServerDaemon()
                            {
                                if(_bDeleteFactory)
                                    delete _pFactory;
                            }
    bool                    killClient(ServerCommunicator* pCom)
                            {
                                pCom->terminateConnection();
                                delete pCom;
								return true;
                            }
    void                    stopServer()
                            {
                                _bRun = false;
                            }
    bool                    openServer(const char *pszService);

protected:
    virtual void            runBody();
    virtual SOCKET&         getSocket() {return _s;}
    virtual void            setSocket(SOCKET s) { _s = s;}
    AbstractServerConnectionFactory* getFactory() const { return _pFactory;    }
private:
    AbstractServerConnectionFactory* _pFactory;
    bool                    _bDeleteFactory;
    SOCKET                  _s;
    bool                    _bRun;
};


#endif
