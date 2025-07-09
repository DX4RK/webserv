#include "GetError.hpp"

GetError::GetError(void) {}
GetError::GetError(Request &request, Config *config, std::string filePath) {
	std::cout << "yes" << std::endl;
	this->_returnCode = 0;
	this->displayErrorPage = false;
	this->server_config = config;
	this->_filePath = filePath;

	const std::string root_page = this->server_config->getLocationRoot("/");

	bool canProcess = this->_handleFileUrl(request, root_page);
	if (!canProcess) { return; }
	this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
}
GetError::~GetError(void) {}

void GetError::process(Response &response, Request &request) {
	(void)request;

	if (this->_returnCode != 0)
		return;
	std::cout << "ee" << std::endl;
	char buffer[4096];
	ssize_t bytesRead;
	std::string fileContent;

	while ((bytesRead = read(this->_fileFd, buffer, sizeof(buffer))) > 0)
		fileContent.append(buffer, bytesRead);

	close(this->_fileFd);

	if (bytesRead < 0) {
		this->_returnCode = 1;
		return;
	}

	this->_content = fileContent;

	if (this->_returnCode == 0) { this->_returnCode = 200; }

	response.addHeader("Content-Type", this->server_config->getContentType(this->_filePath));
	response.addHeader("Content-Length", ft_itoa(this->_content.size()));
	response.addHeader("Last-Modified", getFileModifiedTime(this->_filePath));
}

bool GetError::_handleFileUrl(Request &request, const std::string root) {
	(void)root;
	(void)request;
	if (!fileExists(this->_filePath)) {
		this->_returnCode = 1;
		return (false);
	}

	if (isDirectory(this->_filePath)) {
		this->_returnCode = 1;
		return (false);
	}

	if (!hasReadPermission(this->_filePath)) {
		this->_returnCode = 1;
		return (false);
	}
	return (true);
}