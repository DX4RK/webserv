#include "Post.hpp"

Post::Post(void) {}
Post::Post(Request &request, Config &server_config) {
	this->_returnCode = 0;
	const std::string root_page = server_config.getLocationRoot("/");

	if (this->_handleFileUrl(request, root_page)) return;
}
Post::~Post(void) {}

void Post::process(Response &response, Request &request, Config &server_config) {
	(void)request;
	(void)response;
	(void)server_config;

	if (this->_returnCode != 0) return;
	if (this->_returnCode == 0) { this->_returnCode = 200; }

	//char tempBuffer[30000] = {0};
	//ssize_t bytesRead = recv(this., tempBuffer, 30000, 0);
	//if (bytesRead < 0) { make_error("failed to read from socket", EXIT_FAILURE); }

	//tempBuffer[bytesRead] = '\0';
	//std::string buffer = static_cast<std::string>(tempBuffer);



	return;
}

bool Post::_handleFileUrl(Request &request, const std::string root) {
	const std::string &filePath = request.getUrl();
	std::string fullPath = root + filePath;
	//std::map<std::string, std::string>::const_iterator it = request.getHeaders().find("Referer");

	//if (it != request.getHeaders().end()) {
	//	std::string path = extractPath(request.getHeaders()["Referer"]);
	//	fullPath = WEB_ROOT + trim(path, false) + trim(filePath, false);
	//}

	this->_fileName = getLastSub(fullPath, '/');

	std::cout << fullPath << std::endl;
	if (!fileExists(fullPath)) { this->_returnCode = 404; return (false); }
	if (isDirectory(fullPath)) { this->_returnCode = 405; return (false); }
	if (!hasReadPermission(fullPath)) { this->_returnCode = 403; return (false); }

	this->_filePath = fullPath;
	return (true);
}
