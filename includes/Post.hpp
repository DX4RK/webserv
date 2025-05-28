#pragma once

#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.hpp"
#include "method.hpp"
#include "request.hpp"

class Response;
class Request;

class Post : public Method {
public:
	Post( Request &request, Config &server_config );
	~Post( void );

	void process( Response &response, Request &request, Config &server_config );
private:
	int _fileFd;

	std::string _fileName;
	std::string _filePath;

	bool _handleFileUrl(Request &request, const std::string root);

	Post( void );
};
