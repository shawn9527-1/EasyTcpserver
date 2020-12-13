#include <iostream>
#include <thread>

#include "EasyTcpClient.hpp"

#pragma comment(lib, "ws2_32.lib")

bool g_bExit = true;

void cmdThread(EasyTcpClient* client)
{
	char cmdBuf[256] = { 0 };

	while (g_bExit)
	{
		scanf("%s", cmdBuf);

		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("exit\n");
			g_bExit = false;
			client->Close();
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "xiao");
			strcpy(login.passWord, "123456");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "xiao");
			client->SendData(&logout);
		}
		else
		{
			printf("don't handle this cmd\n");
		}
	}
}

int main(void)
{
	EasyTcpClient client;
	client.InitSocket();
	client.Connect("127.0.0.1", 4567);

	std::thread t1(cmdThread, &client);
	t1.detach();

	while (client.isRun())
	{
		client.OnRun();
	}

	client.Close();

	return 0;
}