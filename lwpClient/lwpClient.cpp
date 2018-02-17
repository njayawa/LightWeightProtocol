// lwpClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Thread.h"
#include "iostream"
#include "LWPClient.hpp"
#include "limits.h"

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

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _WIN32
	//this information is needed by winsock to start the 
	WSADATA     wsaData;
	WORD        wVersionRequested;

	wVersionRequested = MAKEWORD(1, 1);
	WSAStartup(wVersionRequested, &wsaData);
#endif
	MyClientCommunicator* cc = new MyClientCommunicator();
	bool bRtn = cc->connectServer("157.56.186.191", 7733);

	if(bRtn)
		printf("success\n");
	else
		printf("failure\n");

	MYDATA* pData = (MYDATA*) malloc(255 + sizeof(SYNCHRONIZABLEDATA));
	memset(pData, 0, 255 + sizeof(SYNCHRONIZABLEDATA)); 
	pData->sync_id = ntohl(0);
	strncpy((char*)&pData->begin, "Hello\n", 255);
	bRtn = cc->sendMessage(pData, 255);
	printf("Client printing messages (see if sendmessage works) : SENDMESSAGE FINISHED\n");
	pData->sync_id = 0;
	for(int i = 0; i < 255/*INT_MAX*/; i++)
	{
		sprintf((char*)&pData->begin, "Hello%d\n", i);
		bRtn = cc->postMessage(pData, 255);
	}



	if(bRtn)
		printf("success\n");
	else
		printf("failure\n");

	Thread::sleep(INFINITE);
}

