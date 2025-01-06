#include "parser.h"

#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <sstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>

#define CRLF "\r\n"
#define CONCAT(str1, str2) str1 str2

#define MALFORMED_REQUEST "Request structure is malformed"

namespace HTTP
{
	// Reads kth word
	std::string readK(const std::string &response, const int wordNum) {
		std::stringstream stream(response);
		std::string wordBuff = "";
		for (int i = 0; i < wordNum; i++) {
			stream >> wordBuff;
		}
		return wordBuff;
	}
	std::vector<std::string> tokenize(const std::string &str, const std::string &delim) {
		std::string strBuff = str;
		std::vector<std::string> tokensVec;
		size_t location = strBuff.find(delim);

		while (location != std::string::npos) {
			std::string startBuff = strBuff.substr(0, location);
			tokensVec.push_back(startBuff);
			strBuff = strBuff.substr(location + delim.length());
			location = strBuff.find(delim);
		}

		tokensVec.push_back(strBuff);
		return tokensVec;
	}
	// Have parser functions throw malformed_request error and handle it in funtion -> have function that constructs all this info into Data object
	int parseStatus(const std::string &response) {
		try {
			return std::stoi(readK(response, 2));
		}
		catch (const std::exception& e) {
			throw std::runtime_error(MALFORMED_REQUEST);
		}
	}

	std::string parseBody(const std::string &response) {
		std::string fieldDelim = CONCAT(CRLF, CRLF);
		size_t location = response.find(fieldDelim);

		if (location == std::string::npos) throw std::runtime_error(MALFORMED_REQUEST);

		std::string bodyBuff = response.substr(location + fieldDelim.length());
		return bodyBuff;
	}
	std::map<std::string, std::string> parseHeaders(const std::string &response) {
		std::string delim = CRLF;
		size_t location = response.find(delim + delim);

		if (location == std::string::npos) throw std::runtime_error(MALFORMED_REQUEST); // Handle malformed in body of for loop
		
		std::map<std::string, std::string> headers;
		std::string headerBuff = response.substr(0, location);
		std::vector<std::string> tokenizedLines = tokenize(headerBuff, delim);

		bool first = true;
		for (const std::string &header : tokenizedLines) {
			if (first) { first = false; continue; }
			size_t splitLoc = header.find(":");
			// I add 2 because 1 for : and 1 for whitespace... i only use colon once so i think pointless to define it
			headers[header.substr(0, splitLoc)] = header.substr(splitLoc + 2);
		}
		return headers;
	}
	//std::string requestConstructor(ReqType method, std::string hostName, std::string path, std::map<std::string, std::string> headers, std::string body) {

	//}

}