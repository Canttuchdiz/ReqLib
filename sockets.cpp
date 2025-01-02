#include "sockets.h"

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
	ConnectionPool::ConnectionPool(int socnum)
	{
		maxsoc = socnum;
		for (int i = 0; i < socnum; i++)
		{
			// sochdl stands for socket handle
			SOCKET sochdl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sochdl == INVALID_SOCKET) {
				ConnectionPool::clean();
				throw std::runtime_error("Socket creation error: " + WSAGetLastError());
			}
			opsoc.push(sochdl);
		}
	}

	ConnectionPool::~ConnectionPool()
	{
		// Check to see if already closed?
		ConnectionPool::clean();
	}

	SOCKET ConnectionPool::poolSocket()
	{
		if (opsoc.empty()) {
			ConnectionPool::clean; // put close sockets here maybe?
			throw std::runtime_error("Socket pool is empty");
		}
		SOCKET sochdl = opsoc.front();
		opsoc.pop();
		ocsoc.push_back(sochdl);
		return sochdl;
	}

	//Shutdown must occur by the client? function itself is redundant
	// Must be a valid socket...
	void ConnectionPool::returnSocket(SOCKET soc) {
		int iResult = shutdown(soc, SD_BOTH);
		if (iResult == SOCKET_ERROR) {
			// Likely invalid socket; socerr stands for socket error
			ConnectionPool::clean(); // do i put this here or not
			throw std::runtime_error("Invalid socket a/o socerr: " + WSAGetLastError());
		}
		// Removes socket
		ocsoc.erase(std::remove(ocsoc.begin(), ocsoc.end(), soc), ocsoc.end());
		opsoc.push(soc);
		std::cout << "issue" << std::endl;
	}

	// In http object, have a close function which deletes the object which will inherently call this. only call it directly on errors; must be called
	void ConnectionPool::clean() {
		int opsocLen = opsoc.size();
		if (opsocLen < maxsoc) {
			// Find a way to keep track of active sockets, pull them back, and then deactivate them; use ocsoc
			// Make cerr later?
			std::cout << "Sockets still in-use" << std::endl;
		}
		while (!opsoc.empty()) {
			SOCKET sochdl = opsoc.front();
			opsoc.pop();
			closesocket(sochdl);
		}
		WSACleanup();
		std::cout << "Pool successfully closed..." << std::endl;
	}

	// Idk if I need clean bc wsacleanup cld be unnecessary
	std::unique_ptr<ConnectionPool> initialize(int socnum) {
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