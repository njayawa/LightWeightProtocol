#ifdef _WIN32
    #include <process.h>
#endif

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "LWP.h"
#include "safeint.hpp"
#include "Thread.h"

//TODO: remove



////////////////////////////////////////////////////////////////////////////////////////////////////
// Function:    LWP constructor 
//
// Purpose:     to initialize the contents of the LWP.
// 
// Parameters:  None
//
// Returns:     None
//
// Notes:       In windows, we must add special initialization code to start up winsock.
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//LWP::LWP()
//
//{
//#ifdef _WIN32
//    //this information is needed by winsock to start the 
//    WSADATA     wsaData;
//    WORD        wVersionRequested;
//
//    wVersionRequested = MAKEWORD(1, 1);
//    WSAStartup(wVersionRequested, &wsaData);
//#endif
//
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//// Function:    Destructor for LWP 
////
//// Purpose:     to deallocate any resources allocated by the LWP
//// 
//// Parameters:  .
////
//// Returns:     .
////
//// Notes:       .
//////////////////////////////////////////////////////////////////////////////////////////////////////
//LWP::~LWP()
//
//{
//}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Function:    setAsyncronousMode  
//
// Purpose:     This method sets the socket's mode of communication to asynchronous.  ie sets the
//                  the socket to non-blocking mode.
// 
// Parameters:  . s - this is the socket to which the "asynchronization" applies 
//
// Returns:     SOCKET_ERROR if successful, else non SOCKET_ERROR value
//
// Notes:       .
////////////////////////////////////////////////////////////////////////////////////////////////////
int AbstractCommunicator::setAsyncronousMode()

{
    int selectStatus;
    unsigned long arg = 1;              // non-blocking mode

    selectStatus = IOCTL(getSocket(), FIONBIO, &arg);
    return  selectStatus != SOCKET_ERROR;
}


AbstractCommunicator::AbstractCommunicator(const SOCKET& s)
{
    initObject();
    setSocket(s);
}

void AbstractCommunicator::initObject()
{
    _s = 0;
    //create mutex for this object
    _sendLock = NULL;
    char szTemp[63 + 1];
    itoa(SAFE_INT::getUniqueIdentifier(), szTemp, 10);
    _sendLock = new Lock(szTemp);
}

AbstractCommunicator::~AbstractCommunicator()
{
    //	delete _sendLock;
}


