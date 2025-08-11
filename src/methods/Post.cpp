#include "Post.hpp"

Post::Post(void) {}
Post::Post(Request &request, Config *config) {
	this->_returnCode = 0;
	this->displayErrorPage = false;
	this->_cgiResponse = false;
	this->server_config = config;

	if (!this->_isCgiRequest(request)) {
		this->_returnCode = 406;
		return;
	}
	this->_handleFileUrl(request);
	return;
}

Post::~Post(void) {}

bool Post::_isCgiRequest(Request &request) {
	return request.isCgiEnabled();
}

void Post::process(Response &response, Request &request) {
	if (this->_returnCode != 0)
		return;

	try {
		std::string url = request.getUrl();
		std::string scriptPath = request.getPath();
		if (scriptPath.empty()) {
			this->_returnCode = 404;
			return;
		}
		std::string postData = request.getBody();
		this->_executeCgiScript(request, scriptPath, postData);
	} catch (std::exception &e) {
		this->_content = "{\"success\": false, \"error\": \"CGI execution error\"}";
		this->_returnCode = 500;
	}
	this->_checkCgiResponse(response);
	return;
}

bool Post::_handleFileUrl(Request &request) {
	std::string path = request.getPath();

	this->_fileName = getLastSub(path, '/');

	if (!fileExists(path)) { this->_returnCode = 404; return (false); }
	if (isDirectory(path)) { this->_returnCode = 405; return (false); }
	if (!hasReadPermission(path)) { this->_returnCode = 403; return (false); }

	this->_filePath = path;
	return (true);
}

void Post::_checkCgiResponse(Response &response) {
	std::string cgiContent = this->_content;
	size_t headerEnd = cgiContent.find("\r\n\r\n");
	size_t headerLen = 4;
	if (headerEnd == std::string::npos) {
		headerEnd = cgiContent.find("\n\n");
		headerLen = 2;
	}
	std::string headers, body;
	if (headerEnd != std::string::npos) {
		headers = cgiContent.substr(0, headerEnd);
		body = cgiContent.substr(headerEnd + headerLen);
	} else {
		headers = "";
		body = cgiContent;
	}

	bool hasContentType = false;
	bool hasContentLength = false;
	bool hasStatus = false;
	int statusCode = 200;

	std::istringstream headerStream(headers);
	std::string line;
	while (std::getline(headerStream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		size_t colon = line.find(":");
		if (colon == std::string::npos)
			continue;
		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		if (key == "Content-Type") {
			response.addHeader("Content-Type", value);
			hasContentType = true;
		} else if (key == "Content-Length") {
			response.addHeader("Content-Length", value);
			hasContentLength = true;
		} else if (key == "Status") {
			statusCode = ft_atoi(value);
			hasStatus = true;
		} else {
			response.addHeader(key, value);
		}
	}
	if (!hasContentType)
		response.addHeader("Content-Type", "application/json");
	if (!hasContentLength)
		response.addHeader("Content-Length", ft_itoa(body.length()));
	(void)hasStatus;
	this->_returnCode = statusCode;
	this->_content = body;
}

void Post::_executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData) {
	std::string uploadDir = this->server_config->getUploadStore(request.getLocation());
	std::map<std::string, std::string> headers = request.getHeaders();
	std::string contentType = "application/x-www-form-urlencoded";
	std::map<std::string, std::string>::iterator ctIt;

	for (ctIt = headers.begin(); ctIt != headers.end(); ++ctIt) {
		if (ctIt->first == "Content-Type") {
			contentType = ctIt->second;
			break;
		}
	}

	headers["Content-Type"] = contentType;
	headers["Content-Length"] = ft_itoa(postData.length());

	std::string executorPath = this->server_config->getCGIPath(request.getLocation(), request.getCgiExtension());
	CGI cgi_handler(request.getMethod(), request.getProtocol(), headers, request.getServerPort());
	cgi_handler.setEnvironment(scriptPath, executorPath, request.getLocation(), *this->server_config);
	cgi_handler._addEnv("UPLOAD_DIR", uploadDir);
	cgi_handler._addEnv("CONTENT_LENGTH", ft_itoa(postData.length()));
	// THIS IS FUCKING STUPID, BUT IT'S HOW IT WORKS WITH TESTER
	cgi_handler._addEnv("PATH_INFO", request.getPathInfo());
	//cgi_handler._addEnv("SCRIPT_NAME", request.getPathInfo());
	cgi_handler._addEnv("REQUEST_URI", request.getPathInfo());
	cgi_handler.formatEnvironment();


	try {
		this->_content = cgi_handler.execute(postData);
	} catch (std::exception &e) {
		this->_content = "{\"success\": false, \"error\": \"CGI execution error\"}";
		this->_returnCode = 500;
		return;
	}

	//size_t headerEndPos = this->_content.find("\r\n\r\n");
	//if (headerEndPos == std::string::npos) {
	//	headerEndPos = this->_content.find("\n\n");
	//	if (headerEndPos != std::string::npos) {
	//		headerEndPos += 2;
	//	}
	//} else
	//	headerEndPos += 4;
	//if (headerEndPos != std::string::npos)
	//	this->_content = this->_content.substr(headerEndPos);
	this->_returnCode = 200;
}
