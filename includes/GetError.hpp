#pragma once

#include "_libs.hpp"

#include "utils.hpp"
#include "method.hpp"
#include "request.hpp"
#include "CGI.hpp"

class Response;
class Request;

class GetError : public Method {
public:
	GetError( Request &request, Config *config, std::string filePath );
	~GetError( void );

	void process( Response &response, Request &request );
private:
	int _fileFd;

	std::string _fileName;
	std::string _filePath;

	bool _isCgiRequest(Request &request);
	void _handleCgiRequest(Request &request);
	void _executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData);
	std::string _GetErrorContentTypeFromScript(const std::string& scriptPath) const;
	
	bool _handleFileUrl(Request &request, const std::string root);

	GetError( void );
};