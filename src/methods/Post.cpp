#include "Post.hpp"

Post::Post(void) {}
Post::Post(Request &request, Config *config) {
	this->_returnCode = 0;
	this->server_config = config;

	// Gestion login standard
	if (request.getUrl() == "/login/standard") {
		this->_handleStandardLogin(request);
		return;
	}

	//  Gestion forum post
	if (request.getUrl() == "/forum/post") {
		this->_handleForumPost(request);
		return;
	}

	const std::string root_page = this->server_config->getLocationRoot("/");

	if (this->_handleFileUrl(request, root_page)) return;
}
Post::~Post(void) {}

//Fonction pour login standard
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

// AJOUT : Fonction pour forum post
void Post::_handleForumPost(Request &request) {
	try {
		// Vérifier si l'utilisateur est connecté
		std::string session_user = this->_getSessionUser(request);
		
		if (session_user.empty()) {
			// Pas connecté
			this->_content = "{\"success\": false, \"error\": \"Not logged in\"}";
			this->_returnCode = 401;
			return;
		}
		
		// Utilisateur connecté, traiter le post
		std::string body = request.getBody();
		std::string postData = "type=forum&user=" + session_user + "&" + body;
		
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = "application/x-www-form-urlencoded";
		headers["Content-Length"] = ft_itoa(postData.length());
		
		CGI cgi_handler("POST", "HTTP/1.1", headers);
		cgi_handler.setEnvironment("./www/cgi-bin/forum.py", *this->server_config);
		cgi_handler._addEnv("CONTENT_LENGTH", ft_itoa(postData.length()));
		cgi_handler.formatEnvironment();
		cgi_handler.execute(postData);
		
		this->_returnCode = 200;
		
	} catch (std::exception &e) {
		this->_content = "{\"success\": false, \"error\": \"Server error\"}";
		this->_returnCode = 500;
	}
}

//  Fonction pour récupérer l'utilisateur de session
std::string Post::_getSessionUser(Request &request) {
	std::map<std::string, std::string> headers = request.getHeaders();
	std::string cookie_header = "";
	
	// Chercher le header Cookie
	std::map<std::string, std::string>::iterator it;
	for (it = headers.begin(); it != headers.end(); ++it) {
		if (it->first == "Cookie") {
			cookie_header = it->second;
			break;
		}
	}
	
	if (cookie_header.empty()) return "";
	
	// Parser le cookie session_user=username
	size_t pos = cookie_header.find("session_user=");
	if (pos == std::string::npos) return "";
	
	size_t start = pos + 13; // longueur de "session_user="
	size_t end = cookie_header.find(";", start);
	if (end == std::string::npos) end = cookie_header.length();
	
	return cookie_header.substr(start, end - start);
}

void Post::process(Response &response, Request &request) {
	if (request.getUrl() == "/login/standard") {
		response.addHeader("Location", "/");
		return;
	}

	if (request.getUrl() == "/forum/post") {
		response.addHeader("Content-Type", "application/json");
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