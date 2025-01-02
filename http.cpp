#include "http.h" // Look at what I can make not part of the class and merely util functions; file is getting pretty crowded

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
#define HTTP_VERSION "HTTP/1.1"
#define RECV_BUFF 4096

namespace HTTP
{

	const std::map<std::string, std::string> Client::defh = {
		{"Host", ""},
		{"Accept", "*/*"},
		{"Connection", "close"}
	};

	void recvHandler(SOCKET clsoc) {
		std::string data = "";
		char recvBuff[RECV_BUFF] = "";
		int recvC = 0;
		int iResult = 0;
		do {
			iResult = recv(clsoc, recvBuff, sizeof(recvBuff), 0);
			std::cout << "Data size: " << iResult << std::endl;
			if (iResult > 0) {
				// Garbage info from setting char array last exists so need to pull all bytes allocated this time
				data.append(recvBuff, iResult);
				recvC++;
				std::cout << "\r\nRecv Count " << recvC << "\r\nData Preview: \r\n" << recvBuff << std::endl;
			}
			else if (iResult == 0) {
				std::cout << "All data received\r\n" << std::endl;
			}
			else {
				std::cout << "Receive failed: " << WSAGetLastError() << std::endl;
			}
		} while (iResult > 0);

		std::cout << "Received Data:\n" << data << std::endl;
	}

	// Make type Data later and make non blocking
	void Client::get(std::string hostName, std::string path) {
		// Need to resolve hostname - this is its own function in namespace but not class
		// Need to pool - call pooling
		// Need to connect socket - make a function maybe or maybe not most likely yes
		// Need to send request - need function to setup http format and that same function will send it
		// Receive requests connection closed for now (no chunking yet)
		// Return socket to pool (make sure addrinfo is freed)
		Sockets::ConSoc consoc = Client::resolveConnection(hostName);
		ReqType method = ReqType::GET;
		std::string formattedReq = requestConstructor(method, hostName, path);
		std::cout << "Request:\n" << formattedReq << std::endl;
		int iResult = send(consoc.clsoc, formattedReq.c_str(), formattedReq.length(), 0);
		if (iResult == SOCKET_ERROR) {
			pool->clean();
			throw std::runtime_error("Sending failed: " + WSAGetLastError());
		}
		std::cout << "Request successfully sent to " << consoc.getIP() << std::endl;
		recvHandler(consoc.clsoc);

	}

	Client::Client(int socnum) {

		pool = Sockets::initialize(socnum);

	}

	// cshd stands for custom headers; feels like its so inefficient using so many maps; maybe make overloaded function?
	std::string constructHeaders(std::map<std::string, std::string> headers) {
		std::string requestHeader = "";
		for (auto it = headers.begin(); it != headers.end(); ++it) {
			std::string key = it->first;
			std::string value = it->second;
			requestHeader += key + ": " + value + "\r\n";
		}
		requestHeader += "\r\n";
		return requestHeader;
	}

	// Make method like an enum or something with set choices; should it be in class or not idk
	std::string Client::requestConstructor(ReqType method, std::string hostName, std::string path) {
		std::string request = "";
		std::string requestHeaders = "";
		std::string requestBody = "";
		switch (method) {
		case ReqType::GET:
				requestHeaders += "GET " + path + " " + HTTP_VERSION + "\r\n";
				std::map<std::string, std::string> headers = defh;
				headers["Host"] = hostName;
				requestHeaders += constructHeaders(headers);
		}
		request = requestHeaders + requestBody;
		std::cout << "Request preview:\n" << request << std::endl;
		return request;
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

		Sockets::ConSoc consoc(clsoc, srvsoc);

		return consoc;
	}

	// Unnecessary to abstract resolving dns (maybe do later idk); js abstracting connection process
	Sockets::ConSoc Client::resolveConnection(std::string hostName) {
		int iResult = 0;
		struct addrinfo* result = NULL, * ptr = NULL, hints;
		ZeroMemory(&hints, sizeof(hints));
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