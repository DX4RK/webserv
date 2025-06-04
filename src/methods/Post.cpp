#include "Post.hpp"

Post::Post(void) {}
Post::Post(Request &request, Config *config) {
	this->_returnCode = 0;
	this->server_config = config;

	//  Gestion login standard
	if (request.getUrl() == "/login/standard") {
		this->_handleStandardLogin(request);
		return;
	}

	const std::string root_page = this->server_config->getLocationRoot("/");

	if (this->_handleFileUrl(request, root_page)) return;
}
Post::~Post(void) {}

// Fonction pour login standard
void Post::_handleStandardLogin(Request &request) {
	try {
		std::string body = request.getBody();
		std::string postData = "type=standard&" + body;
		
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = "application/x-www-form-urlencoded";
		headers["Content-Length"] = ft_itoa(postData.length());
		
		CGI cgi_handler("POST", "HTTP/1.1", headers);
		cgi_handler.setEnvironment("./www/cgi-bin/login.py", *this->server_config);
		cgi_handler._addEnv("CONTENT_LENGTH", ft_itoa(postData.length()));
		cgi_handler.formatEnvironment();
		cgi_handler.execute(postData);
		
		// todo noldiane : notification sucess login ou failed
		// JSON {"success": true/false, "login": "username"}
		// Si login success, ajouter un cookie de session


		this->_returnCode = 302;
	} catch (std::exception &e) {
		// TODO NOLDIANE: Notification d'erreur jsp genre bizarre ?
		this->_returnCode = 302;
	}
}

void Post::process(Response &response, Request &request) {
	// Redirection pour login standard
	if (request.getUrl() == "/login/standard") {
		response.addHeader("Location", "/");
		return;
	}

	(void)response;
	(void)server_config;

	if (this->_returnCode != 0) return;
	if (this->_returnCode == 0) { this->_returnCode = 200; }

	if (request.isCgiEnabled()) {
		CGI cgi_handler(request.getMethod(), request.getProtocol(), request.getHeaders());
		cgi_handler.setEnvironment(this->_filePath, *this->server_config);
		cgi_handler.formatEnvironment();
		cgi_handler.execute(request.getBody());
	}

	return;
}

bool Post::_handleFileUrl(Request &request, const std::string root) {
	const std::string &filePath = request.getUrl();
	std::string fullPath = root + filePath;

	this->_fileName = getLastSub(fullPath, '/');

	if (!fileExists(fullPath)) { this->_returnCode = 404; return (false); }
	if (isDirectory(fullPath)) { this->_returnCode = 405; return (false); }
	if (!hasReadPermission(fullPath)) { this->_returnCode = 403; return (false); }

	this->_filePath = fullPath;
	return (true);
}