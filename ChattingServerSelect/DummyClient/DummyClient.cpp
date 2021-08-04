#include "pch.h"
//윈도우소켓
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 4096
#define IP "127.0.0.1"


void OnSend(SOCKET clientSocket)
{
	char sendBuffer[BUFSIZE];
	while (true)
	{
		cin.getline(sendBuffer, BUFSIZE);
		
		if (::send(clientSocket, sendBuffer, strlen(sendBuffer), 0) == SOCKET_ERROR)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			break;
		}

		cout << "Send Data Len = " << strlen(sendBuffer) << endl;

		this_thread::sleep_for(1s);

	}
}

void OnRecv(SOCKET clientSocket)
{
	char recvBuffer[BUFSIZE];
	while (true)
	{
		int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

		if (recvLen == SOCKET_ERROR)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			break;
		}
		else if (recvLen == 0)
		{
			break;
		}

		recvBuffer[recvLen] = '\0';
		cout << "recv Data = " << recvBuffer << endl;
		this_thread::sleep_for(1s);
	}
}

int main()
{
	WSAData wsaData;
	WORD wsaVer = MAKEWORD(2, 2);
	if (::WSAStartup(wsaVer, &wsaData) != 0)
		return 0;


	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
		return 0;

	u_long on = 1;
	if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
		return 0;

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(9000);

	while (true)
	{
		if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			if (::WSAGetLastError() == WSAEISCONN)
				break;

			break;
		}
	}

	cout << "Connect to Server" << endl;

	thread 	t1(OnRecv, clientSocket);
	thread 	t2(OnSend, clientSocket);

	t1.join();
	t2.join();

	::closesocket(clientSocket);

	::WSACleanup();

}
