#include <cstring>
#include <iostream>

int main() {
    char str[] = "GET /index HTTP/1.1";
    const char *del = " ";
    char *t = strtok(str, del);
    while (t != nullptr) {
        std::cout << "\"" << t << "\" ";
        t = strtok(nullptr, del);
    }
    return 0;
}
