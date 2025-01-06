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

#define DEFSOC_LEN (15)

namespace HTTP
{

	// Make readonly
	typedef struct Data {
		const int status;
		const std::map<std::string, std::string> headers;
		const std::string content;

		std::string getHeaders() const;

		Data(const int statusCode, const std::map<std::string, std::string> &reqHead, const std::string &recvBuff)
			: status(statusCode), headers(reqHead), content(recvBuff) {}
	};

	class Client
	{
	// Instance variables
	private:
		static const std::map<std::string, std::string> defh;
		std::unique_ptr<Sockets::ConnectionPool> pool;
	// Constructor
	public:
		// Initializes everything and makes unique_ptr
		Client(const int socNum);
	// Methods
	public:
		// maybe have overload with and without headers; for now add headers onto previous ones
		Data get(const std::string &hostName, const std::string &path);
	// Methods
	private:
		Sockets::ConSoc resolveConnection(const std::string &hostName);
	};
}

#endif