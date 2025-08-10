#include "Post.hpp"

Post::Post(void) {}
Post::Post(Request &request, Config *config) {
	this->_returnCode = 0;
	this->displayErrorPage = false;
	this->_cgiResponse = false;
	this->server_config = config;

	if (!this->_isCgiRequest(request)) {
		this->_returnCode = 406;
		return;
	}
	this->_handleFileUrl(request);
	return;
}

Post::~Post(void) {}

bool Post::_isCgiRequest(Request &request) {
	return request.isCgiEnabled();
}

void Post::process(Response &response, Request &request) {
	if (this->_returnCode != 0)
		return;

	try {
		std::string url = request.getUrl();
		std::string scriptPath = request.getPath();
		if (scriptPath.empty()) {
			this->_returnCode = 404;
			return;
		}
		std::string postData = request.getBody();
		this->_executeCgiScript(request, scriptPath, postData);
	} catch (std::exception &e) {
		this->_content = "{\"success\": false, \"error\": \"CGI execution error\"}";
		this->_returnCode = 500;
	}
	this->_checkCgiResponse(response);
	return;
}

bool Post::_handleFileUrl(Request &request) {
	std::string path = request.getPath();

	this->_fileName = getLastSub(path, '/');

	if (!fileExists(path)) { this->_returnCode = 404; return (false); }
	if (isDirectory(path)) { this->_returnCode = 405; return (false); }
	if (!hasReadPermission(path)) { this->_returnCode = 403; return (false); }

	this->_filePath = path;
	return (true);
}

void Post::_checkCgiResponse(Response &response) {
	std::string cgiContent = this->_content;

	size_t contentTypePos = cgiContent.find("Content-Type: ");
	size_t contentTypeEnd = cgiContent.find_first_of('\n');

	if (contentTypePos != std::string::npos) {
		response.addHeader("Content-Type", cgiContent.substr(contentTypePos + 14, contentTypeEnd - 1));
		cgiContent = cgiContent.substr(contentTypeEnd, cgiContent.length());
	} else
		response.addHeader("Content-Type", "application/json");

	size_t statusCodePos = cgiContent.find("Status: ");
	size_t statusCodeEnd = cgiContent.find_first_of('\n');

	if (statusCodePos != std::string::npos) {
		this->_returnCode = ft_atoi(cgiContent.substr(statusCodePos + 8, statusCodeEnd - 1));
		cgiContent = cgiContent.substr(statusCodeEnd, cgiContent.length());
	} else
		this->_returnCode = 200;

	size_t contentLengthPos = cgiContent.find("Content-Length: ");
	size_t contentLengthEnd = cgiContent.find_first_of('\n');

	if (contentLengthPos != std::string::npos) {
		response.addHeader("Content-Length", cgiContent.substr(contentLengthPos + 16, contentLengthEnd - 1));
		cgiContent = cgiContent.substr(contentLengthEnd, cgiContent.length());
	} else
		response.addHeader("Content-Length", ft_itoa(cgiContent.length()));
}

void Post::_executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData) {
	std::string uploadDir = this->server_config->getUploadStore(request.getLocation());
	std::map<std::string, std::string> headers = request.getHeaders();
	std::string contentType = "application/x-www-form-urlencoded";
	std::map<std::string, std::string>::iterator ctIt;

	for (ctIt = headers.begin(); ctIt != headers.end(); ++ctIt) {
		if (ctIt->first == "Content-Type") {
			contentType = ctIt->second;
			break;
		}
	}

	headers["Content-Type"] = contentType;
	headers["Content-Length"] = ft_itoa(postData.length());

	std::string executorPath = this->server_config->getCGIPath(request.getLocation(), request.getCgiExtension());
	CGI cgi_handler(request.getMethod(), request.getProtocol(), headers, request.getServerPort());
	cgi_handler.setEnvironment(scriptPath, executorPath, request.getLocation(), *this->server_config);
	cgi_handler._addEnv("UPLOAD_DIR", uploadDir);
	cgi_handler._addEnv("CONTENT_LENGTH", ft_itoa(postData.length()));
	cgi_handler.formatEnvironment();


	try {
		this->_content = cgi_handler.execute(postData);
		std::cout << "Content: " << std::endl << this->_content << std::endl;
	} catch (std::exception &e) {
		this->_content = "{\"success\": false, \"error\": \"CGI execution error\"}";
		this->_returnCode = 500;
		return;
	}

	size_t headerEndPos = this->_content.find("\r\n\r\n");
	if (headerEndPos == std::string::npos) {
		headerEndPos = this->_content.find("\n\n");
		if (headerEndPos != std::string::npos) {
			headerEndPos += 2;
		}
	} else
		headerEndPos += 4;

	if (headerEndPos != std::string::npos)
		this->_content = this->_content.substr(headerEndPos);
	this->_returnCode = 200;
}
