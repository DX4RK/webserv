#include "method.hpp"

Method::Method(void) {}
Method::~Method(void) {}

Method::Method(Request &request, Config &server_config) {
	this->_returnCode = 0;
	(void)request; (void)server_config;
}

int Method::getReturnCode( void ) { return this->_returnCode; }
std::string Method::getContent(void) { return this->_content; }
//std::string Method::getContentType(void) { return this->_contentType; }
//long		Method::getContentLength(void) { return this->_contentLength; }
