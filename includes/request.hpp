#pragma once

#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <sstream>
#include "listenSocket.hpp"

class ListenSocket;

struct parsing {
	bool proceed;
	int status_code;
	std::string status_message;

};

class Request {
public:
	Request( ListenSocket &listener );
private:
	Request( void );

	std::string _method;
	std::string _url;
	std::string _protocol;

	struct parsing _parsingResult;
};
