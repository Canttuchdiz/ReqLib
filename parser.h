#ifndef HTTP_P_H
#define HTTP_P_H
// use buffer word
#pragma once

#include <iostream>
#include <winsock2.h>
#include <string>
#include <map>

namespace HTTP
{

	enum class ReqType { GET, POST };

	// Used in get function which will return Data
	std::string parseStatus(std::string response);
	std::string parseBody(std::string response);
	std::map<std::string, std::string> parseHeaders(std::string response);
	std::string requestConstructor(ReqType method, std::string hostName, std::string path, std::map<std::string, std::string> headers, std::string body);
}

#endif