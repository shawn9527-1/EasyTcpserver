#include "EasyTcpServer.hpp"

int main(void)
{
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	
	while (server.isRun())
	{
		server.onRun();
	}

	server.Close();

	return 0;
}