#pragma once

#include <iostream>

class Request;

class Method {
public:
	Method( void );
	Method( Request &rq );
	virtual ~Method( void );

	std::string getContent( void );
	std::string getContentType( void );
	long 		getContentLength( void );
protected:
	std::string _content;
	std::string	_contentType;
	long		_contentLength;
};
