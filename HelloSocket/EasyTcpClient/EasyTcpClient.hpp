#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib");
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SOCKET) (0)
	#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include "MessageHeader.hpp"

#define MAX_BUF_SIZE (512)

class EasyTcpClient
{
public:
	EasyTcpClient() : _sock(INVALID_SOCKET)
	{
		
	}

	virtual ~EasyTcpClient()
	{

	}

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
			printf("close before connet\n");
			Close();
		}

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	int Connect(char *ip, unsigned short port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);

#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("connect error\n");
			return -1;
		}
	}

	void Close()
	{
#ifdef _WIN32
		closesocket(_sock);
		WSACleanup();
#else
		close(_sock);
#endif
	}

	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdRead;
			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);
			timeval t = { 1, 0 };
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0)
			{
				//printf("recv event end\n");
				return false;
			}

			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);

				if (-1 == RecvData(_sock))
				{

				}
			}

			return true;
		}

		return false;
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	int RecvData(int sock)
	{
		char szRecv[4096] = {};
		int nLen = recv(sock, szRecv, sizeof(DataHeader), 0);
		if (nLen > 0)
		{
			DataHeader* header = (DataHeader *)szRecv;
			int nLen = recv(sock, szRecv + sizeof(DataHeader), header->dataLength, 0);

			OnNetMsg(header);
		}		

		return 0;
	}

	void OnNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* login = (LoginResult *)header;
			printf("loginRet:%d\n", login->result);
			break;
		}
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logout = (LogoutResult *)header;
			printf("##############logout ret:%d\n", logout->result);
			break;
		}
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* join = (NewUserJoin *)header;
			printf("NewUserJoin sock:%d\n", join->sock);
			break;
		}
		default:
			break;
		}
	}

	int SendData(DataHeader *header)
	{
		return send(_sock, (const char *)header, header->dataLength, 0);
	}

private:
	SOCKET _sock;
};

#endif