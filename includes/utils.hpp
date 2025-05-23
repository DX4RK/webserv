#pragma once

#include <map>
#include <string>
#include <vector>
#include <vector>
#include <sstream>
#include <sstream>
#include <iostream>

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BOLD "\e[1m"
#define RESET "\e[0m"

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define BLUE "\033[0;94m"

#define LIGHT_CYAN "\033[96m"
#define LIGHT_PURPLE "\033[95m"

//std::map<std::string, std::string> mimes;

template <typename T>
std::string ft_itoa(T num) {
	std::ostringstream ss;
	ss << num;
	return ss.str();
}

std::string getTime(void);
std::string trim(const std::string& str);
std::string extractPath(const std::string& url);
std::string getFileModifiedTime(std::string path);
std::vector<std::string> getLines(std::string buffer);
std::vector<std::string> splitString(std::string str);
std::string getLastSub(const std::string& src, char c);
std::string getContentType(const std::string& file_path);

void initMimes();
void make_error(std::string errorMessage, int exitCode);

bool isCGIRequest(const std::string& path);
bool fileExists(const std::string &filePath);
bool isDirectory(const std::string &path);
bool hasReadPermission(const std::string &filePath);
bool methodValid(std::string method);
bool protocolValid(std::string protocol);
