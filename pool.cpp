#include "pool.h"

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


namespace Sockets
{
	// Use smart pointers
	ConnectionPool::ConnectionPool(const int socNum) : maxsoc(socNum) // confused abt ts where did maxsoc come from
	{
		for (int i = 0; i < socNum; i++)
		{
			// sochdl stands for socket handle
			SOCKET sochdl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sochdl == INVALID_SOCKET) {
				clean();
				throw std::runtime_error("Socket creation error: " + WSAGetLastError());
			}
			opsoc.push(sochdl);
		}
	}

	ConnectionPool::~ConnectionPool()
	{
		clean();
	}

	SOCKET ConnectionPool::poolSocket()
	{
		if (opsoc.empty()) {
			clean(); // put close sockets here maybe?
			throw std::runtime_error("Socket pool is empty");
		}
		SOCKET sochdl = opsoc.front();
		opsoc.pop();
		ocsoc.push_back(sochdl);
		return sochdl;
	}

	void ConnectionPool::endConnection(const SOCKET &soc) {
		int iResult = shutdown(soc, SD_BOTH);
		if (iResult == SOCKET_ERROR) {
			// Likely invalid socket; socerr stands for socket error
			clean(); // do i put this here or not
			throw std::runtime_error("Invalid socket a/o socerr: " + WSAGetLastError());
		}
	}

	//Shutdown must occur by the client? function itself is redundant
	// Must be a valid socket...
	void ConnectionPool::returnSocket(const SOCKET &soc) {
		endConnection(soc);
		ocsoc.erase(std::remove(ocsoc.begin(), ocsoc.end(), soc), ocsoc.end());
		opsoc.push(soc);
	}

	void ConnectionPool::returnCS() { // Iterating issues while removing... run tests; check what iterator points to, print vector, etc
		for (auto it = ocsoc.begin(); it != ocsoc.end();) {
			SOCKET soc = *it;
			it = ocsoc.erase(it);
			endConnection(soc);
		}
	}

	// In http object, have a close function which deletes the object which will inherently call this. only call it directly on errors; must be called
	void ConnectionPool::clean() {
		int opsocLen = opsoc.size();
		if (opsocLen < maxsoc) {
			returnCS();
		}
		while (!opsoc.empty()) {
			SOCKET sochdl = opsoc.front();
			opsoc.pop();
			closesocket(sochdl);
		}
		WSACleanup();
		std::cout << "Pool successfully closed..." << std::endl;
	}

	// Idk if I need clean bc wsacleanup cld be unnecessary; is this a fine location for it?
	std::unique_ptr<ConnectionPool> initialize(const int socnum) {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			throw std::runtime_error("Startup failed: " + WSAGetLastError());
		}
		std::unique_ptr<ConnectionPool> pool(new ConnectionPool(socnum));
		return pool;
	}

	std::string ConSoc::getIP() const {
		struct sockaddr_in *srvinf = (struct sockaddr_in*)(srvsoc);
		char serverIP[INET_ADDRSTRLEN] = "";
		inet_ntop(AF_INET, &(srvinf->sin_addr), serverIP, sizeof(serverIP));
		return serverIP;
	}
}