#include "CGI.hpp"

//! REMOVE LEAKS

CGI::CGI(std::string method, std::string protocol, std::map<std::string, std::string> headers) {
	this->_envpFormatted = false;

	this->_method = method;
	this->_protocol = protocol;
	this->_headers = headers;
}
CGI::~CGI(void) {
	//delete this->_envp;
}

void CGI::execute() {
	pid_t pid;
	int status;
	char **envp = this->_envp;

	int fd[2];
	if (pipe(fd) < 0) return ;

	//std::string cmd = "/bin/ls";
	std::string script = this->_env["SCRIPT_NAME"];
	//char *arguments[] = {(char*)cmd.c_str(), (char*)0};
	//char *arguments[] = {(char*)cmd.c_str(), (char*)script.c_str(), (char*)0};

	//std::cout << script << std::endl;

	std::string cmd = "/bin/python3";
	char *arguments[] = {(char*)"/bin/python3", (char*)script.c_str(), 0};

	pid = fork();
	if (pid < 0)
		return ;
	else if (pid == 0) {
		close(fd[0]);
		if (dup2(fd[1], STDOUT_FILENO) < 0) {
			return ;
		}
		close(fd[1]);
		if (execve(cmd.c_str(), arguments, envp) < 0) {
			return ;
		}
	} else {
		close(fd[1]);
		char bff[4096];
		read(fd[0], bff, sizeof(bff) - 1);
		std::cout << bff << std::endl;
		close(fd[0]);
		waitpid(pid, &status, 0);
	}
}

char **CGI::formatEnvironment() {
	if (this->_envpFormatted) { return this->_envp; }

	size_t envSize = this->_env.size();
	char **envp = new char*[envSize + 1];

	size_t index = 0;
	std::map<std::string, std::string>::iterator it;
	for (it = this->_env.begin(); it != this->_env.end(); ++it, ++index) {
		std::string data = it->first + "=" + it->second;
		envp[index] = ft_strdup(data.c_str());
	}
	envp[envSize] = NULL;
	this->_envp = envp;
	this->_envpFormatted = true;
	return envp;
}

void CGI::setEnvironment( std::string scriptPath, Config &config ) {
	this->_addEnv("REQUEST_METHOD", ft_upper(this->_method));
	this->_addEnv("SCRIPT_NAME", scriptPath);
	this->_addEnv("QUERY_STRING", "");

	if (!this->_addEnvHeader("CONTENT_TYPE", "Content-Type")) throw std::exception();
	if (!this->_addEnvHeader("CONTENT_LENGTH", "Content-Length")) throw std::exception();

	this->_addEnv("SERVER_PROTOCOL", this->_protocol);
	this->_addEnv("SERVER_SOFTWARE", config.getServerInfo());
	this->_addEnv("GATEWAY_INTERFACE", "CGI/1.1");
	this->_addEnv("SERVER_NAME", config.getServerName());
	this->_addEnv("SERVER_PORT", ft_itoa(config.getServerPort()));
}

bool CGI::_addEnvHeader(std::string index, std::string headerIndex) {
	try {
		std::string headerValue = this->_findHeader(headerIndex);
		this->_addEnv(index, headerValue);
	} catch (std::exception &e) { return false; }
	return true;
}

void CGI::_addEnv(std::string index, std::string value) {
	this->_env[index] = value;
}

// GETTERS & SETTERS

std::string CGI::_findHeader( std::string index ) {
	std::map<std::string, std::string>::const_iterator it = this->_headers.find(index);

	if (it != this->_headers.end()) return (it->second);
	throw std::exception();
}

