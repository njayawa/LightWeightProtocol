#ifdef _WIN32
#include <process.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined( _WIN32 ) || defined( sun ) || defined( linux )
#include <iostream>
#else
#include <iostream.h>
#endif

#include "LWPClient.hpp"
#include "Thread.h"

#define HOST_NAME_BUFFER_SIZE 100




//*************************************************************************
//
// Constructor
//
//*************************************************************************
ClientCommunicator::ClientCommunicator() 
{
	bConnected = false;
}

//*************************************************************************
//
// Destructor
//
//*************************************************************************
ClientCommunicator::~ClientCommunicator()

{
}



//*************************************************************************
//
// Function : connectToServer
//
//*************************************************************************
bool ClientCommunicator::connectServer(const char *pszName, unsigned nPortNum)

{
	char             szPort[63 + 1];
	char             szHost[HOST_NAME_BUFFER_SIZE];

	setSocket(INVALID_SOCKET);
	//TODO: throw exception
	//
	if (!pszName || strlen(pszName) >= sizeof szHost)  return false;

	strncpy(szHost, pszName, HOST_NAME_BUFFER_SIZE);               // separate szHost pszName and port #

	itoa(nPortNum, szPort, 10);

	if (connectTCP(szHost, szPort))            // if socket opened ok
	{
		setAsyncronousMode();          // disable socket blocking

		Thread t(this);
		t.start();
		bConnected = true;
		return true;
	}

	return false;
}

//*************************************************************************
//
// Function : connectTCP
//
//*************************************************************************
bool ClientCommunicator::connectTCP(const char *pszHost, const char *pszService)
{
	struct hostent      *phe;           // pszHost info
	struct servent      *pse;           // Service info
	struct sockaddr_in   sin;           // internet endpoint address
	int                  type;          // socket type
#ifdef _WIN32
	struct protoent     *ppe;           // protocol info
#endif

	memset(&sin, 0, sizeof sin);
	sin.sin_family = AF_INET;
	//
	// map Service name to port number
	//
	if (pse = getservbyname(pszService, "tcp"))
	{
		sin.sin_port = pse->s_port;
	}
	else if ((sin.sin_port = htons((u_short)atoi(pszService))) == 0)
	{
		return false;
	}
	//
	// map host name to ip address
	//
	if (phe = gethostbyname(pszHost))
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	else if ((sin.sin_addr.s_addr = inet_addr(pszHost)) == INADDR_NONE)
	{
		return false;
	}
#ifdef _WIN32
	//
	// map protocol name to protocol number
	//
	if ((ppe = getprotobyname("tcp")) == 0)
	{
		return false;
	}
#endif

	type = SOCK_STREAM;

	//
	// allocate a socket
	//
#ifdef _WIN32
	setSocket(socket(PF_INET, type, ppe->p_proto));
#else
	setSocket(socket(PF_INET, type, 0));
#endif

	if (getSocket() == INVALID_SOCKET)
	{
		return false;
	}

#ifndef _WIN32
	unsigned long ulSocketBufferSize = 208000;

	if (setsockopt(getSocket(), SOL_SOCKET, SO_SNDBUF,
		&ulSocketBufferSize, sizeof(ulSocketBufferSize)) < 0) 
	{
		//TODO: report error
	}
	else 
	{
		unsigned long ulBufferSize;
		int i = sizeof(ulBufferSize);
		getsockopt(getSocket(), SOL_SOCKET, SO_SNDBUF, (void*)&ulBufferSize, &i);
	}


	if (setsockopt(getSocket(), SOL_SOCKET, SO_RCVBUF,
		&ulSocketBufferSize, sizeof(ulSocketBufferSize)) < 0) 
	{
		//TODO: throw an exception or log error
	}
	else 
	{
		unsigned long ulBufferSize;
		int i = sizeof(ulBufferSize);
		getsockopt(getSocket(), SOL_SOCKET, SO_RCVBUF, (void*)&ulBufferSize, &i);
	}
#endif

	//
	// connect socket
	//
	if (connect(getSocket(), (struct sockaddr *) &sin, sizeof sin) == SOCKET_ERROR)
	{
		//TODO: report error
		terminateConnection();
		setSocket(INVALID_SOCKET);
		return false;
	}

	return true;
}









