#include "method.hpp"

Method::Method(void) {}
Method::~Method(void) {}

Method::Method(Request &rq) { (void)rq; }

std::string Method::getContent(void) { return this->_content; }
//std::string Method::getContentType(void) { return this->_contentType; }
//long		Method::getContentLength(void) { return this->_contentLength; }
