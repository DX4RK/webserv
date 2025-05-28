#pragma once

#include <iostream>

#include "config.hpp"
#include "response.hpp"
#include "listenSocket.hpp"

class ListenSocket;

class Request {
public:
	Request( void );
	Request( ListenSocket &listener, Config &server_config );

	int getStatusCode( void ) const;

	std::string getUrl( void ) const;
	std::string getMethod( void ) const;
	std::string getProtocol( void ) const;

	bool isCgiEnabled( void ) const;

	std::map<std::string, std::string> getHeaders( void ) const;
private:
	int	_statusCode;

	std::string _url;
	std::string _refer;
	std::string _method;
	std::string _protocol;

	bool _cgiEnabled;

	std::string _body;

	std::map<std::string, std::string> _headers;

	//bool _cgiFormatted;
	bool _formatHeader( const std::string &headerLine );
	//bool _handleFileRequest( const std::string root, const std::string &filePath );
};