//*************************************************************************
//
// Function : openServer
//
//*************************************************************************
bool ServerDaemon::openServer(const char *pszService)

{
	struct servent     *pse;            // Service info
	struct protoent    *ppe;            // protocol info
	struct sockaddr_in  sin;            // internet endpoint address
	char   protocol[] = "tcp";
	int                 type;

	memset( &sin, 0, sizeof sin );
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	//
	// map Service name to port number
	//
	if (pse = getservbyname(pszService, protocol))
		sin.sin_port = pse->s_port;
	else if ((sin.sin_port = htons((u_short)atoi(pszService))) == 0)
	{
		return false;
	}
	//
	// map protocol name to protocol number
	//
	if ((ppe = getprotobyname(protocol)) == 0)
	{
		return false;
	}
	//
	// use protocol to chose a socket type
	//
	if (strcmp( protocol, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;
	//
	// allocate a socket
	//
	setSocket(socket(PF_INET, type, ppe->p_proto));

	if (getSocket() == INVALID_SOCKET)
	{
		//TODO throw an error
		assert(!"can't create socket\n");
		return false;
	}
	//
	// bind socket
	//
	if (bind(getSocket(), (struct sockaddr *) &sin, sizeof sin) == SOCKET_ERROR)
	{
		//TODO throw an error
		assert(!"can't bind\n");
		CLOSESOCKET(getSocket());
		setSocket(INVALID_SOCKET);
		return false;
	}

#ifndef _WIN32
	unsigned long ulSocketBufferSize = 208000;

	if (setsockopt(getSocket(), SOL_SOCKET, SO_SNDBUF,
		&ulSocketBufferSize, sizeof(ulSocketBufferSize)) < 0) {
			perror("Can't set SO_SNDBUF option on socket: %m");
			exit(1);
	}
	else {
		unsigned long ulBufferSize;
		int i = sizeof(ulBufferSize);
		getsockopt(getSocket(), SOL_SOCKET, SO_SNDBUF, (void*)&ulBufferSize, &i);
		//printf("SO_SNDBUF is  %lu\n", ulBufferSize);
	}


	if (setsockopt(getSocket(), SOL_SOCKET, SO_RCVBUF,
		&ulSocketBufferSize, sizeof(ulSocketBufferSize)) < 0) {
			perror("Can't set SO_SNDBUF option on socket: %m");
			exit(1);
	}
	else {
		unsigned long ulBufferSize;
		int i = sizeof(ulBufferSize);
		getsockopt(getSocket(), SOL_SOCKET, SO_RCVBUF, (void*)&ulBufferSize, &i);
		//printf("SO_RCVBUF is  %lu\n", ulBufferSize);
	}
#endif


	if (type == SOCK_STREAM  &&  listen(getSocket(), QLEN) == SOCKET_ERROR)
	{
		//TODO: throw an error
		assert(!"can't listen\n");
		CLOSESOCKET(getSocket());
		setSocket(INVALID_SOCKET);
	}

	_bRun = true;
	return true;
}

//*************************************************************************
//
// Function : acceptLoop
//
//*************************************************************************
void ServerDaemon::runBody()

{
	int              len;
	struct           sockaddr_in sin;
	SOCKET           s;


	while (_bRun)
	{
		len = sizeof sin;
		memset( &sin, 0, sizeof sin );

		s = accept(getSocket(), (struct sockaddr *) &sin, &len);

		if (s == SOCKET_ERROR)  break;  // if accept() failed

		Thread t(getFactory()->createServerCommunicator(s, this));
		t.start();

	}
	CLOSESOCKET(getSocket());
}




