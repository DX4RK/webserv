#include "Delete.hpp"

Delete::Delete(void) {}
Delete::Delete(Request &request, Config *config) {
	this->_returnCode = 0;
	this->displayErrorPage = false;
	this->server_config = config;
	const std::string root_page = this->server_config->getLocationRoot("/");
	if (this->_handleFileUrl(request, root_page)) return;
}
Delete::~Delete(void) {}

void Delete::process(Response &response, Request &request) {
	(void)response;
	(void)request;
	(void)server_config;

	if (this->_returnCode != 0) return;

	char* const args[] = {
		const_cast<char*>("/bin/rm/"),
		const_cast<char*>(this->_filePath.c_str()),
		NULL
	};

	execve("/bin/rm", args, NULL);
	perror("execve");
	return;
}

bool Delete::_handleFileUrl(Request &request, const std::string root) {
	const std::string &filePath = request.getUrl();
	std::string fullPath = root + filePath;

	this->_fileName = getLastSub(fullPath, '/');
	T_PATH_PARSING path_permissions = parse_path(fullPath);

	if (!path_permissions.exist) {
		this->_returnCode = 404;
		return (false);
	} else if (!path_permissions.can_write) {
		this->_returnCode = 403;
	}

	this->_filePath = fullPath;
	return (true);
}
