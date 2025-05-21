#include "Get.hpp"

Get::Get(void) {}
Get::Get(Request &rq) { this->_fileFd = open(rq.getUrl().c_str(), O_RDONLY); }
Get::~Get(void) {}

void Get::process(Request &rq) {
	(void)rq;

	char tempBuffer[30000] = {0};
	ssize_t bytesRead = read(this->_fileFd, tempBuffer, 30000);
	if (bytesRead < 0) {
		close(this->_fileFd);
		return;
	}

	this->_content = std::string(tempBuffer, bytesRead);

	close(this->_fileFd);

	this->_contentLength = this->_content.length();
}