bool AbstractCommunicator::sendMessage(SYNCHRONIZABLEDATA* pData, unsigned nLength)
{
    bool bRtn = false;
    Synchronization s;
    pData->sync_id = s.getSynchronizationId();
    bRtn = postMessage(pData, nLength);
    s.waitForSynchronization();
    pData->sync_id = 0;
    return bRtn;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Function:    runBody  
//
// Purpose:     this method will be called to receive any and all incoming messages.  It will
//                  delegate all messages to either the control method (if the message is LWP
//                  related), or the processData method if it is an application related message.
// 
// Parameters:  s - the socket from which will receive data    
//
// Returns:     .
//
// Notes:       This is the main method through which we will route raw data to the "processData" 
//                  method which will be subclassed by derivatives of the LWP class
////////////////////////////////////////////////////////////////////////////////////////////////////

void AbstractCommunicator::runBody()

{
    int    selectStatus = 1;

    receiverStarted();

    try
    {
        while (selectStatus  &&  selectStatus != SOCKET_ERROR)
        {
            unsigned      iLen = 0;
            unsigned      n = 0;
            unsigned      host_len = 0;
            unsigned char buf[MAX_MSG_SIZE];
            fd_set        readFDS;

            FD_ZERO(&readFDS);
            FD_SET(getSocket(), &readFDS);
            //
            // Read the header, message header's length (network format)
            //
            while (n < sizeof(host_len)  &&  selectStatus  &&  selectStatus != SOCKET_ERROR)
            {
                selectStatus = select(getSocket() + 1, &readFDS, NULL, NULL, NULL);

                if (selectStatus != SOCKET_ERROR)
                {
                    selectStatus = recv(getSocket() + 1, (char *)&host_len + n, sizeof host_len - n, 0);
                    n += selectStatus;                 // (ok if selectStatus == -1)
                }
            }

            iLen = ntohl(host_len); // convert the length to host format

            if (!iLen  &&  iLen > MAX_MSG_SIZE)   // if illegal length
            {
                IllegalMSGLengthException e;
                throw e;
            }
            //
            // Read the message
            //
            n = 0;
            while (n < iLen  &&  selectStatus  &&  selectStatus != SOCKET_ERROR)
            {
                selectStatus = select(getSocket() + 1, &readFDS, NULL, NULL, NULL);
                if (selectStatus != SOCKET_ERROR)
                {
                    selectStatus = recv(getSocket(), (char *)buf + n, iLen-n, 0);
                    n += selectStatus;                 // (ok if selectStatus == -1)
                }
            }
            //
            // Send the data to the processData method
            //
            if (selectStatus  &&  selectStatus != SOCKET_ERROR)
            {
                SYNCHRONIZABLEDATA* pData = (SYNCHRONIZABLEDATA*)buf;
                preProcessData(pData, iLen); 
                processData(pData, iLen); // call receiver function
                postProcessData(pData, iLen); 
                Thread::sleep(5);

           }
        }
    }
    catch(const exception& e)
    {
        terminateConnection();
        receiverStopped();
        throw e;
    }
    terminateConnection();
    receiverStopped();
}



void AbstractCommunicator::terminateConnection()
{
    if(isRunning())
    {
        CLOSESOCKET(getSocket());
        setSocket(0);
    }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Function:    postMessage  
//
// Purpose:     This method sends a message to the socket
// 
// Parameters:  . id - the Socket to which a message is being sent by this object
//              . p - this is the socket to which the "asynchronization" applies 
//              . len - this is the length of the message being sent to the socket by this object. 
//              . pHeader - this is the Message header that is pre-pended to the message so that 
//                  the receiving client can identify the type of the header. 
//              . headerLen - this is the length of the message header which is passed along to the 
//                  socket by this object. 
//
// Returns:     SOCKET_ERROR if successful, else non SOCKET_ERROR value
//
// Notes:       Given a socket id, this method sends a message to the socket along with the message
//                  header. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool AbstractCommunicator::postMessage(SYNCHRONIZABLEDATA *p, unsigned len) 

{
    //translate the data first
    //
    translateData(p, len);
    fd_set sendfds;
    int selectStatus = SOCKET_ERROR;
    struct timeval timeout;
    unsigned host_n;

#ifndef _WIN32
    // Add ways to block signal (SIGRTMIN+2) that generated by
    // the timer function
    //
    sigset_t sigset_timer;
    sigemptyset(&sigset_timer);
    sigaddset(&sigset_timer, SIGRTMIN+2);
#endif

    if(!p) 
    {
        NULLMSGException e;
        throw e;
    }

    if(len > MAX_MSG_SIZE)
    {
        IllegalMSGLengthException e;
        throw e;
    }


    timeout.tv_sec = 0;
    timeout.tv_usec = SEND_TIMEOUT;


    FD_ZERO(&sendfds);
    FD_SET(getSocket(), &sendfds);

#ifndef _WIN32
    sigprocmask(SIG_BLOCK, &sigset_timer, NULL);
#endif
    getSendLock()->LockIt();

    selectStatus = select(0, NULL, &sendfds, NULL, &timeout);

    if (selectStatus == SELECT_STATUS_VALID)                     
    {                               

      host_n = htonl(len);
      selectStatus = send(getSocket(), (char *) &host_n, sizeof host_n, 0);

      if (selectStatus != SOCKET_ERROR)
      {
         selectStatus = send(getSocket(), (char *)p, (int) len, 0);
      }
    }
    getSendLock()->UnLockIt();
#ifndef _WIN32
    sigprocmask(SIG_UNBLOCK, &sigset_timer, NULL);
#endif

    return selectStatus != SOCKET_ERROR;
}



void AbstractCommunicator::preProcessData(SYNCHRONIZABLEDATA* pData, unsigned nLength)
{
    translateData(pData, nLength);
    if(pData->sync_id != 0 /*&& pData->originationId == getSocket()*/)
    {
        Synchronization::synchronize(pData);
    }
        
}

void AbstractCommunicator::postProcessData(SYNCHRONIZABLEDATA* pData, unsigned nLength)
{
    //free(pData);
}

void AbstractCommunicator::translateData(SYNCHRONIZABLEDATA* pData, unsigned nLength)
{
    pData->sync_id = htonl(pData->sync_id);
}


typedef std::map < SYNC_HANDLE, SynchronizationData* , std::less <SYNC_HANDLE> > SYNC_MAP;
static SYNC_MAP _theMap;
void Synchronization::synchronize(SYNCHRONIZABLEDATA* pData)
{
    SYNC_MAP::iterator i = _theMap.find(pData->sync_id);
    if(i != _theMap.end())
    {
        i->second->setMsgData(pData, false);
    }
    else
    {
        //assert(!"UNABLE TO FIND CORRECT SYNCHRONIZATION HANDLE");
    }
}

Synchronization::Synchronization()
{
	SynchronizationData* pData = new SynchronizationData();
	_theMap.insert(SYNC_MAP::value_type((SYNC_HANDLE) pData, pData)); 
	_handle = (SYNC_HANDLE) pData;

};

void Synchronization::waitForSynchronization(SYNCHRONIZABLEDATA* pData /*= NULL*/, int nLength/* = 0*/) const volatile
{
	assert(getSynchronizationId());
	SYNC_MAP::iterator i = _theMap.find(getSynchronizationId());
	assert(i != _theMap.end());
	while(!i->second->getMsgData())
		Thread::sleep(10);
	if(pData && nLength > 0)
	{
		void* pTemp = (void*) pData;
		memcpy(pData, i->second->getMsgData(), nLength);
	}
}

bool Synchronization::deleteSync()
{
	SYNC_MAP::iterator i = _theMap.find(getSynchronizationId());
	if(i != _theMap.end())
	{
		if(i->second)
			free(i->second);
		_theMap.erase(i);
		return true;
	}
	return false;
}

















