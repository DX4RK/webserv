#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int64_t timespecToInt(const struct timespec &ts) {
    return (int64_t)ts.tv_sec * 1000000000 + (int64_t)ts.tv_nsec;
}

int main(void) {
	std::string filepath = "test.cpp";
	struct stat result;
	timespec time;
	if (stat(filepath.c_str(), &result) == 0) {
		time = result.st_mtim;
		std::cout << timespecToInt(time) << std::endl;
	}

	return (0);
}
