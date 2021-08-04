#include "pch.h"
#include "SelectServerCore.h"

SelectServer::SelectServer()
{
	FD_ZERO(&m_reads);
	FD_ZERO(&m_writes);
	m_ClientSession.reserve(100);
	m_listenSocket = NULL;
}

SelectServer::~SelectServer()
{
	for (Session& session : m_ClientSession)
	{
		//if (session.recvBuffer)
			//delete[] session.recvBuffer;

		//if (session.sendBuffer)
			//delete[] session.sendBuffer;
	}
}

bool SelectServer::InitSocket()
{
	WSADATA wsaData;
	WORD ver = MAKEWORD(2, 2);
	
	if (::WSAStartup(ver, &wsaData) != 0)
		return false;

	m_listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_listenSocket == INVALID_SOCKET)
		return false;

	u_long on = 1;
	if (::ioctlsocket(m_listenSocket, FIONBIO, &on) == INVALID_SOCKET)
		return false;

	cout << "Socket Init Complete" << endl;
	return true;
}

bool SelectServer::BindandListen(int32 nBindPort)
{
	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(nBindPort);

	if (::bind(m_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return false;

	if (::listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return false;

	cout << "Accept" << endl;
	return true;
}

bool SelectServer::ServerRun()
{
	timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 5000;

	while (1)
	{
		UpdateSelect();
		
		auto result = select(0, &m_reads, 0, 0, &timeout);
		if (result == SOCKET_ERROR)
			return false;
		if(result == 0)
			continue;

		OnAccept();

		SelectResult();

	}
	return true;
}

void SelectServer::MultiCastClients(Session& session)
{

	for (Session& Multisession : m_ClientSession)
	{
		if (session.socket != Multisession.socket)
		{
			int32 sendLen = ::send(Multisession.socket, session.recvBuffer, session.recvBytes, 0);
			cout << "SC : " << sendLen << "_Len\t" << Multisession.socket << "_Socket" << endl;
		}
	}
	
}

void SelectServer::OnAccept()
{
	if (FD_ISSET(m_listenSocket, &m_reads))
	{
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);
		SOCKET clientSocket = ::accept(m_listenSocket, (SOCKADDR*)&clientAddr, &addrLen);

		if (clientSocket != INVALID_SOCKET)
		{
			Session newSession;
			newSession.socket = clientSocket;
			newSession.sessionIndex = static_cast<int32>(m_ClientSession.size());
			m_ClientSession.push_back(newSession);
		}

		cout << "New Client Connect to Server : " << clientSocket << endl;

	}
}

bool SelectServer::OnRecv(Session& session)
{
	if (FD_ISSET(session.socket, &m_reads))
	{
		int32 recvLen = ::recv(session.socket, session.recvBuffer, BUFSIZE, 0);
		if (recvLen <= 0)
		{
			CloseSocket(session);
			return false;
		}

		cout << "CS : " << recvLen << "_Len\t" << session.socket << "_Socket" << endl;

		session.recvBytes = recvLen;

		MultiCastClients(session);
	}

	return true;
}

bool SelectServer::OnSend(Session& session)
{
	int32 sendLen = ::send(session.socket, session.recvBuffer, session.recvBytes, 0);
	cout << "SC : " << sendLen <<endl;

	session.recvBytes = 0;

	return true;
}

void SelectServer::UpdateSelect()
{
	FD_ZERO(&m_reads);

	FD_SET(m_listenSocket, &m_reads);

	for (Session& session : m_ClientSession)
	{
		FD_SET(session.socket, &m_reads);
	}
}

void SelectServer::SelectResult()
{
	for (Session& session : m_ClientSession)
	{
		OnRecv(session);
	}
}

void SelectServer::CloseSocket(Session& session)
{
	cout << "Closed Client : " << session.socket << endl;

	closesocket(session.socket);
	FD_CLR(m_reads.fd_array[session.sessionIndex],&m_reads);
	m_ClientSession.erase(m_ClientSession.begin() + session.sessionIndex);

}

