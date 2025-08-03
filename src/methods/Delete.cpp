#include "Delete.hpp"

Delete::Delete(void) {}
Delete::Delete(Request &request, Config *config) {
	this->_returnCode = 0;
	this->displayErrorPage = false;
	this->_cgiResponse = false;
	this->server_config = config;
	const std::string root_page = this->server_config->getLocationRoot("/");
	if (this->_handleFileUrl(request, root_page)) return;
}
Delete::~Delete(void) {}

void Delete::process(Response &response, Request &request) {
	(void)response;
	(void)request;
	(void)server_config;

	if (this->_returnCode != 0)
		return;

	if (std::remove(this->_filePath.c_str()) != 0) {
		this->_returnCode = 501;
	} else {
		this->_returnCode = 200;
	}
}


bool Delete::_handleFileUrl(Request &request, const std::string root) {
	(void)root;

	std::string path = request.getPath();

	this->_fileName = getLastSub(path, '/');

	T_PATH_PARSING path_permissions = parse_path(path);

	if (!path_permissions.exist) {
		this->_returnCode = 404;
		return (false);
	} else if (!path_permissions.can_write) {
		this->_returnCode = 403;
	}

	this->_filePath = path;
	return (true);
}
