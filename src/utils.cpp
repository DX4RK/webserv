#include "../includes/utils.hpp"

std::string getTime(void) {
	time_t timestamp = time(NULL);
	struct tm datetime = *gmtime(&timestamp);

	char output[50];
	strftime(output, 50, "%a, %e %b %Y %H:%M:%S GMT", &datetime);
	return static_cast<std::string>(output);
}

void make_error(std::string errorMessage, int exitCode) {
	std::cout << errorMessage << std::endl;
	exit(exitCode);
}

std::string getLastSub(const std::string& src, char c) {
	std::size_t lastSlashPos = src.find_last_of(c);
	if (lastSlashPos == std::string::npos) {
		return src;
	}
	return src.substr(lastSlashPos + 1);
}

// NEW

bool fileExists(const std::string &filePath) {
	struct stat buffer;
	return (stat(filePath.c_str(), &buffer) == 0);
}

bool isDirectory(const std::string &path) {
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0) {
		return false;
	}
	return S_ISDIR(buffer.st_mode);
}

bool hasReadPermission(const std::string &filePath) {
	return (access(filePath.c_str(), R_OK) == 0);
}

bool methodValid(std::string method) {
	std::string methods[3] = { "GET", "POST", "PUT" };
	for (size_t i = 0; i < methods->length(); i++) {
		if (methods[i] == method) { return true; }
	}
	return false;
}

bool protocolValid(std::string protocol) {
	if (protocol.substr(0, 4).compare("HTTP") != 0) { return false; }
	if (protocol[4] != '/') { return false; }
	if (protocol.substr(5, 3).compare("1.1") != 0) { return false; }

	return (true);
}

std::vector<std::string> getLines(std::string buffer) {
	std::string line;
	std::vector<std::string> lines;

	for (size_t i = 0; i <= buffer.length(); i++) {
		if (buffer[i] == '\n') {
			lines.push_back(line); line = "";
			continue;
		}
		line = line + buffer[i];
	}
	if (line.length() > 0) { lines.push_back(line); line = ""; }

	return (lines);
}

std::vector<std::string> splitString(std::string str) {
	std::stringstream ss(str);
	std::string word;
	std::vector<std::string> words;

	while (ss >> word) { words.push_back(word); }
	return (words);
}

std::map<std::string, std::string> mimes;

void initMimes() {
	mimes["css"] = "text/css; charset=utf-8";
	mimes["gif"] = "image/gif; charset=utf-8";
	mimes["htm"] = "text/html; charset=utf-8";
	mimes["html"] = "text/html; charset=utf-8";
	mimes["png"] = "image/png; charset=utf-8";
	mimes["jpg"] = "image/jpeg; charset=utf-8";
	mimes["jpeg"] = "image/jpeg; charset=utf-8";
	mimes["txt"] = "text/plain; charset=utf-8";
	mimes["js"] = "text/javascript; charset=utf-8";
	mimes["json"] = "application/json; charset=utf-8";
	mimes["php"] = "application/x-httpd-php; charset=utf-8";
	mimes["svg"] = "image/svg+xml; charset=utf-8";
	mimes["webp"] = "image/webp";
	mimes["woff"] = "font/woff; charset=utf-8";
	mimes["woff2"] = "font/woff2; charset=utf-8";
	mimes["xml"] = "application/xml; charset=utf-8";
	mimes["xhtml"] = "application/xhtml+xml; charset=utf-8";
	mimes["ico"] = "image/vnd.microsoft.icon; charset=utf-8";
}

std::string getContentType(const std::string& fileName) {
	std::string type = getLastSub(fileName, '.');
	std::map<std::string, std::string>::iterator it = mimes.find(type);
	if (it != mimes.end()) {
		return it->second;
	}
	return "application/octet-stream";
}
