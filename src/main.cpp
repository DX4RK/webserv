#include "config.hpp"
#include "bindingSocket.hpp"
#include "listenSocket.hpp"

volatile sig_atomic_t g_keepRunning = 1;

void handle_sigint(int signum) {
	(void)signum;
	g_keepRunning = 0;
}

std::string getConfigFile(int argc, char **argv) {
	if (argc <= 1) {
		// Try default config file first
		std::string defaultConfig = "./config/default.conf";
		if (fileExists(defaultConfig))
			return defaultConfig;
		throw std::exception();
	}
	std::string fileName = static_cast<std::string>(argv[1]);
	if (!fileExists(fileName))
		throw std::exception();
	return fileName;
}

int main(int argc, char **argv) {
	signal(SIGINT, handle_sigint);
	signal(SIGTERM, handle_sigint);

	std::string config_file;
	try {
		config_file = getConfigFile(argc, argv);
	} catch (std::exception &e) {
		config_file = "./config/default.conf";
		if (argc > 1) {
			std::cout << LIGHT_BLUE << BOLD << "[webserv]" << RESET << RED << BOLD << " Error: specified config file does not exist, default config will be used." << RESET << std::endl;
		} else {
			std::cout << LIGHT_BLUE << BOLD << "[webserv]" << RESET << LIGHT_YELLOW << BOLD << " Warning: no config file specified, using default config." << RESET << std::endl;
		}
	}

	Config *config = new Config(config_file);

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
