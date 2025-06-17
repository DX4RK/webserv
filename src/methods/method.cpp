#include "method.hpp"

Method::Method(void) {}
Method::~Method(void) {}

//Method::Method(Request &request, Config *config) {
//	this->_returnCode = 0;
//	this->server_config = config;
//	(void)request;
//}

int Method::getReturnCode(void) { return this->_returnCode; }
std::string Method::getContent(void) { return this->_content; }
bool Method::isCgiResponse(void) { return this->_cgiResponse; }
//std::string Method::getContentType(void) { return this->_contentType; }
//long		Method::getContentLength(void) { return this->_contentLength; }
