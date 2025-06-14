#include "config.hpp"
#include "bindingSocket.hpp"
#include "listenSocket.hpp"

int main(void) {

	Config *config = new Config("./config/default.conf");
	
	std::vector<int> ports = config->getServerPorts();
	std::vector<BindingSocket*> bindingSockets;
	
	for (size_t i = 0; i < ports.size(); i++) {
		BindingSocket* socket = new BindingSocket(AF_INET, SOCK_STREAM, 0, INADDR_ANY, config, ports[i]);
		bindingSockets.push_back(socket);
	}
	
	ListenSocket listener(bindingSockets, config);
	listener.launch();

	for (size_t i = 0; i < bindingSockets.size(); i++) {
		delete bindingSockets[i];
	}
	delete config;
	return 0;
}
