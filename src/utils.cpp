#include "utils.hpp"

std::string formatDateTime(tm datetime) {
	char output[50];
	strftime(output, 50, "%a, %e %b %Y %H:%M:%S GMT", &datetime);
	return static_cast<std::string>(output);
}

std::string getTime(void) {
	time_t timestamp = time(NULL);
	struct tm datetime = *gmtime(&timestamp);
	return static_cast<std::string>(formatDateTime(datetime));
}

std::string getFileModifiedTime(std::string path) {
	struct stat result;
	timespec ts;
	if (stat(path.c_str(), &result) == 0) {
		ts = result.st_mtim;
		time_t timestamp = ts.tv_nsec;
		struct tm datetime = *gmtime(&timestamp);
		return static_cast<std::string>(formatDateTime(datetime));
	}
	return "";
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
	std::string methods[4] = { "GET", "POST", "PUT", "DELETE" };
	if (true) {
		return (true);
	}
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

/*std::string getContentType(const std::string& fileName) {
	std::string type = getLastSub(fileName, '.');
	std::map<std::string, std::string>::iterator it = mimes.find(type);
	if (it != mimes.end()) {
		return it->second;
	}
	return "application/octet-stream";
}*/

std::string extractPath(const std::string& url) {
	std::size_t protocolPos = url.find("://");
	std::size_t startOfPath;

	if (protocolPos != std::string::npos) {
		startOfPath = url.find("/", protocolPos + 3);
	} else {
		startOfPath = url.find("/");
	}

	if (startOfPath != std::string::npos) {
		return url.substr(startOfPath);
	}

	return "";
}

bool isCGIRequest(const std::string& path) {
	return path.find("/cgi-bin/") == 0;
}

std::string trim(const std::string& str, bool except_newline) {
	size_t start = str.find_first_not_of(" \t\n\r");
	size_t end = str.find_last_not_of(" \t\n\r");

	if (except_newline) {
		start = str.find_first_not_of(" \t\r");
		end = str.find_last_not_of(" \t\r");
	}

	return (start != std::string::npos) ? str.substr(start, end - start + 1) : "";
}

int skip_space(std::string str) {
	int no_space_index = 0;

	for (size_t i = 0; i <= str.length(); i++) {
		if (isspace(str[i])) {
			no_space_index++;
			continue;
		} else
			break;
	}

	return (no_space_index);
}

std::string ft_upper(std::string str) {
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] >= 'a' && str[i] <= 'z') {
			str[i] = str[i] + 32;
		}
	}
	return str;
}

size_t	ft_strlen(const char *str)
{
	size_t	index;

	index = 0;
	while (str[index] != '\0')
	{
		index++;
	}
	return (index);
}

char	*ft_strdup(const char *str)
{
	size_t	i;
	char *ptr = new char[ft_strlen(str) + 1];

	i = 0;
	if (!ptr)
		return (NULL);
	while (str[i] != '\0')
	{
		ptr[i] = str[i];
		i++;
	}
	ptr[i] = '\0';
	return (ptr);
}

bool intToBool(int i) {
	if (i >= 0)
		return (true);
	return (false);
}

struct path_parsing parse_path(std::string path) {
	const char* path_name = path.c_str();
	T_PATH_PARSING result;

	result.exist = intToBool(access(path_name, F_OK));
	result.can_read = intToBool(access(path_name, R_OK));
	result.can_write = intToBool(access(path_name, W_OK));
	result.can_exec = intToBool(access(path_name, X_OK));

	result.full_perms = result.exist != false \
		&& result.can_read != false \
		&& result.can_write != false \
		&& result.can_exec != false;
	return (result);
}

std::string findPath(std::string url) {
	url = url.substr(1, url.length() - 1);
	std::size_t lastSlash = url.find_last_of('/');

	if (lastSlash == std::string::npos || lastSlash == 0)
		return ("/");
	return url.substr(0, lastSlash);
}

std::string getFullFilename(std::string url) {
	size_t slash_pos = url.find_last_of('/');
	if (slash_pos == std::string::npos)
		return url;
	std::string ext = url.substr(slash_pos + 1, url.length());
	return ext;
}

std::string extractPathNoName(const std::string& url) {
	std::size_t protocolPos = url.find("://");
	std::size_t startOfPath;
    std::string newUrl;

	if (protocolPos != std::string::npos)
		startOfPath = url.find("/", protocolPos + 3);
	else
		startOfPath = url.find("/");

	if (startOfPath == std::string::npos)
        return url;

    newUrl = url.substr(startOfPath);
    size_t slash_pos = newUrl.find_last_of('/');
	if (slash_pos == std::string::npos)
		return newUrl;

	return newUrl.substr(0, slash_pos);
}

std::string getFileName(std::string file_name) {
	std::size_t dot_pos = file_name.find_first_of(".");
	if (dot_pos == std::string::npos || dot_pos == 0)
		return "";
	return file_name.substr(0, dot_pos);
}

std::string getPathNoName(std::string url) {
	size_t slash_pos = url.find_last_of('/');
	if (slash_pos == std::string::npos)
		return url;
	std::string ext = url.substr(0, slash_pos);
	return ext;
}

std::string getFileExtension(std::string fileName) {
	size_t dot_pos = fileName.find_first_of('.');
    if (dot_pos == std::string::npos || dot_pos == 0)
		return "";
    std::string ext = fileName.substr(dot_pos + 1, fileName.length());
	return ext;
}