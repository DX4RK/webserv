#pragma once

#include "_libs.hpp"

#define BOLD "\e[1m"
#define RESET "\e[0m"
#define DIM "\e[2m"

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define BLUE "\033[0;94m"

#define LIGHT_BLUE "\033[94m"
#define LIGHT_CYAN "\033[96m"
#define LIGHT_GREEN "\033[92m"
#define LIGHT_YELLOW "\033[93m"
#define LIGHT_ORANGE "\033[91m"
#define LIGHT_PURPLE "\033[95m"

//std::map<std::string, std::string> mimes;

template <typename T>
std::string ft_itoa(T num) {
	std::ostringstream ss;
	ss << num;
	return ss.str();
}

typedef struct path_parsing {
	bool exist;
	bool can_read;
	bool can_write;
	bool can_exec;

	bool full_perms;
} T_PATH_PARSING;

int ft_atoi(std::string str);
int skip_space(std::string str);

std::string getTime(void);
std::string ft_upper(std::string str);
std::string findPath(std::string url);
int getCurrentPort(int socket);
std::string extractPath(const std::string& url);
std::string getFileModifiedTime(std::string path);
std::string getPathNoName(std::string url);
std::string extractPathNoName(const std::string& url);
std::vector<std::string> getLines(std::string buffer);
std::vector<std::string> splitString(std::string str);
std::string getLastSub(const std::string& src, char c);
std::string getFileName(std::string file_name);
void removeWhitespace(std::string& str);
std::string getWithoutSlashes(std::string url);
std::string getFullFilename(std::string url);
std::string getFullFilenameV2(std::string url);
std::string getFileExtension(std::string fileName);
std::string trim(const std::string& str, bool except_newline);
/*std::string getContentType(const std::string& file_path);*/

void initMimes();
void make_error(std::string errorMessage, int exitCode);

char	*ft_strdup(const char *str);

bool isCGIRequest(const std::string& path);
bool fileExists(const std::string &filePath);
bool isDirectory(const std::string &path);
bool hasReadPermission(const std::string &filePath);
bool methodValid(std::string method);
bool protocolValid(std::string protocol);

T_PATH_PARSING parse_path(std::string path);
