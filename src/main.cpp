#include "config.hpp"
#include "bindingSocket.hpp"
#include "listenSocket.hpp"

int main(void) {

	//initMimes();

	Config *config = new Config("test");
	BindingSocket main_socket(AF_INET, SOCK_STREAM, 0, INADDR_ANY, config);
	ListenSocket listener(&main_socket, config);

	listener.launch();

	delete config;
	return 0;
}
