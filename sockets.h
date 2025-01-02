#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <winsock2.h>

namespace Sockets
{
	// Not user facing
	class ConnectionPool
	{
		// Constructor shenanigans
	public:
		// Open multiple sockets
		ConnectionPool(int socNum);
		~ConnectionPool();
		// Methods
	public:
		SOCKET poolSocket();
		// opsoc is like open socket
		void returnSocket(SOCKET soc);
		void clean();
		// Instance variables
	private:
		// Max sockets in pool; use getters and setters later? prolly unnecessary bc this isnt userfacing
		int maxsoc;
		// opsoc stands for open sockets (available sockets)
		std::queue<SOCKET> opsoc;
		// ocsoc stands for occupied sockets
		std::vector<SOCKET> ocsoc;
	};

	std::unique_ptr<ConnectionPool> initialize(int socNum);

	typedef struct {
		SOCKET clsoc;
		struct sockaddr *srvsoc;

		std::string getIP() const;
	} ConSoc; // Connected sockets
}

#endif