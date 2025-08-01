#include "Post.hpp"

Post::Post(void) {}
Post::Post(Request &request, Config *config) {
	this->_returnCode = 0;
	this->displayErrorPage = false;
	this->_cgiResponse = false;
	this->server_config = config;

	if (this->_isCgiRequest(request)) {
		this->_handleCgiRequest(request);
		return;
	}

	this->_cgiResponse = true;
	const std::string root_page = this->server_config->getLocationRoot("/");

	if (this->_handleFileUrl(request, root_page)) return;
}
Post::~Post(void) {}

void Post::_handleCgiRequest(Request &request) {
	try {
		std::string url = request.getUrl();

		try {
			location_config location = this->server_config->getLocationFromPath(url);

			std::string scriptPath = request.getPath();
			//std::string scriptPath = this->server_config->getCgiScriptPath(url);
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
				this->_returnCode = 404;
				return;
			}
	} catch (std::exception &e) {
		this->_content = "{\"success\": false, \"error\": \"CGI execution error\"}";
		this->_returnCode = 500;
	}
}

void Post::_executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData) {
	std::map<std::string, std::string> headers = request.getHeaders();

	std::string contentType = "application/x-www-form-urlencoded";
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

	std::cout << "get:" << std::endl;
	std::string executorPath = this->server_config->getCGIPath(request.getLocation(), request.getCgiExtension());
	std::cout << "Executor Path: " << executorPath << std::endl;
	CGI cgi_handler("POST", request.getProtocol(), cgiHeaders, 8080);
	cgi_handler.setEnvironment(scriptPath, executorPath, request.getLocation(), *this->server_config);
	cgi_handler._addEnv("CONTENT_LENGTH", ft_itoa(postData.length()));

	std::string uploadDir = this->server_config->getUploadStore(request.getLocation());
	cgi_handler._addEnv("UPLOAD_DIR", uploadDir);
	std::cout << "jjiohoh" << std::endl;

	cgi_handler.formatEnvironment();
	std::string cgiOutput = cgi_handler.execute(postData);
	std::cout << "eeheheh" << std::endl;
	size_t headerEndPos = cgiOutput.find("\r\n\r\n");
	if (headerEndPos == std::string::npos) {
		headerEndPos = cgiOutput.find("\n\n");
		if (headerEndPos != std::string::npos) {
			headerEndPos += 2;
		}
	} else {
		headerEndPos += 4;
	}

	if (headerEndPos != std::string::npos) {
		this->_content = cgiOutput.substr(headerEndPos);
	} else {
		this->_content = cgiOutput;
	}

	//if (scriptPath.find(".py") != std::string::npos)
	std::cout << "FLLF" << std::endl;
	this->_returnCode = 200;
}

bool Post::_isCgiRequest(Request &request) {
		//std::string url = request.getUrl();
		//return this->server_config->isCgiPath(url);
	return request.isCgiEnabled();
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

		// Déterminer le Content-Type basé sur l'extension
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
		std::string scriptPath = request.getPath();
		//std::string scriptPath = this->server_config->getCgiScriptPath(url);

		std::string executorPath = this->server_config->getCGIPath(request.getLocation(), request.getCgiExtension());

		CGI cgi_handler(request.getMethod(), request.getProtocol(), request.getHeaders(), 8080);
		cgi_handler.setEnvironment(scriptPath, executorPath, request.getLocation(), *this->server_config);
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
