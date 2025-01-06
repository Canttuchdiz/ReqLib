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

#define MALFORMED_REQUEST "Request structure is malformed" // Leave here because only used here?

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

	// Check if returning as pointer is more performant; idk if it will create copy or not
	std::map<std::string, std::string> parseHeaders(const std::string &response) {
		std::string delim = CRLF;
		size_t location = response.find(delim + delim);

		if (location == std::string::npos) throw std::runtime_error(MALFORMED_REQUEST); // If theres no double CRLF then this request is malformed
		
		std::map<std::string, std::string> headers;
		std::string headerBuff = response.substr(0, location);
		std::vector<std::string> tokenizedLines = tokenize(headerBuff, delim);

		bool first = true;
		for (const std::string &header : tokenizedLines) {
			if (first) { first = false; continue; }
			size_t splitLoc = header.find(":");
			if (location == std::string::npos) throw std::runtime_error(MALFORMED_REQUEST); // If there isnt a colon its clearly malformed
			// I add 2 because 1 for : and 1 for whitespace... i only use colon once so i think pointless to define it
			headers[header.substr(0, splitLoc)] = header.substr(splitLoc + 2);
		}
		return headers;
	}

	// cshd stands for custom headers
	std::string constructHeaders(const std::map<std::string, std::string>& headers) {
		std::string requestHeader = "";
		for (auto it = headers.begin(); it != headers.end(); ++it) {
			std::string key = it->first;
			std::string value = it->second;
			requestHeader += key + ": " + value + "\r\n";
		}
		requestHeader += "\r\n";
		return requestHeader;
	}

	// Maybe add another function for other types of requests
	std::string requestConstructor(const ReqType& method, const std::string& hostName, 
		const std::string& path, const std::string& httpVersion, const std::map<std::string, std::string>& defh) {
		std::string request = "";
		std::string requestHeaders = "";
		std::string requestBody = "";
		switch (method) {
		case ReqType::GET:
			requestHeaders += "GET " + path + " " + httpVersion + "\r\n";
			std::map<std::string, std::string> headers = defh;
			headers["Host"] = hostName;
			requestHeaders += constructHeaders(headers);
		}
		request = requestHeaders + requestBody;
		return request;
	}
}