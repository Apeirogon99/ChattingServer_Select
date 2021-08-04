#pragma once
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include "CorePch.h"
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 4096

struct Session
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[BUFSIZE] = {};
	int32 recvBytes = 0;
	int32 sendBytes = 0;
	int32 sessionIndex = NULL;
};

class SelectServer
{
public:
	SelectServer();
	~SelectServer();

public:
	bool InitSocket();
	bool BindandListen(int32 nBindPort);

	bool ServerRun();

	void MultiCastClients(Session& session);
	void OnAccept();
	bool OnRecv(Session& session);
	bool OnSend(Session& session);

	void UpdateSelect();
	void SelectResult();

	void CloseSocket(Session& session);

private:

private:
	SOCKET					m_listenSocket;
	std::vector<Session>	m_ClientSession;
	fd_set					m_reads;
	fd_set					m_writes;
};

