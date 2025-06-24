#include "Put.hpp"
#include <fstream>

Put::Put(void) {}
Put::Put(Request &request, Config *config) {
	this->_returnCode = 0;
	this->server_config = config;

	const std::string root_page = this->server_config->getLocationRoot("/");

	if (this->_handleFileUrl(request, root_page)) return;
}
Put::~Put(void) {}

bool Put::_hasWritePermission(const std::string &filePath) {
	return (access(filePath.c_str(), W_OK) == 0);
}

void Put::process(Response &response, Request &request) {
	(void)response;
	(void)server_config;

	if (this->_returnCode != 0) return;

	// Essayer d'écrire le contenu du body dans le fichier
	std::ofstream file(this->_filePath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		this->_returnCode = 500;
		return;
	}

	std::string body = request.getBody();
	file.write(body.c_str(), body.length());
	file.close();

	if (fileExists(this->_filePath)) {
		this->_returnCode = 200;
	} else {
		this->_returnCode = 201;
	}

	if (request.isCgiEnabled()) {
		CGI cgi_handler(request.getMethod(), request.getProtocol(), request.getHeaders(), 8080);
		cgi_handler.setEnvironment(this->_filePath, request.getLocation(), *this->server_config);
		cgi_handler.formatEnvironment();
		cgi_handler.execute(request.getBody());
	}

	return;
}

bool Put::_handleFileUrl(Request &request, const std::string root) {
	const std::string &filePath = request.getUrl();
	std::string fullPath = root + filePath;

	this->_fileName = getLastSub(fullPath, '/');

	// Pour PUT on peut créer un nouveau fichier donc pas besoin de vérifier l'existence
	if (isDirectory(fullPath)) {
		this->_returnCode = 405;
		return (false);
	}

	// Vérifier les permissions d'écriture
	std::string parentDir = fullPath.substr(0, fullPath.find_last_of('/'));
	if (!this->_hasWritePermission(parentDir)) {
		this->_returnCode = 403;
		return (false);
	}

	this->_filePath = fullPath;
	return (true);
}
