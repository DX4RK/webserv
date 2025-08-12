#include "listenSocket.hpp"

ListenSocket::~ListenSocket(void) {
	this->_pollfds.clear();
	this->_clientBuffers.clear();
	this->_sockets.clear();
}

ListenSocket::ListenSocket(std::vector<BindingSocket*> bindingSockets, std::vector<Config*> configs)
	: _sockets(bindingSockets), _configs(configs) {
	for (size_t i = 0; i < this->_sockets.size(); i++) {
		int sock = this->_sockets[i]->get_sock();
		if (listen(sock, 10) < 0) {
			std::cerr << "failed to listen on socket!" << std::endl;
			throw std::runtime_error("failed to listen on socket");
		}
	}

	std::cout << LIGHT_BLUE << BOLD << "[webserv]" << RESET << " listening on ports: " << BOLD;
	for (size_t i = 0; i < this->_sockets.size(); i++) {
		if (i < this->_configs.size() && this->_configs[i]) {
			std::vector<int> ports = this->_configs[i]->getServerPorts();
			for (size_t j = 0; j < ports.size(); j++) {
				std::cout << ports[j];
				if (j < ports.size() - 1) std::cout << ", ";
			}
		}
		if (i < this->_sockets.size() - 1) std::cout << ", ";
	}
	std::cout << RESET << std::endl;
}

std::string ListenSocket::getBuffer(void) const {
	return this->_buffer;
}

void ListenSocket::handler() {
	char* tempBuffer = new char[BUFFER_SIZE];
	std::string request;

	bool headersParsed = false;
	bool isChunked = false;
	size_t contentLength = 0;
	size_t totalRead = 0;

	while (true) {
		ssize_t bytesRead = recv(this->_newSocket, tempBuffer, BUFFER_SIZE - 1, 0);
		if (bytesRead <= 0) {
			break;
		}

		totalRead += static_cast<size_t>(bytesRead);
		tempBuffer[bytesRead] = '\0';
		request.append(tempBuffer, bytesRead);

		if (!headersParsed) {
			size_t headerEnd = request.find("\r\n\r\n");
			if (headerEnd != std::string::npos) {
				headersParsed = true;
				std::string headers = request.substr(0, headerEnd);

				if (headers.find("Transfer-Encoding: chunked") != std::string::npos) {
					isChunked = true;
				} else {
					size_t pos = headers.find("Content-Length:");
					if (pos != std::string::npos) {
						size_t start = headers.find_first_of("0123456789", pos);
						if (start != std::string::npos) {
							contentLength = static_cast<size_t>(std::atoll(headers.c_str() + start));
							if (totalRead >= (headerEnd + 4 + contentLength)) {
								break;
							}
						} else
							break;
					} else
						break;
				}
			}
		} else if (!isChunked && contentLength > 0) {
			if (totalRead >= contentLength)
				break;
		} else if (isChunked && request.find("\r\n0\r\n\r\n") != std::string::npos)
			break;
	}

	delete[] tempBuffer;
	this->_buffer = request;

	Config* config = NULL;
	std::map<int, size_t>::iterator it = this->_clientFdToConfigIdx.find(this->_newSocket);
	if (it != this->_clientFdToConfigIdx.end() && it->second < this->_configs.size()) {
		config = this->_configs[it->second];
	} else if (!this->_configs.empty()) {
		config = this->_configs[0];
	}
	if (!config) {
		std::cerr << "[webserv] ERROR: No config found for client fd " << this->_newSocket << std::endl;
		return;
	}
	Request req(*this, config, 0);
	Response response(req, config);
	this->response = response.getResponse();

	/* DEBUG */

	{
		std::string status_color;
		int status_code = response.getResponseCode();

		if (status_code >= 100) { status_color = BLUE; }
		if (status_code >= 200) { status_color = GREEN; }
		if (status_code >= 300) { status_color = LIGHT_CYAN; }
		if (status_code >= 400) { status_color = RED; }

		std::cout << "Status code: " << status_code << std::endl;
		//std::cout << "Received " << BOLD << LIGHT_PURPLE << req.getMethod() << RESET << " code: " << status_code << " url: " << req.getUrl() << std::endl;
		std::cout << LIGHT_BLUE << BOLD << "[webserv]" << RESET << " treated request " << LIGHT_ORANGE << BOLD << status_code << RESET << DIM << " " << req.getUrl() << RESET << std::endl;
	}
}

void ListenSocket::responder(void) {
	write(this->_newSocket, this->response.c_str(), response.length());
	close(this->_newSocket);
	this->_clientFdToConfigIdx.erase(this->_newSocket);
	this->response = "";
}

void ListenSocket::launch(volatile sig_atomic_t &keepRunning) {
	std::cout << LIGHT_BLUE << BOLD
	<< "[webserv]" << RESET
	<< " website is ready, urls: " << GREEN << BOLD;
	for (size_t i = 0; i < this->_sockets.size(); i++) {
		Config* config = (i < this->_configs.size()) ? this->_configs[i] : NULL;
		if (config) {
			if (i == 1) this->_timeout = config->getTimeout();
			std::vector<int> ports = config->getServerPorts();
			for (size_t j = 0; j < ports.size(); j++) {
				std::cout << "http://" << config->getServerName() << ":" << ports[j];
				if (j < ports.size() - 1) std::cout << ", ";
			}
			if (i < this->_sockets.size() - 1) std::cout << ", ";
		}
	}
	std::cout << RESET << std::endl << std::endl;

	for (size_t i = 0; i < this->_sockets.size(); i++)
	{
		pollfd listenPfd;
		listenPfd.fd = this->_sockets[i]->get_sock();
		listenPfd.events = POLLIN;
		listenPfd.revents = 0;
		this->_pollfds.push_back(listenPfd);
	}

	while (keepRunning)
	{
		int pollResult = poll(&this->_pollfds[0], this->_pollfds.size(), 1000);

		if (pollResult < 0) {
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll failed");
		}

		for (size_t i = 0; i < this->_sockets.size(); i++) {
			int listenSock = this->_sockets[i]->get_sock();

			for (size_t j = 0; j < this->_pollfds.size(); j++) {
				if (this->_pollfds[j].fd == listenSock && (this->_pollfds[j].revents & POLLIN)) {
					struct sockaddr_in adress = this->_sockets[i]->get_address();
					int adress_len = sizeof(adress);

					int newSocket = accept(listenSock, (struct sockaddr *)&adress, (socklen_t*)&adress_len);
					if (newSocket >= 0) {
						pollfd clientPfd;
						clientPfd.fd = newSocket;
						clientPfd.events = POLLIN;
						clientPfd.revents = 0;
						this->_pollfds.push_back(clientPfd);
						this->_clientBuffers[newSocket] = "";
					}
					this->_pollfds[j].revents = 0;
				}
			}
		}

		for (size_t i = this->_sockets.size(); i < this->_pollfds.size(); )
		{
			if (this->_pollfds[i].revents & POLLIN)
			{
				this->_newSocket = this->_pollfds[i].fd;
				this->handler();
				this->responder();

				this->_clientBuffers.erase(this->_newSocket);
				this->_pollfds.erase(this->_pollfds.begin() + i);
			}
			else
				i++;
		}
	}
}
