#pragma once

#include "_libs.hpp"

#include "utils.hpp"
#include "method.hpp"
#include "request.hpp"
#include "response.hpp"

class Response;

class Post : public Method {
public:
	Post( void );
	Post( Request &request, Config *config );
	~Post( void );

	void process( Response &response, Request &request );
private:
	int _fileFd;

	std::string _fileName;
	std::string _filePath;

	bool _handleFileUrl(Request &request, const std::string root);
};
