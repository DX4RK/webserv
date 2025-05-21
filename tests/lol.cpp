#include <sstream>
#include <string>
#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

std::string getFileName(const std::string& url) {
	std::size_t lastSlashPos = url.find_last_of('.');
	if (lastSlashPos == std::string::npos) {
		return url;
	}
	return url.substr(lastSlashPos + 1);
}

int main() {
	std::string lol = "index.html";
	std::cout << getFileName(lol) << std::endl;
}
