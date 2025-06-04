#include "Get.hpp"

Get::Get(void) {}
Get::Get(Request &request, Config *config) {
	this->_returnCode = 0;
	this->server_config = config;

	//  Gestion récupération posts forum
	if (request.getUrl() == "/forum/posts") {
		this->_handleGetPosts(request);
		return;
	}

	const std::string root_page = this->server_config->getLocationRoot("/");

	bool canProcess = this->_handleFileUrl(request, root_page);

	if (!canProcess) { return; }
	this->_fileFd = open(this->_filePath.c_str(), O_RDONLY);
}
Get::~Get(void) {}

//  Fonction pour récupérer les posts
void Get::_handleGetPosts(Request &request) {
	(void)request; // Fix warning unused parameter
	
	try {
		std::string postData = "type=get_posts";
		
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = "application/x-www-form-urlencoded";
		headers["Content-Length"] = ft_itoa(postData.length());
		
		CGI cgi_handler("POST", "HTTP/1.1", headers);
		cgi_handler.setEnvironment("./www/cgi-bin/forum.py", *this->server_config);
		cgi_handler._addEnv("CONTENT_LENGTH", ft_itoa(postData.length()));
		cgi_handler.formatEnvironment();
		cgi_handler.execute(postData);
		
		this->_returnCode = 200;
		
	} catch (std::exception &e) {
		this->_content = "{\"posts\": [], \"error\": \"Server error\"}"; // Fix R"" syntax
		this->_returnCode = 500;
	}
}

void Get::process(Response &response, Request &request) {
	(void)request;

	//  Réponse JSON pour forum posts
	if (request.getUrl() == "/forum/posts") {
		response.addHeader("Content-Type", "application/json");
		return;
	}

	// Si c'est une page générée, on utilise le contenu déjà préparé
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

	// Check if it's a directory
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