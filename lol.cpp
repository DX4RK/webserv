#include <sstream>
#include <string>
#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
time_t timestamp = time(NULL);
struct tm datetime = *gmtime(&timestamp);

char output[50];

strftime(output, 50, "%a, %e %b %Y %H:%M:%S GMT", &datetime);
std::cout << output << "\n";
}
