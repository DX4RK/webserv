#pragma once

#include "_libs.hpp"

#include "config.hpp"
#include "response.hpp"
#include "listenSocket.hpp"

class ListenSocket;

class Request {
public:
	Request( ListenSocket &listener, Config *config, int errorCode );

	int getStatusCode( void ) const;
	int getServerPort( void ) const;

	std::string getUrl( void ) const;
	std::string getOriginalUrl(void) const;
	std::string getPath( void ) const;
	std::string getLocation( void ) const;
	std::string getBody( void ) const;
	std::string getMethod( void ) const;
	std::string getCgiExtension( void ) const;
	std::string getProtocol( void ) const;
	std::string getFileName( void ) const;

	bool isCgiEnabled( void ) const;
	bool isReqDirectory( void ) const;
	std::string findHeader( std::string index );

	std::map<std::string, std::string> getHeaders( void ) const;
private:
	int _currentPort;
	int	_statusCode;
	bool _cgiEnabled;
	bool _isDirectory;

	std::string _url;
	std::string _path;
	std::string _fileName;
	std::string _location;
	std::string _cgiExtension;
	std::string _originalUrl;

	std::string _refer;
	std::string _body;
	std::string _method;
	std::string _protocol;

	std::map<std::string, std::string> _headers;

	Config *server_config;

	bool _formatHeader( const std::string &headerLine );

	Request( void );
};
