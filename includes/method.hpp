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
protected:
	int _returnCode;

	std::string _content;
	std::string	_contentType;
	long		_contentLength;

	Config *server_config;
};
