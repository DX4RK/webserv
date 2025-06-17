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
		// Gestion GitHub login
		if (request.getUrl() == "/login/github") {
			this->_responseCode = 302;
			std::string redirectUrl = "https://github.com/login/oauth/authorize?client_id=Ov23liqR1ibSAhoNpfGM&redirect_uri=http://localhost:8080/github/callback&scope=user";
			this->addHeader("Location", redirectUrl);
			methodContent = "";
		}
		// Gestion callback GitHub
		else if (request.getUrl().find("/github/callback") == 0) {
			this->_handleGithubCallback(request);
			this->_responseCode = 302;
			this->addHeader("Location", "/");
			methodContent = "";
		}
		else {
			Method methodResult = this->_processRequest(request.getMethod(), request);
			this->_responseCode = methodResult.getReturnCode();

			if (this->_responseCode == 0) { this->_responseCode = 500; std::cout << "yes" << std::endl; return; }

			CGI_response = methodResult.isCgiResponse();
			methodContent = methodResult.getContent();
		}
	}

	std::string responseCode = ft_itoa(this->_responseCode);
	std::string responseMessage = this->server_config->getStatusCode(responseCode);

	this->_response = request.getProtocol() + " " + responseCode + " " + responseMessage + "\n";

	if (CGI_response) {
		std::cout << "yes" << std::endl;
		if (methodContent.find("Content-Type:") == 0 || methodContent.find("Content-type:") == 0)
			this->_response += this->_headers + methodContent;
	} else {
		this->_response += this->_headers + "\n" + methodContent;
	}
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
			cgi_handler.setEnvironment("./www/cgi-bin/login.py", *this->server_config);
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
