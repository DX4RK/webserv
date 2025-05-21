#pragma once

#include <iostream>

#include "response.hpp"
#include "listenSocket.hpp"

#define WEB_ROOT "./web"

class ListenSocket;

struct parsing {
	bool proceed;
	int status_code;
	std::string status_message;

};

class Request {
public:
	Request( void );
	Request( ListenSocket &listener );

	std::string getMethod( void ) const;
	std::string getUrl( void ) const;
	std::string getProtocol( void ) const;
	std::string getFileName( void ) const;

	struct parsing getParsing( void ) const;
	std::map<std::string, std::string> getHeaders( void ) const;
private:
	std::string _method;
	std::string _protocol;
	std::string _filePath;
	std::string _fileName;
	std::map<std::string, std::string> _headers;

	bool _formatHeader(const std::string &headerLine);
	bool _handleFileRequest(const std::string &filePath);

	struct parsing _parsingResult;
};
