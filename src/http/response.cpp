#include "response.hpp"
#include "Put.hpp"

Response::Response(void) {}
Response::Response(Request &request, Config *config) {
	this->server_config = config;
	this->_responseCode = request.getStatusCode();

	bool CGI_response = false;

	std::string methodHeaders = "";
	std::string methodContent = "";

	this->addHeader("Server", "webserv/1.0");
	this->addHeader("Date", getTime());

	if (this->_responseCode == 0) {
		location_config loc = config->getLocationFromPath(request.getLocation());
		if (loc.redirect_code != 0) {
			this->_responseCode = loc.redirect_code;
			this->addHeader("Location", loc.redirect_url);
			this->_response = request.getProtocol() + " " +
				ft_itoa(this->_responseCode) + " " +
			this->server_config->getStatusCode(ft_itoa(this->_responseCode)) + "\n";
			this->_response += this->_headers + "\n";
			return;
		}

		Method methodResult = this->_processRequest(request.getMethod(), request);
		this->_responseCode = methodResult.getReturnCode();
		if (this->_responseCode == 0) { this->_responseCode = 500; return; }

		CGI_response = methodResult.isCgiResponse();
		methodContent = methodResult.getContent();

		if (methodResult.displayErrorPage) {
			std::string path = this->server_config->getErrorPath(methodResult.getReturnCode());
			GetError errorResult = GetError(request, this->server_config, path);
			errorResult.process(*this, request);

			if (errorResult.getReturnCode() == 200) {
				CGI_response = false;
				methodContent = errorResult.getContent();
			}
		}
	} else if (this->_responseCode >= 300 && 400 > this->_responseCode) {
		this->addHeader("Location", this->server_config->getRedirectPath(request.getLocation()));
	} else {
		std::string path = this->server_config->getErrorPath(this->_responseCode);
		GetError errorResult = GetError(request, this->server_config, path);
		errorResult.process(*this, request);
		if (errorResult.getReturnCode() == 200) {
			CGI_response = false;
			methodContent = errorResult.getContent();
		}
	}
	
	std::string responseCode = ft_itoa(this->_responseCode);
	std::string responseMessage = this->server_config->getStatusCode(responseCode);

	this->_response = request.getProtocol() + " " + responseCode + " " + responseMessage + "\n";

	(void)CGI_response;
	//if (CGI_response)
	//{
	//	this->addHeader("Content-type", "text/plain");
	//	this->addHeader("Content-Length", ft_itoa(methodContent.size()));
	//}
	this->_response += this->_headers + "\n" + methodContent;
}

Response::~Response( void ) {
	this->_response = "";
}

Method Response::_processRequest(std::string method, Request &request) {
	if (method.compare("GET") == 0) {
		Get methodResult(request, server_config);
		methodResult.process(*this, request);
		return methodResult;
	} else if (method.compare("POST") == 0) {
		Post methodResult(request, server_config);
		methodResult.process(*this, request);
		return methodResult;
	} else if (method.compare("PUT") == 0) {
		Put methodResult(request, server_config);
		methodResult.process(*this, request);
		return methodResult;
	} else if (method.compare("DELETE") == 0) {
		Delete methodResult(request, server_config);
		methodResult.process(*this, request);
		return methodResult;
	}

	Method methodResult;
	return methodResult;
}

void Response::addHeader(std::string headerName, std::string headerValue) {
	this->_headers += trim(headerName, false) + ": " + trim(headerValue, false) + "\n";
}

/* GETTERS & SETTERS */

int Response::getResponseCode(void) const { return (this->_responseCode); }
std::string Response::getResponse(void) const { return (this->_response); }
