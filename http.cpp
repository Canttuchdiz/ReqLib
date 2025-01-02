#include "http.h"

#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <string>
#include <map>

#include "sockets.h"

#define HTTP_PORT "80"

namespace HTTP
{
	// Do I need the client thing there?
	Data Client::get(std::string hostName, std::map<std::string, std::string> headers) {
		// Need to resolve hostname - this is its own function in namespace but not class
		// Need to pool - call pooling
		// Need to connect socket - make a function maybe or maybe not most likely yes
		// Need to send request - need function to setup http format and that same function will send it
		// Receive requests connection closed for now (no chunking yet)
		// Return socket to pool (make sure addrinfo is freed)
		Client::resolveConnection("example.com");

	}

	Client::Client(int socnum) {
		pool = Sockets::initialize(socnum);
		// Initialize host when connection is formed
		defh = {
			{"Host", ""},
			{"Accept", "*/*"},
			{"Connection", "close"} };

	}

	Sockets::ConSoc Client::findConnection(addrinfo *result) {
		// Initialize to non-garbage value
		SOCKET clsoc = INVALID_SOCKET;
		struct sockaddr* srvsoc = nullptr;
		addrinfo *ptr = NULL;
		int iResult = 0;

		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			if (ptr->ai_family == AF_INET)
			{
				clsoc = pool->poolSocket();
				if (clsoc == INVALID_SOCKET) {
					freeaddrinfo(result); // Any way to make it so I dont have to write these two lines each time I throw exception
					pool->clean();
					throw std::runtime_error("Socket creation failed: " + WSAGetLastError());
				}

				iResult = connect(clsoc, ptr->ai_addr, sizeof *(ptr->ai_addr));
				if (iResult == SOCKET_ERROR) {
					pool->returnSocket(clsoc);
					clsoc = INVALID_SOCKET;
					continue;
				}
				freeaddrinfo(result);

				if (clsoc == INVALID_SOCKET) {
					pool->clean();
					throw std::runtime_error("Connection failed: " + WSAGetLastError());
				}
				srvsoc = ptr->ai_addr;
				break;
			}
		}

		Sockets::ConSoc consoc;
		consoc.clsoc = clsoc;
		consoc.srvsoc = srvsoc;

		return consoc;
	}

	// Unnecessary to abstract resolving dns (maybe do later idk); js abstracting connection process
	Sockets::ConSoc Client::resolveConnection(std::string hostName) {
		int iResult = 0;
		struct addrinfo* result = NULL, * ptr = NULL, hints;
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Why pass in pointer to result instead of result itself bc result is a pointer
		iResult = getaddrinfo(hostName.c_str(), HTTP_PORT, &hints, &result);
		if (iResult != 0) {
			pool->clean();
			throw std::runtime_error("Address failed: " + WSAGetLastError());
		}

		Sockets::ConSoc consoc = Client::findConnection(result);

		std::cout << "Connected to " << consoc.getIP().c_str() << " at port " << HTTP_PORT << std::endl;
		
		return consoc;
	}
}