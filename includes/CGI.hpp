#ifndef CGI_HPP
#define CGI_HPP

#include "config.hpp"
#include "utils.hpp"
#include <map>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

class CGI {
	public:
		CGI(std::string method, std::string protocol, std::map<std::string, std::string> headers, int serverPort);
		~CGI(void);

		std::string execute(const std::string& body);  // Maintenant retourne std::string
		std::string getOutput() const;  // Getter pour la sortie
		char **formatEnvironment();
		void setEnvironment( std::string scriptPath, std::string location, Config &config );
		void _addEnv(std::string index, std::string value);  // Public

	private:
		std::string _method;
		std::string _protocol;
		std::string _output;  // AJOUT : stockage de la sortie
		std::map<std::string, std::string> _headers;
		std::map<std::string, std::string> _env;
		int _serverPort;  // AJOUT : port du serveur

		char **_envp;
		bool _envpFormatted;

		bool _addEnvHeader(std::string index, std::string headerIndex);
		std::string _findHeader(std::string index);
};

#endif
