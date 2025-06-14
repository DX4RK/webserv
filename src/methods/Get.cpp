#include "Get.hpp"

Get::Get(void) {}
Get::Get(Request &request, Config *config) {
	this->_returnCode = 0;
	this->server_config = config;

	if (this->_isCgiRequest(request)) {
		this->_handleCgiRequest(request);
		return;
	}

	const std::string root_page = this->server_config->getLocationRoot("/");

	bool canProcess = this->_handleFileUrl(request, root_page);

	if (!canProcess) { return; }
	this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
}
Get::~Get(void) {}

void Get::_handleCgiRequest(Request &request) {
	try {
		std::string url = request.getUrl();

		std::string scriptPath = this->server_config->getCgiScriptPath(url);
		
		if (scriptPath.empty()) {
			this->_returnCode = 404;
			return;
		}

		if (!fileExists(scriptPath)) {
			this->_returnCode = 404;
			return;
		}

		if (!hasReadPermission(scriptPath)) {
			this->_returnCode = 403;
			return;
		}

		std::string postData = "";

		this->_executeCgiScript(request, scriptPath, postData);

	} catch (std::exception &e) {
		this->_content = "{\"success\": false, \"error\": \"CGI execution error\"}";
		this->_returnCode = 500;
	}
}

void Get::_executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData) {
	std::map<std::string, std::string> headers;

	headers["Content-Type"] = this->_getContentTypeFromScript(scriptPath);
	
	headers["Content-Length"] = ft_itoa(postData.length());

	std::string method = postData.empty() ? request.getMethod() : "POST";

	CGI cgi_handler(method, request.getProtocol(), headers, 8080);
	cgi_handler.setEnvironment(scriptPath, *this->server_config);
	
	if (!postData.empty()) {
		cgi_handler._addEnv("CONTENT_LENGTH", ft_itoa(postData.length()));
	}
	
	cgi_handler.formatEnvironment();
	this->_content = cgi_handler.execute(postData);

	this->_returnCode = 200;
}

bool Get::_isCgiRequest(Request &request) {
	std::string url = request.getUrl();

	return this->server_config->isCgiPath(url);
}

void Get::process(Response &response, Request &request) {
	(void)request;

	if (this->_isCgiRequest(request)) {
		std::string url = request.getUrl();
		std::string scriptPath = this->server_config->getCgiScriptPath(url);
		response.addHeader("Content-Type", this->_getContentTypeFromScript(scriptPath));
		return;
	}

	if (!this->_content.empty() && this->_returnCode == 200) {
		response.addHeader("Content-Type", "text/html");
		response.addHeader("Content-Length", ft_itoa(this->_content.length()));
		return;
	}

	if (this->_returnCode == 404 && (getLastSub(this->_fileName, '.') == this->_fileName || this->_fileName.find(".html"))) {
		this->_fileName = "404";
		this->_filePath = "./src/_default/404.html";
		this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
	} else if (this->_returnCode != 0) { return; }

	char buffer[4096];
	ssize_t bytesRead;
	std::string fileContent;

	while ((bytesRead = read(this->_fileFd, buffer, sizeof(buffer))) > 0) {
		fileContent.append(buffer, bytesRead);
	}

	close(this->_fileFd);

	if (bytesRead < 0) {
		this->_returnCode = 500;
		return;
	}

	this->_content = fileContent;

	if (this->_returnCode == 0) { this->_returnCode = 200; }

	response.addHeader("Content-Type", this->server_config->getContentType(this->_filePath));
	response.addHeader("Content-Length", ft_itoa(this->_content.size()));
	response.addHeader("Last-Modified", getFileModifiedTime(this->_filePath));
}

bool Get::_handleFileUrl(Request &request, const std::string root) {
	const std::string &filePath = request.getUrl();
	std::string fullPath = root + filePath;
	std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Referer");

	if (it != request.getHeaders().end()) {
		std::string path = extractPath(request.getHeaders()["Referer"]);
		fullPath = WEB_ROOT + trim(path, false) + trim(filePath, false);
	}

	this->_fileName = getLastSub(fullPath, '/');

	if (!fileExists(fullPath)) {
		this->_returnCode = 404;
		return (false);
	}

	if (isDirectory(fullPath)) {
		fullPath += "/index.html";
		this->_fileName = "index";
		if (!fileExists(fullPath)) {
			this->_returnCode = 403;
			return (false);
		}
	}

	if (!hasReadPermission(fullPath)) {
		this->_returnCode = 403;
		return (false);
	}

	this->_filePath = fullPath;
	return (true);
}

std::string Get::_getContentTypeFromScript(const std::string& scriptPath) const {
	size_t dotPos = scriptPath.find_last_of('.');
	if (dotPos != std::string::npos) {
		std::string extension = scriptPath.substr(dotPos);
		if (extension == ".py" || extension == ".sh") {
			return "application/json";
		} 
	}
	return "text/html";
}