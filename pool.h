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
		ConnectionPool(const int socNum);
		~ConnectionPool();
		// Methods
	public:
		SOCKET poolSocket();
		// Return CS means return connected sockets
		void returnCS();
		// opsoc is like open socket
		void returnSocket(const SOCKET &soc);
		void clean();
		// Instance variables
	private:
		// Max sockets in pool; use getters and setters later? prolly unnecessary bc this isnt userfacing
		const int maxsoc;
		// opsoc stands for open sockets (available sockets)
		std::queue<SOCKET> opsoc;
		// opcon stands for open connection
		std::vector<SOCKET> ocsoc;
	// Methods
	private:
		void endConnection(const SOCKET &soc);
	};

	std::unique_ptr<ConnectionPool> initialize(const int socNum);

	typedef struct ConSoc{
		const SOCKET clsoc;
		const struct sockaddr *srvsoc;

		ConSoc(const SOCKET &client, const struct sockaddr* server) : clsoc(client), srvsoc(server) {}

		std::string getIP() const;
	}; // Connected sockets
}

#endif