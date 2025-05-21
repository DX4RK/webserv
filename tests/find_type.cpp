#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <string>
#include <iostream>
#include <map>

int main(void) {
	std::string type = "html";
	std::map<std::string, std::string> mimes;

	mimes["css"] = "text/css; charset=utf-8";
	mimes["gif"] = "image/gif; charset=utf-8";
	mimes["htm"] = "text/html; charset=utf-8";
	mimes["html"] = "text/html; charset=utf-8";
	mimes["png"] = "image/png; charset=utf-8";
	mimes["jpg"] = "image/jpeg; charset=utf-8";
	mimes["jpeg"] = "image/jpeg; charset=utf-8";
	mimes["txt"] = "text/plain; charset=utf-8";
	mimes["js"] = "text/javascript; charset=utf-8";
	mimes["json"] = "application/json; charset=utf-8";
	mimes["php"] = "application/x-httpd-php; charset=utf-8";
	mimes["svg"] = "image/svg+xml; charset=utf-8";
	mimes["webp"] = "image/webp; charset=utf-8";
	mimes["woff"] = "font/woff; charset=utf-8";
	mimes["woff2"] = "font/woff2; charset=utf-8";
	mimes["xml"] = "application/xml; charset=utf-8";
	mimes["xhtml"] = "application/xhtml+xml; charset=utf-8";
	mimes["ico"] = "image/vnd.microsoft.icon; charset=utf-8";

	std::map<std::string, std::string>::iterator it = mimes.find(type);
	if (it != mimes.end()) {
		std::cout << "Content-Type: " << mimes[type] << std::endl;
	}

}
