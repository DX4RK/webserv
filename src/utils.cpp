#include "../includes/utils.hpp"

void make_error(std::string errorMessage, int exitCode) {
	std::cout << errorMessage << std::endl;
	exit(exitCode);
}
