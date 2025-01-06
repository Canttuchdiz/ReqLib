#ifndef HTTP_P_H
#define HTTP_P_H
// use buffer word
#pragma once

#include <iostream>
#include <winsock2.h>
#include <string>
#include <vector>
#include <map>

namespace HTTP
{

	enum class ReqType { GET, POST };

	std::vector<std::string> tokenize(const std::string &str, const std::string &delim);
	// Used in get function which will return Data
	int parseStatus(const std::string &response);
	std::string parseBody(const std::string &response);
	std::map<std::string, std::string> parseHeaders(const std::string &response);
}

#endif