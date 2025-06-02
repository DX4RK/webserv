#pragma once

#include <iostream>

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
