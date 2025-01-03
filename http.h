#ifndef HTTP_H
#define HTTP_H
// use buffer word
#pragma once

#include <iostream>
#include <winsock2.h>
#include <string>
#include <map>

#include "pool.h"
#include "parser.h"

namespace HTTP
{

	class Client
	{
	// Instance variables
	private:
		static const std::map<std::string, std::string> defh;
		std::unique_ptr<Sockets::ConnectionPool> pool;
	// Constructor
	public:
		// Initializes everything and makes unique_ptr
		Client(int socNum);
	// Methods
	public:
		// maybe have overload with and without headers; for now add headers onto previous ones
		void get(std::string hostName, std::string path);
	// Methods
	private:
		Sockets::ConSoc resolveConnection(std::string hostName);

		std::string requestConstructor(ReqType method, std::string hostName, std::string path);
	};

	// Make readonly
	typedef struct Data{
		const int status;
		const std::map<std::string, std::string> headers;
		const std::string content;

		Data(int statusCode, std::map<std::string, std::string> reqHead, std::string recvBuff)
			: status(statusCode), headers(reqHead), content(recvBuff) {}
	};	
}

#endif