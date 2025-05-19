#include <sstream>
#include <string>
#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	int result;
	//std::string path = "./web/test/king.lol";
	std::string path = "/root";
	DIR *t = opendir(path.c_str());
	dirent *r = readdir(path.c_str());

	std::cout << t << std::endl;
	result = access(path.c_str(), F_OK);
	std::cout << result << std::endl;
}
