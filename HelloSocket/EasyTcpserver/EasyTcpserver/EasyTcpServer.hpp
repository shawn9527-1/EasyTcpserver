#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define SOCKET int
#define INVALID_SOCKET (SOCKET) (0)
#define SOCKET_ERROR            (-1)
#endif

#include <vector>
#include "MessageHeader.hpp"

class EasyTcpServer
{
public:
	EasyTcpServer() : _sock(INVALID_SOCKET)
	{

	}

	virtual ~EasyTcpServer()
	{

	}

	//init
	void InitSocket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#else
		
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("close before connect\n");
			Close();
		}

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	//bind
	int Bind(const char *ip, int port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		if (SOCKET_ERROR == bind(_sock, (sockaddr *)&_sin, sizeof(_sin)))
		{
			printf("bind error\n");
			return -1;
		}

		return 0;
	}
	
	//listen
	int Listen(int backlog)
	{
		if (SOCKET_ERROR == listen(_sock, backlog))
		{
			printf("listen error\n");
			return -1;
		}

		return 0;
	}

	SOCKET Accept()
	{
		SOCKET client;
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		client = accept(_sock, (sockaddr *)&clientAddr, &nAddrLen);

//		printf("accept:%d, addr:%d, port:%d\n", client, inet_a clientAddr.sin_addr.S_un.S_addr)

		g_clients.push_back(client);

		for (int n = g_clients.size() - 1; n >= 0; n--)
		{
			NewUserJoin join;
			join.sock = client;
			SendData(g_clients[n], &join);
		}		

		return client;
	}

	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
			closesocket(_sock);
		}
	}

	//recvData
	bool onRun()
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);

		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		timeval t = { 0, 500 };
		int ret = select(_sock + 1, &fdRead, nullptr, nullptr, &t);
		if (ret < 0)
		{
			printf("select error\n");
			return false;
		}

		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			Accept();
		}

		return true;
	}

	bool isRun()
	{
		return (_sock != INVALID_SOCKET);
	}

	int RecvData(SOCKET sock)
	{
		int nLen = 0;
		char _recvBuf[1024] = { 0 };
		DataHeader *header = nullptr;
		//5. recv data
		memset(_recvBuf, 0, sizeof(_recvBuf));
		nLen = recv(sock, _recvBuf, sizeof(DataHeader), 0);
		header = (DataHeader *)_recvBuf;
		if (nLen <= 0)
		{
			printf("client exit\n");


			return -1;
		}

		OnNetMsg(sock, header);
		return 0;
	}

	int SendData(SOCKET sock, DataHeader *dataHeader)
	{
		int sendBytes = 0;
		if (_sock != INVALID_SOCKET)
		{
			sendBytes = send(sock, (const char*)dataHeader, dataHeader->dataLength, 0);
		}

		return sendBytes;
	}

	virtual void OnNetMsg(SOCKET sock, DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login login = {};
			recv(sock, (char *)&login + sizeof(DataHeader), sizeof(Login), 0);

			printf("recv login:cmd:%d, dataLen:%d, user:%s, passwd:%s\n", login.cmd, login.dataLength, login.userName, login.passWord);

			LoginResult ret;

			send(sock, (char *)&ret, sizeof(LoginResult), 0);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout logout = {};
			recv(sock, (char *)&logout + sizeof(DataHeader), sizeof(Logout), 0);

			printf("recv logout:cmd:%d, dataLen:%d, user:%s\n", logout.cmd, logout.dataLength, logout.userName);

			LogoutResult ret;

			send(sock, (char *)&ret, sizeof(LoginResult), 0);
			break;
		}
		default:
		{
			DataHeader respone;
			respone.cmd = CMD_ERROR;
			send(sock, (char *)&respone, sizeof(DataHeader), 0);
			break;
		}
		}

		return;
	}

	void SendDataToAll(DataHeader *data)
	{
		std::vector<int>::iterator iter = g_clients.begin();
		for (; iter != g_clients.end(); iter++)
		{
			LogoutResult logout;
			logout.result = 999;
			send(*iter, (char*)&logout, sizeof(LogoutResult), 0);
		}
	}

private:
	SOCKET _sock;
	std::vector<int> g_clients;
};

#endif