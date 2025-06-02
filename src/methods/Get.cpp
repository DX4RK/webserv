#include "Get.hpp"

Get::Get(void) {}
Get::Get(Request &request, Config *config) {
	this->_returnCode = 0;
	this->server_config = config;

	const std::string root_page = this->server_config->getLocationRoot("/");

	bool canProcess = this->_handleFileUrl(request, root_page);

	if (!canProcess) { return; }
	this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
}
Get::~Get(void) {}

void Get::process(Response &response, Request &request) {
	(void)request;

	if (this->_returnCode == 404 && (getLastSub(this->_fileName, '.') == this->_fileName || this->_fileName.find(".html"))) {
		this->_fileName = "404";
		this->_filePath = "./src/_default/404.html";
		this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
	} else if (this->_returnCode != 0) { return; }

	char tempBuffer[30000] = {0};
	ssize_t bytesRead = read(this->_fileFd, tempBuffer, 30000);

	if (bytesRead < 0) {
		close(this->_fileFd);
		this->_returnCode = 500;
		return;
	}

	this->_content = std::string(tempBuffer, bytesRead);
	close(this->_fileFd);
	if (this->_returnCode == 0) { this->_returnCode = 200; }

	response.addHeader("Content-Type", this->server_config->getContentType(this->_filePath));
	response.addHeader("Content-Length", ft_itoa(bytesRead));
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

	// Check if it’s a directory
	if (isDirectory(fullPath)) {
		fullPath += "/index.html"; // Try to resolve to index.html
		this->_fileName = "index";
		if (!fileExists(fullPath)) {
			this->_returnCode = 403;
			return (false);
		}
	}

	// Check read permissions
	if (!hasReadPermission(fullPath)) {
		this->_returnCode = 403;
		return (false);
	}

	this->_filePath = fullPath;
	return (true);
}
