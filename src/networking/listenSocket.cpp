#include "listenSocket.hpp"

ListenSocket::~ListenSocket(void) {}

ListenSocket::ListenSocket(BindingSocket *mainSocket, Config *config) : _socket(mainSocket) {
	this->server_config = config;
	int sock = mainSocket->get_sock();
	if (listen(sock, 10) < 0) {
		std::cerr << "failed to listen on socket!" << std::endl;
		exit (EXIT_FAILURE);
	}
}

std::string ListenSocket::getBuffer(void) const {
	return this->_buffer;
}

void ListenSocket::accepter(void) {
	int sock = this->_socket->get_sock();
	struct sockaddr_in adress = this->_socket->get_address();
	int adress_len = sizeof(adress);

	this->_newSocket = accept(sock, (struct sockaddr *)&adress, (socklen_t*)&adress_len);
	if (this->_newSocket < 0) {
		make_error("failed to accept socket", EXIT_FAILURE);
	}

	char tempBuffer[4096];

	ssize_t bytesRead;
	size_t contentLength = 0;

	std::string request;
	bool headersParsed = false;

	while (true) {
		bytesRead = recv(this->_newSocket, tempBuffer, sizeof(tempBuffer) - 1, 0);
		if (bytesRead <= 0) break;

		tempBuffer[bytesRead] = '\0';
		request.append(tempBuffer, bytesRead);

		if (!headersParsed) {
			size_t headerEnd = request.find("\r\n\r\n");
			if (headerEnd != std::string::npos) {
				headersParsed = true;

				std::string headers = request.substr(0, headerEnd);
				size_t pos = headers.find("Content-Length:");
				if (pos != std::string::npos) {
					size_t start = headers.find_first_of("0123456789", pos);
					if (start != std::string::npos) {
						contentLength = std::atoi(headers.c_str() + start);
					}
				}

				size_t totalNeeded = headerEnd + 4 + contentLength;
				if (request.size() >= totalNeeded) break;
			}
		} else {
			size_t headerEnd = request.find("\r\n\r\n");
			size_t totalNeeded = headerEnd + 4 + contentLength;
			if (request.size() >= totalNeeded) break;
		}
	}

	this->_buffer = request;
}
void ListenSocket::handler() {
	Request request(*this, server_config);
	Response response(request, server_config);

	this->response = response.getResponse();

	/* DEBUG */

	{
		std::string status_color;
		int status_code = response.getResponseCode();

		if (status_code >= 100) { status_color = BLUE; }
		if (status_code >= 200) { status_color = GREEN; }
		if (status_code >= 300) { status_color = LIGHT_CYAN; }
		if (status_code >= 400) { status_color = RED; }

		std::cout << "Received " << BOLD << LIGHT_PURPLE << request.getMethod() << RESET << " code: " << status_code << " url: " << request.getUrl() << std::endl;
	}

}

void ListenSocket::responder(void) {
	write(this->_newSocket, this->response.c_str(), response.length());
	close(this->_newSocket);
	this->response = "";
}

void ListenSocket::launch() {
	while (true) {
		this->accepter();
		this->handler();
		this->responder();
	}
}
