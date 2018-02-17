
class LWP 
{
public:

                    LWP();
    virtual         ~LWP();
    void            receive(SOCKET s) throw (exception, IllegalMSGLengthException);
    SOCKET&         getSocket() {return _s;}
    void            setSocket(SOCKET s) { _s = s;}
    Lock*           getSendLock() {return _sendLock;}
    void            setSendLock(Lock* l) {_sendLock = l; } 

protected:
    static int      setAsyncronousMode(SOCKET s);
    bool            sendMessage (SOCKET id, const void *x, unsigned n, const void *hdr, unsigned hdrn) throw (exception, IllegalMSGLengthException, NULLMSGException);

    //LWP Overridables
    //
    virtual void    processData(unsigned, unsigned char *, unsigned) = 0;
    virtual void    lwpControl(SOCKET id, unsigned code) = 0;
    virtual void    receiverStarted(SOCKET id) = 0;
    virtual void    receiverStopped(SOCKET id) = 0;


private:
    Lock *_sendLock;
    mutable SOCKET _s;
};





class LWPClient : public LWP
{
public :
                    LWPClient();
      virtual       ~LWPClient();
      bool          connectServer(const char *name);
      void          disconnectServer() {CLOSESOCKET(getSocket());};
      bool          connectionOpen() {return bConnected;};

      bool          sendToServer(const void *p, unsigned len);


protected:
      virtual void  receiverStarted(SOCKET id) {lwpControl(id,LWP_START);};
      virtual void  receiverStopped(SOCKET id) {lwpControl(id,LWP_STOP); bConnected = false;};

private:
      bool          connectTCP(const char *host, const char *service);
      bool          bConnected;
};




class LWPServer :  public LWP
{
public :
                    LWPServer();
  virtual           ~LWPServer();
  bool              openServer(const char *service);
  void              acceptLoop();
  bool              sendToClient(SOCKET id, const void *p, unsigned len);
  bool              killClient(SOCKET s);

protected:
  virtual void      receiverStarted(SOCKET id) {lwpControl(id,LWP_CONNECT);};
  virtual void      receiverStopped(SOCKET id) {lwpControl(id,LWP_DISCONNECT);};
};
