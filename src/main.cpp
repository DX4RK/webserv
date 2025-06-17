#include "config.hpp"
#include "bindingSocket.hpp"
#include "listenSocket.hpp"

volatile sig_atomic_t g_keepRunning = 1;

void handle_sigint(int signum) {
	(void)signum;
	g_keepRunning = 0;
}

int main(void) {
	signal(SIGINT, handle_sigint);
	signal(SIGTERM, handle_sigint);

	Config *config = new Config("./config/default.conf");

	std::vector<int> ports = config->getServerPorts();
	std::vector<BindingSocket*> bindingSockets;

	for (size_t i = 0; i < ports.size(); i++) {
		BindingSocket* socket = new BindingSocket(AF_INET, SOCK_STREAM, 0, INADDR_ANY, config, ports[i]);
		bindingSockets.push_back(socket);
	}

	try {
		ListenSocket listener(bindingSockets, config);
		listener.launch(g_keepRunning);
	} catch (const std::exception &e) {
		std::cerr << "Fatal error: " << e.what() << std::endl;
	}

	{
		for (size_t i = 0; i < bindingSockets.size(); i++)
			delete bindingSockets[i];
		bindingSockets.clear();
	}

	delete config;
	return 0;
}
