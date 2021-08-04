#include "pch.h"

int main()
{
	SelectServer server;
	if (!server.InitSocket())
	{
		cout << "Init Error" << endl;
		return 0;
	}	if (!server.BindandListen(9000))
	{
		cout << "bindListen Error" << endl;
		return 0;
	}
	server.ServerRun();
}