#pragma once

#include "_libs.hpp"

#include "CGI.hpp"
#include "config.hpp"

class Request;

class Method {
public:
	Method( void );
	virtual ~Method( void );

	int getReturnCode( void );
	std::string getContent( void );
	bool isCgiResponse( void );

	bool displayErrorPage;
protected:
	int _returnCode;
	bool _cgiResponse;

	std::string _content;
	std::string	_contentType;
	long		_contentLength;

	Config *server_config;
};
