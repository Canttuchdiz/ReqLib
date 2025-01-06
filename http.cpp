#include "http.h" // Look at what I can make not part of the class and merely util functions; file is getting pretty crowded
// Organize such that namespace functions and stuff are at the bottom and namespace functions & objects are at the top
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <sstream>
#include <string>
#include <map>

#include "pool.h"
#include "parser.h"

#define HTTP_PORT "80"
#define HTTP_VERSION "HTTP/1.1"
#define RECV_BUFF (4096)

namespace HTTP
{

	const std::map<std::string, std::string> Client::defh = {
		{"Host", ""},
		{"Accept", "*/*"},
		{"Connection", "close"}
	};

	std::string recvHandler(const SOCKET &clsoc) {
		std::string data = "";
		char recvBuff[RECV_BUFF] = "";
		int recvC = 0;
		int iResult = 0;
		do {
			iResult = recv(clsoc, recvBuff, sizeof(recvBuff), 0);
			if (iResult > 0) {
				// Garbage info from setting char array last exists so need to pull all bytes allocated this time
				data.append(recvBuff, iResult);
				recvC++;
			}
			else if (iResult == 0) {
				std::cout << "All data received\r\n" << std::endl;
			}
			else {
				std::cout << "Receive failed: " << WSAGetLastError() << std::endl;
			}
		} while (iResult > 0);

		return data;
	}

	Data Client::get(const std::string &hostName, const std::string &path) {
		std::unique_ptr<Sockets::ConSoc> consoc = resolveConnection(hostName);
		ReqType method = ReqType::GET;
		std::string formattedReq = requestConstructor(method, hostName, path, HTTP_VERSION, defh);
		std::cout << "\nRequest:\n" << formattedReq << std::endl; // have file for outputting stuff... like debug file?
		int iResult = send(consoc->clsoc, formattedReq.c_str(), formattedReq.length(), 0);
		if (iResult == SOCKET_ERROR) {
			pool->clean();
			throw std::runtime_error("Sending failed: " + WSAGetLastError());
		}
		std::cout << "Request successfully sent to " << consoc->getIP() << std::endl;
		std::string resBuff = recvHandler(consoc->clsoc);

		return Data(parseStatus(resBuff), parseHeaders(resBuff), parseBody(resBuff));
	}

	Client::Client(const int socNum) {

		pool = Sockets::initialize(socNum);

	}

	void findConnection(addrinfo *result, std::unique_ptr<Sockets::ConSoc>& consoc, std::unique_ptr<Sockets::ConnectionPool> &pool) {
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

		Sockets::ConSoc *connection = new Sockets::ConSoc(clsoc, srvsoc);
		consoc.reset(connection);
	}

	// Unnecessary to abstract resolving dns (maybe do later idk); js abstracting connection process
	std::unique_ptr<Sockets::ConSoc> Client::resolveConnection(const std::string &hostName) {
		int iResult = 0;
		std::unique_ptr<Sockets::ConSoc> consoc;
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

		findConnection(result, consoc, pool);

		std::cout << "Connected to " << consoc->getIP().c_str() << " at port " << HTTP_PORT << std::endl;
		
		return consoc;
	}

	std::string Data::getHeaders() const {
		std::string delim = CRLF;
		std::string headerBuff = "";
		for (auto &header : headers) {
			headerBuff += header.first + ": " + header.second + delim;
		}
		return headerBuff.substr(0, headerBuff.length() - delim.length());
	}
}