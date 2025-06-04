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
		CGI(std::string method, std::string protocol, std::map<std::string, std::string> headers);
		~CGI(void);

		void execute(const std::string& body);
		char **formatEnvironment();
		void setEnvironment(std::string scriptPath, Config &config);
		void _addEnv(std::string index, std::string value);  // AJOUT : rendu public

	private:
		std::string _method;
		std::string _protocol;
		std::map<std::string, std::string> _headers;
		std::map<std::string, std::string> _env;
		
		char **_envp;
		bool _envpFormatted;

		bool _addEnvHeader(std::string index, std::string headerIndex);
		std::string _findHeader(std::string index);
};

#endif