#include <iostream>
#include <string>

std::string extractPath(const std::string& url) {
    std::size_t protocolPos = url.find("://");
    std::size_t startOfPath;

    if (protocolPos != std::string::npos) {
        startOfPath = url.find("/", protocolPos + 3);
    } else {
        startOfPath = url.find("/");
    }

    if (startOfPath != std::string::npos) {
        return url.substr(startOfPath + 1);
    }

    return "";
}

int main() {
    std::string refer1 = "http://localhost:8080/upload";
    std::string refer2 = "http://example.com/";
    std::string refer3 = "upload";

    std::cout << "Path 1: " << extractPath(refer1) << std::endl;
    std::cout << "Path 2: " << extractPath(refer2) << std::endl;
    std::cout << "Path 3: " << extractPath(refer3) << std::endl;

    return 0;
}
