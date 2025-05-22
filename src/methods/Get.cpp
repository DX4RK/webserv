#include "Get.hpp"

Get::Get(void) {}
Get::Get(Request &rq) { std::cout << rq.getUrl(); this->_fileFd = open(rq.getUrl().c_str(), O_RDONLY); }
Get::~Get(void) {}

void Get::process(Response &response, Request &request) {

	char tempBuffer[30000] = {0};
	ssize_t bytesRead = read(this->_fileFd, tempBuffer, 30000);
	if (bytesRead < 0) {
		close(this->_fileFd);
		return;
	}

	this->_content = std::string(tempBuffer, bytesRead);
	close(this->_fileFd);

	response.addHeader("Content-Type", getContentType(request.getUrl()));
	response.addHeader("Content-Length", ft_itoa(bytesRead));
	response.addHeader("Last-Modified", getFileModifiedTime(request.getUrl()));


}
