#include "Get.hpp"

Get::Get(void) {}
Get::Get(Request &request, Config *config) {
	this->_returnCode = 0;
	this->displayErrorPage = false;
	this->_cgiResponse = false;
	this->server_config = config;

	if (!this->_isCgiRequest(request) && !config->listLocation(request.getLocation(), request.isReqDirectory())) {
		bool canProcess = this->_handleFileUrl(request);
		if (!canProcess) { return; }
		this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
	}
}
Get::~Get(void) {}

bool Get::_isCgiRequest(Request &request) {
	return request.isCgiEnabled();
}

void Get::process(Response &response, Request &request) {
	if (this->server_config->listLocation(request.getLocation(), request.isReqDirectory())) {
		this->_executeCgiScript(request, LISTING_CGI, "", true);
		this->_cgiResponse = true;
		this->_checkCgiResponse(response);
		return;
	}

	if (this->_isCgiRequest(request)) {
		this->_handleCgiRequest(request);
		this->_cgiResponse = true;
		this->_checkCgiResponse(response);
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

bool Get::_handleFileUrl(Request &request) {
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

void Get::_checkCgiResponse(Response &response) {
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

void Get::_executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData, bool listing) {
	std::map<std::string, std::string> headers;

	std::string location;
	std::string executorPath;
	std::string pathInfo;

	if (listing) {
		//pathInfo = LISTING_CGI;
		pathInfo = request.getPath();
		location = LISTING_CGI_LOCATION;
		executorPath = LISTING_CGI_EXECUTOR;
	} else {
		pathInfo = request.getPathInfo();
		location = request.getLocation();
		executorPath = this->server_config->getCGIPath(request.getLocation(), request.getCgiExtension());
	}

	CGI cgi_handler(request.getMethod(), request.getProtocol(), headers, request.getServerPort());
	cgi_handler.setEnvironment(scriptPath, executorPath, location, *this->server_config);
	// THIS IS FUCKING STUPID, BUT IT'S HOW IT WORKS WITH TESTER
	cgi_handler._addEnv("PATH_INFO", pathInfo);
	cgi_handler.formatEnvironment();

	try {
		this->_content = cgi_handler.execute(postData);
		std::cout << this->_content << std::endl;
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
	//this->_content = this->_content.substr(headerEndPos);
	this->_returnCode = 200;
}

void Get::_handleCgiRequest(Request &request) {
	try {
		std::string url = request.getUrl();
		std::string scriptPath = request.getPath();
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
		this->_executeCgiScript(request, scriptPath, postData, false);
	} catch (std::exception &e) {
		this->_content = "{\"success\": false, \"error\": \"CGI execution error\"}";
		this->_returnCode = 500;
	}
}
