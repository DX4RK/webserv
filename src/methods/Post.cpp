#include "Post.hpp"

Post::Post(void) {}
Post::Post(Request &request, Config *config) {
	this->_returnCode = 0;
	this->server_config = config;

	if (this->_isCgiRequest(request)) {
		this->_handleCgiRequest(request);
		return;
	}

	this->_cgiResponse = true; // TO REMOVE
	const std::string root_page = this->server_config->getLocationRoot("/");

	if (this->_handleFileUrl(request, root_page)) return;
}
Post::~Post(void) {}

void Post::_handleCgiRequest(Request &request) {
	try {
		std::string url = request.getUrl();
<<<<<<< HEAD
		std::string scriptPath = "";
		std::string postData = "";

		if (url == "/login/standard") {
			scriptPath = "./www/cgi-bin/login.py";
			postData = "type=standard&" + request.getBody();
		}
		else if (url == "/forum/post") {
			std::string session_user = this->_getSessionUser(request);

			if (session_user.empty()) {
				this->_content = "{\"success\": false, \"error\": \"Not logged in\"}";
				this->_returnCode = 401;
				return;
			}

			scriptPath = "./www/cgi-bin/forum.py";
			postData = "type=forum&user=" + session_user + "&" + request.getBody();
		}
		else if (url.find("/cgi-bin/") == 0) {
			scriptPath = "./www" + url;
			postData = request.getBody();
		}
		else {
=======
		
		// Trouver le location config correspondant à cette URL
		location_config* location = this->server_config->findLocationForPath(url);
		if (!location) {
>>>>>>> 175f051 (return json for script python, fix read picture conversion to binary, add condition to post et request for multipart  if binary)
			this->_returnCode = 404;
			return;
		}
		
		// Construire le script path en utilisant le root du location
		std::string scriptPath = this->server_config->getCgiScriptPath(url);
		std::string postData = request.getBody();

		if (!fileExists(scriptPath)) {
			this->_returnCode = 404;
			return;
		}

		if (!hasReadPermission(scriptPath)) {
			this->_returnCode = 403;
			return;
		}

		this->_executeCgiScript(request, scriptPath, postData);

	} catch (std::exception &e) {
		this->_content = "{\"success\": false, \"error\": \"CGI execution error\"}";
		this->_returnCode = 500;
	}
}

void Post::_executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData) {
	std::map<std::string, std::string> headers = request.getHeaders();
	
	// Utiliser le vrai Content-Type de la requête au lieu de le forcer
	std::string contentType = "application/x-www-form-urlencoded"; // default
	std::map<std::string, std::string>::iterator ctIt;
	for (ctIt = headers.begin(); ctIt != headers.end(); ++ctIt) {
		if (ctIt->first == "Content-Type") {
			contentType = ctIt->second;
			break;
		}
	}
	
	std::map<std::string, std::string> cgiHeaders;
	cgiHeaders["Content-Type"] = contentType;
	cgiHeaders["Content-Length"] = ft_itoa(postData.length());

	CGI cgi_handler("POST", request.getProtocol(), cgiHeaders, 8080);
	cgi_handler.setEnvironment(scriptPath, *this->server_config);
	cgi_handler._addEnv("CONTENT_LENGTH", ft_itoa(postData.length()));
	cgi_handler.formatEnvironment();
	this->_content = cgi_handler.execute(postData);

	if (scriptPath.find(".py") != std::string::npos)
		this->_returnCode = 200;
}

bool Post::_isCgiRequest(Request &request) {
	std::string url = request.getUrl();

<<<<<<< HEAD
	if (url == "/login/standard" || url == "/forum/post") {
		return true;
	}

	if (url.find("/cgi-bin/") == 0) {
		return true;
=======
	std::vector<std::string> cgiExtensions = this->server_config->getCgiExtensions();
	for (size_t i = 0; i < cgiExtensions.size(); i++) {
		if (url.find(cgiExtensions[i]) != std::string::npos) {
			return true;
		}
>>>>>>> 175f051 (return json for script python, fix read picture conversion to binary, add condition to post et request for multipart  if binary)
	}

	return false;
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
	if (this->_isCgiRequest(request)) {
		std::string url = request.getUrl();
<<<<<<< HEAD

		if (url == "/login/standard") {
			response.addHeader("Location", "/");
			return;
		}

		if (url == "/forum/post") {
			response.addHeader("Content-Type", "application/json");
			return;
		}

=======
		
		// Déterminer le Content-Type basé sur l'extension
>>>>>>> 175f051 (return json for script python, fix read picture conversion to binary, add condition to post et request for multipart  if binary)
		if (url.find(".py") != std::string::npos) {
			response.addHeader("Content-Type", "application/json");
		} else {
			response.addHeader("Content-Type", "text/html");
		}

		return;
	}

	if (this->_returnCode != 0) return;
	if (this->_returnCode == 0) { this->_returnCode = 200; }

	if (request.isCgiEnabled()) {
		std::string url = request.getUrl();
		std::string scriptPath = this->server_config->getCgiScriptPath(url);
		
		CGI cgi_handler(request.getMethod(), request.getProtocol(), request.getHeaders(), 8080);
		cgi_handler.setEnvironment(scriptPath, *this->server_config);
		cgi_handler.formatEnvironment();
		this->_content = cgi_handler.execute(request.getBody());
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
