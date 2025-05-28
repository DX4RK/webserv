#pragma once

#include <iostream>

#include "config.hpp"

class Request;

class Method {
public:
	Method( void );
	Method( Request &request, Config &server_config );
	virtual ~Method( void );

	int getReturnCode( void );
	std::string getContent( void );
	//std::string getContentType( void );
	//long 		getContentLength( void );
protected:
	int _returnCode;

	std::string _content;
	std::string	_contentType;
	long		_contentLength;
};
