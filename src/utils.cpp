#include "../includes/utils.hpp"

std::string ft_itoa(int num) {
	std::ostringstream ss;
	ss << num;
	return ss.str();
}

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
