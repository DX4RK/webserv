#include "Get.hpp"

Get::Get(void) {}
Get::Get(Request &request, Config *config) {
	this->_returnCode = 0;
	this->displayErrorPage = false;
	this->_cgiResponse = false;
	this->server_config = config;

	const std::string root_page = this->server_config->getLocationRoot("/");

	if (!this->_isCgiRequest(request) && !config->listLocation(request.getLocation(), request.isReqDirectory())) {
		bool canProcess = this->_handleFileUrl(request, root_page);
		if (!canProcess) { return; }
		this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
	}
}
Get::~Get(void) {}

void Get::_executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData) {
	std::map<std::string, std::string> headers;

	std::string executorPath = this->server_config->getCGIPath(request.getLocation(), request.getCgiExtension());
	CGI cgi_handler(request.getMethod(), request.getProtocol(), headers, 8080);
	cgi_handler.setEnvironment(scriptPath, executorPath, request.getLocation(), *this->server_config);
	cgi_handler.formatEnvironment();

	this->_content = cgi_handler.execute(postData);
	this->_returnCode = 200;
}

bool Get::_isCgiRequest(Request &request) {
		//std::string url = request.getUrl();
		//return this->server_config->isCgiPath(url);
	return request.isCgiEnabled();
}

void Get::process(Response &response, Request &request) {
	(void)request;

	if (this->server_config->listLocation(request.getLocation(), request.isReqDirectory())) {
		this->_executeCgiScript(request, LISTING_CGI, "");
		this->_cgiResponse = true;
		return;
	}

	if (this->_isCgiRequest(request)) {
		this->_handleCgiRequest(request);
		this->_cgiResponse = true;
		response.addHeader("Content-Length", ft_itoa(this->_content.size()));
		return;
	}

	if (this->_returnCode != 0) {
		std::string fileExtension = getFileExtension(request.getOriginalUrl());
		if (fileExtension != "") {
			try {
				request.findHeader("Referer");
			} catch (std::exception &e) {
				this->displayErrorPage = true;
			}
		}
		return;
	}

	char buffer[4096];
	ssize_t bytesRead;
	std::string fileContent;

	while ((bytesRead = read(this->_fileFd, buffer, sizeof(buffer))) > 0)
		fileContent.append(buffer, bytesRead);

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
	(void)root;

	std::string path = request.getPath();

	this->_fileName = getLastSub(path, '/');

	if (!fileExists(path)) {
		this->_returnCode = 404;
		std::string fileExtension = getFileExtension(request.getOriginalUrl());
		if (fileExtension == "")
			this->displayErrorPage = true;
		return (false);
	}

	if (isDirectory(path)) {
		this->_returnCode = 404;
		this->displayErrorPage = true;
		std::vector<std::string> indexes = this->server_config->getLocationIndex(request.getLocation());
		for (size_t i = 0; i < indexes.size(); i++) {
			std::string testPath = path + indexes.at(i);
			if (path.at(path.length() - 1) != '/')
				testPath = path + '/' + indexes.at(i);

			if (fileExists(testPath)) {
				this->_returnCode = 0;
				this->_fileName = getFileName(indexes.at(i));

				path = testPath;
				this->_returnCode = 0;
				this->displayErrorPage = false;
				break;
			}
		}
	}

	if (!fileExists(path)) {
		this->_returnCode = 404;
		return (false);
	}

	if (!hasReadPermission(path)) {
		this->_returnCode = 403;
		return (false);
	}

	this->_filePath = path;
	return (true);
}

void Get::_handleCgiRequest(Request &request) {
	try {
		std::string url = request.getUrl();
		std::string scriptPath = request.getPath();
		//std::string scriptPath = this->server_config->getCgiScriptPath(url);
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
