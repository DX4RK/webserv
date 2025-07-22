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
		std::cout << errorResult.getReturnCode() << std::endl;
		if (errorResult.getReturnCode() == 200) {
			CGI_response = false;
			methodContent = errorResult.getContent();
		}
	}

	std::string responseCode = ft_itoa(this->_responseCode);
	std::string responseMessage = this->server_config->getStatusCode(responseCode);

	this->_response = request.getProtocol() + " " + responseCode + " " + responseMessage + "\n";

	if (CGI_response) {
		std::cout << "shouldnt be cgi" << std::endl;
		size_t contentTypePos = methodContent.find("Content-Type: ");
		size_t contentTypeEnd = methodContent.find_first_of('\n');

		if (contentTypePos != std::string::npos) {
			this->addHeader("Content-Type", methodContent.substr(contentTypePos + 14, contentTypeEnd - 1));
			methodContent = methodContent.substr(contentTypeEnd, methodContent.length());
		} else {
			this->addHeader("Content-Type", "application/json");
		}
	}

	this->_response += this->_headers + "\n" + methodContent;
}

Response::~Response( void ) {
	this->_response = "";
}

// Fonction pour gérer le callback GitHub
void Response::_handleGithubCallback(Request &request) {
	std::string url = request.getUrl();
	std::string code = "";

	size_t codePos = url.find("code=");
	if (codePos != std::string::npos) {
		size_t start = codePos + 5;
		size_t end = url.find("&", start);
		if (end == std::string::npos) end = url.length();
		code = url.substr(start, end - start);
	}

	if (!code.empty()) {
		try {
			std::string postData = "type=github&code=" + code;
			std::map<std::string, std::string> headers;
			headers["Content-Type"] = "application/x-www-form-urlencoded";
			headers["Content-Length"] = ft_itoa(postData.length());

			CGI cgi_handler("POST", "HTTP/1.1", headers, 8080);
			cgi_handler.setEnvironment("./www/cgi-bin/login.py", request.getLocation(), *this->server_config);
			cgi_handler._addEnv("CONTENT_LENGTH", ft_itoa(postData.length()));
			cgi_handler.formatEnvironment();
			std::string cgi_output = cgi_handler.execute(postData);  // Capturer la sortie

			// todo noldiane : notification sucess login ou failed
			// JSON {"success": true/false, "login": "username"}
			// Si login success, ajouter un cookie de session
		} catch (std::exception &e) {
			// TODO NOLDIANE: Notification d'erreur general bizarre comme post.cpp
		}
	}
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
