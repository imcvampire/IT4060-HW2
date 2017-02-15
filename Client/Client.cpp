#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<string>

using namespace std;

const int PORT = 5000;
const char ADDR[] = "127.0.0.1";
const int BUFFER_SIZE = 2048;
const int timeout = 10000;

int main(int argc, char* argv[])
{
	WSADATA wsa_data;
	WORD version = MAKEWORD(2, 2);

	if (WSAStartup(version, &wsa_data))
	{
		cerr << "Version is not supported!" << endl;

		return 1;
	}

	SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char *) (&timeout), sizeof(int));

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;

	if (argc == 1)
	{
		server_addr.sin_port = htons((u_short) PORT);
		inet_pton(AF_INET, ADDR, (void*) &(server_addr.sin_addr.s_addr));
	}
	else if (argc == 2)
	{
		server_addr.sin_port = htons((u_short) PORT);
		inet_pton(AF_INET, argv[1], (void*)  &(server_addr.sin_addr.s_addr));
	}
	else
	{
		server_addr.sin_port = htons((u_short) stoi(argv[2]));
		inet_pton(AF_INET, argv[1], (void*) &(server_addr.sin_addr.s_addr));
	}

	char buffer[BUFFER_SIZE];
	char buffer_server[BUFFER_SIZE];
	int ret;
	int server_addr_len = sizeof(server_addr);

	for (;;)
	{
		cout << "Send to server: " << endl;
		cin >> buffer;

		ret = sendto(client, buffer, strlen(buffer), 0, (sockaddr *) &server_addr, server_addr_len);

		if (ret == SOCKET_ERROR)
		{
			cerr << "Can not send to server! Error code: " << WSAGetLastError() << endl;

			continue;
		}

		buffer[ret] = 0;

		ret = recvfrom(client, buffer_server, BUFFER_SIZE, 0, (sockaddr *) &server_addr, &server_addr_len);

		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT)
			{
				cerr << "Timeout!" << endl;
			}
			else
			{
				cerr << "Can not receive message! Error code: " << WSAGetLastError() << endl;
			}

			continue;
		}
		else if (strlen(buffer) > 0)
		{
			buffer_server[ret] = 0;

			char addr[INET_ADDRSTRLEN];

			inet_ntop(AF_INET, &(server_addr.sin_addr), addr, INET_ADDRSTRLEN);

			cout
				<< "Convert "
				<< buffer
				<< " to "
				<< buffer_server
				<< endl;
		}

		cout << endl;
	} 

	closesocket(client);

	WSACleanup();

	return 0;
}