#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<string>

using namespace std;

const int PORT = 5000;
const char ADDR[] = "127.0.0.1";
const int BUFFER_SIZE = 2048;

// <summary>Start UDP Echo Server. This program can receive up to 2 arguments. Server will response with hostname or IP of client's request.</summary>
// <argument name="address">Server's address</argument>
// <argument name="port">Server's port</argument>
int main(int argc, char* argv[])
{
	WSADATA wsa_data;
	WORD version = MAKEWORD(2, 2);

	if (WSAStartup(version, &wsa_data))
	{
		cerr << "Version is not supported!" << endl;

		return 1;
	}

	SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;

	if (argc == 1)
	{
		server_addr.sin_port = htons((u_short) PORT);
		if (inet_pton(AF_INET, ADDR, (void*) &(server_addr.sin_addr.s_addr)) != 1)
		{
			cerr << "Can not convert little-endian to big-endian" << endl;

			return 1;
		}
	}
	else if (argc == 2)
	{
		server_addr.sin_port = htons((u_short) PORT);
		if (inet_pton(AF_INET, argv[1], (void*)  &(server_addr.sin_addr.s_addr)) != 1)
		{
			cerr << "Can not convert little-endian to big-endian" << endl;

			return 1;
		}
	}
	else
	{
		server_addr.sin_port = htons((u_short) stoi(argv[2]));
		if (inet_pton(AF_INET, argv[1], (void*) &(server_addr.sin_addr.s_addr)) != 1)
		{
			cerr << "Can not convert little-endian to big-endian" << endl;

			return 1;
		}
	}

	cout << "Server started!" << endl;

	sockaddr_in client_addr;
	char buffer[BUFFER_SIZE];
	int ret;
	int client_addr_len = sizeof(client_addr);

	for (;;)
	{
		ret = recvfrom(server, buffer, BUFFER_SIZE, 0, (sockaddr*) &client_addr, &client_addr_len);

		if (ret == SOCKET_ERROR)
		{
			cerr << "Error: " << WSAGetLastError() << endl;
		}
		else if (strlen(buffer) > 0)
		{
			buffer[ret] = 0;

			sockaddr_in address;
			address.sin_family = AF_INET;
			inet_pton(AF_INET, buffer, (void*) &(address.sin_addr.s_addr));
			address.sin_port = client_addr.sin_port;

			char result_hostname[NI_MAXHOST];
			char result_server_info[NI_MAXSERV];

			// Need to initialize 2 var below (if not it can be set to 0)
			int convert_result_1 = 1, convert_result_2 = 1;

			// Try to convert request to hostname
			convert_result_1 = getnameinfo(
				(sockaddr*) &address,
				sizeof sockaddr,
				result_hostname,
				NI_MAXHOST,
				result_server_info,
				NI_MAXSERV,
				NI_NUMERICSERV
			);

			addrinfo *result_addrinfo;

			// Try to convert request to IP
			convert_result_2 = getaddrinfo(buffer, "http", NULL, &result_addrinfo);

			// getnameinfo will return 0 if it receive IP (and set result to this IP)
			if (convert_result_1 == 0 && strcmp(result_hostname, buffer))
			{
				ret = sendto(
					server,
					result_hostname,
					strlen(result_hostname),
					0,
					(sockaddr *) &client_addr,
					sizeof(client_addr)
				);
			} 
			else if (convert_result_2 == 0) 
			{
				sockaddr_in address;

				memcpy(&address, result_addrinfo->ai_addr, result_addrinfo->ai_addrlen);

				char result_ip[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(address.sin_addr), result_ip, INET_ADDRSTRLEN);

				ret = sendto(
					server,
					result_ip,
					strlen(result_ip),
					0,
					(sockaddr *) &client_addr,
					sizeof(client_addr)
				);
			}

			if (ret == SOCKET_ERROR)
			{
				cerr << "Error: " << WSAGetLastError() << endl;
			}

			freeaddrinfo(result_addrinfo);
		}
	}

	closesocket(server);

	WSACleanup();

	return 0;
}