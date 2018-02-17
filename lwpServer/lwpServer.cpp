// lwpServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Thread.h"
#include "iostream"
#include "LWPClient.hpp"
#include "limits.h"
#include <map>


typedef struct tagMYDATA : public SYNCHRONIZABLEDATA
{
	char begin;
}MYDATA;

class MyClientCommunicator: public ClientCommunicator
{
public:
	virtual void            processData         (SYNCHRONIZABLEDATA* pData, unsigned nLength) 
	{
		MYDATA* m = (MYDATA*) (pData);  
		char tmp[1024];
		strncpy(tmp, &m->begin, nLength);
		printf("Client got Message:  %s\n", tmp);
		printf(tmp);
	}; 
	virtual void            control             (unsigned code) {};
};


class MyServerCommunicator: public ServerCommunicator
{
public:
	MyServerCommunicator(const SOCKET& s) : ServerCommunicator(s){};
	virtual void            processData         (SYNCHRONIZABLEDATA* pData, unsigned nLength)
	{
		MYDATA* m = (MYDATA*) (pData);  
		char tmp[1024];
		strncpy(tmp, &m->begin, nLength);
		printf("SERVER got Message:  %s\n", tmp);
		printf("Sending response back to client...\n");
		postMessage(pData, nLength);
		printf("Server printing messages (see if sendmessage works\n");
	};
	virtual void            control             (unsigned code) {};
};

class MyServerConnectionFactory: public AbstractServerConnectionFactory
{
public:
	virtual AbstractCommunicator* createServerCommunicator(const SOCKET& s, AbstractServerDaemon* pComm)
	{
		return new MyServerCommunicator(s);
	}
};



int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _WIN32
	//this information is needed by winsock to start the 
	WSADATA     wsaData;
	WORD        wVersionRequested;

	wVersionRequested = MAKEWORD(1, 1);
	WSAStartup(wVersionRequested, &wsaData);
#endif

	//Runnable* aRunnables[2];
	//aRunnables[0] = new MyRunnable(100, "A");
	//aRunnables[1] = new MyRunnable(200, "B");
	//Thread t1(aRunnables[0]);
	//Thread t2(aRunnables[1]);
	//t1.start();
	//t2.start();
	//Thread::waitForRunnables(2, aRunnables);
	MyServerConnectionFactory* pFac = new MyServerConnectionFactory();
	ServerDaemon sd(pFac);
	sd.openServer("7733");
	Thread t1(&sd);
	t1.start();
// 	Thread::sleep(1000);
// 	MyClientCommunicator* cc = new MyClientCommunicator();
// 	bool bRtn = cc->connectServer("localhost", 7733);
// 
// 	if(bRtn)
// 		printf("success\n");
// 	else
// 		printf("failure\n");
// 
// 	MYDATA* pData = (MYDATA*) malloc(255 + sizeof(SYNCHRONIZABLEDATA));
// 	memset(pData, 0, 255 + sizeof(SYNCHRONIZABLEDATA)); 
// 	strncpy((char*)&pData->begin, "Hello\n", 255);
// 	bRtn = cc->sendMessage(pData, 255);
// 	printf("Client printing messages (see if sendmessage works) : SENDMESSAGE FINISHED\n");
// 	pData->sync_id = 0;
// 	for(int i = 0; i < 255/*INT_MAX*/; i++)
// 	{
// 		sprintf((char*)&pData->begin, "Hello%d\n", i);
// 		bRtn = cc->postMessage(pData, 255);
// 	}
// 
// 
// 
// 	if(bRtn)
// 		printf("success\n");
// 	else
// 		printf("failure\n");

	Thread::waitForRunnable(&sd);
	return 0;
}

