#ifndef HTTP_H
#define HTTP_H
// use buffer word
#pragma once

#include <iostream>
#include <winsock2.h>
#include <string>
#include <map>

#include "sockets.h"

namespace HTTP
{
	class Client // Inherit ConnectionPool?
	{
	// Instance variables
	public:
		// Are set to default headers; more headers can be added on when passing in information; maybe make private and have getters/setters?
		std::map<std::string, std::string> defh;
	// Instance variables
	private:
		std::unique_ptr<Sockets::ConnectionPool> pool;
	// Constructor
	public:
		// Initializes everything and makes unique_ptr
		Client(int socNum);
	// Methods
	public:
		// maybe have overload with and without headers; for now add headers onto previous ones
		Data get(std::string hostName, std::map<std::string, std::string> headers);
	// Methods
	private:
		// Place output in this function that says connected to IP
		Sockets::ConSoc findConnection(addrinfo* result);
		Sockets::ConSoc resolveConnection(std::string hostName);
	};

	// Make readonly
	typedef struct {
		int status;
		std::map<std::string, std::string> headers;
		std::string content;
	} Data;	
}

#endif