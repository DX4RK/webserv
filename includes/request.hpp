#pragma once

#include <iostream>

#include "config.hpp"
#include "response.hpp"
#include "listenSocket.hpp"

class ListenSocket;

class Request {
public:
	Request( ListenSocket &listener, Config *config );

	int getStatusCode( void ) const;

	std::string getUrl( void ) const;
	std::string getMethod( void ) const;
	std::string getProtocol( void ) const;

	bool isCgiEnabled( void ) const;
	std::string findHeader( std::string index );

	std::map<std::string, std::string> getHeaders( void ) const;
private:
	int	_statusCode;
	bool _cgiEnabled;

	std::string _url;
	std::string _refer;
	std::string _body;
	std::string _method;
	std::string _protocol;

	std::map<std::string, std::string> _headers;

	Config *server_config;

	bool _formatHeader( const std::string &headerLine );


	Request( void );
};
