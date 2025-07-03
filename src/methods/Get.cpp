#include "Get.hpp"

Get::Get(void) {}
Get::Get(Request &request, Config *config) {
	this->_returnCode = 0;
	this->server_config = config;

	const std::string root_page = this->server_config->getLocationRoot("/");

	if (!this->_isCgiRequest(request) && !config->listLocation(request.getLocation())) {
		bool canProcess = this->_handleFileUrl(request, root_page);
		if (!canProcess) { return; }
		std::cout << this->_filePath.c_str() << std::endl;
		this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
	}
}
Get::~Get(void) {}

void Get::_executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData) {
	std::map<std::string, std::string> headers;

	CGI cgi_handler(request.getMethod(), request.getProtocol(), headers, 8080);
	cgi_handler.setEnvironment(scriptPath, request.getLocation(), *this->server_config);
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

	if (this->server_config->listLocation(request.getLocation())) {
		this->_executeCgiScript(request, LISTING_CGI, "");
		this->_cgiResponse = true;
		return;
	}

	if (this->_isCgiRequest(request)) {
		this->_handleCgiRequest(request);
		this->_cgiResponse = true;
		return;
	}

	if (!this->_content.empty() && this->_returnCode == 200) {
		response.addHeader("Content-Type", "text/html");
		response.addHeader("Content-Length", ft_itoa(this->_content.length()));
		return;
	}

	if (this->_returnCode == 404 && (getLastSub(this->_fileName, '.') == this->_fileName || this->_fileName.find(".html"))) {
		this->_fileName = "404";
		this->_filePath = "./src/_default/errors/404.html";
		this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
	} else if (this->_returnCode != 0) { return; }

	char buffer[4096];
	ssize_t bytesRead;
	std::string fileContent;

	while ((bytesRead = read(this->_fileFd, buffer, sizeof(buffer))) > 0)
		fileContent.append(buffer, bytesRead);

	close(this->_fileFd);

	if (bytesRead < 0) {
		std::cout << "tyufk" << std::endl;
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
		return (false);
	}

	if (isDirectory(path)) {
		std::vector<std::string> indexes = this->server_config->getLocationIndex(request.getLocation());

		for (size_t i = 0; i < indexes.size(); i++) {
			std::string testPath = path + indexes.at(i);

			if (path.at(path.length() - 1) != '/')
				testPath = path + '/' + indexes.at(i);

			std::cout << testPath << std::endl;

			if (fileExists(testPath)) {
				this->_returnCode = 0;
				this->_fileName = getFileName(indexes.at(i));

				path = testPath;
				std::cout << testPath << std::endl;
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
		std::cout << "yes" << std::endl;
		this->_returnCode = 500;
	}
}
