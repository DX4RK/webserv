#include "CGI.hpp"

CGI::CGI(std::string method, std::string protocol, std::map<std::string, std::string> headers, int serverPort) {
	this->_envpFormatted = false;
	this->_output = "";

	this->_method = method;
	this->_protocol = protocol;
	this->_headers = headers;
	this->_serverPort = serverPort;
}

CGI::~CGI(void) {
	if (this->_envpFormatted && this->_envp) {
		for (size_t i = 0; this->_envp[i]; ++i)
			delete[] this->_envp[i];
		delete[] this->_envp;
	}
}

std::string CGI::execute(const std::string& body) {
	int stdin_pipe[2], stdout_pipe[2];
	if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0) {
		this->_output = "{\"success\": false, \"error\": \"Pipe creation failed\"}";
		return this->_output;
	}

	pid_t pid = fork();
	if (pid < 0) {
		this->_output = "{\"success\": false, \"error\": \"Fork failed\"}";
		return this->_output;
	}

	if (pid == 0) {
		close(stdin_pipe[1]);
		close(stdout_pipe[0]);

		dup2(stdin_pipe[0], STDIN_FILENO);
		dup2(stdout_pipe[1], STDOUT_FILENO);

		close(stdin_pipe[0]);
		close(stdout_pipe[1]);

		this->formatEnvironment();

		std::string scriptPath = this->_env["SCRIPT_NAME"];
		char *arguments[3];
		
		if (scriptPath.find(".py") != std::string::npos) {
			arguments[0] = (char *)"/bin/python3";
			arguments[1] = (char *)scriptPath.c_str();
			arguments[2] = NULL;
			execve("/bin/python3", arguments, this->_envp);
		}
		else if (scriptPath.find(".sh") != std::string::npos) {
			arguments[0] = (char *)"/bin/bash";
			arguments[1] = (char *)scriptPath.c_str();
			arguments[2] = NULL;
			execve("/bin/bash", arguments, this->_envp);
		}
		else {
			arguments[0] = (char *)"/bin/python3";
			arguments[1] = (char *)scriptPath.c_str();
			arguments[2] = NULL;
			execve("/bin/python3", arguments, this->_envp);
		}
		exit(1);
	} else {
		close(stdin_pipe[0]);
		close(stdout_pipe[1]);

		if (!body.empty()) {
			write(stdin_pipe[1], body.c_str(), body.length());
		}
		close(stdin_pipe[1]);

		char buffer[4096];
		ssize_t bytesRead;
		this->_output = "";
		
		while ((bytesRead = read(stdout_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
			buffer[bytesRead] = '\0';
			this->_output += buffer;
		}

		close(stdout_pipe[0]);
		waitpid(pid, NULL, 0);
	}
	
	return this->_output;
}

std::string CGI::getOutput() const {
	return this->_output;
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
	this->_addEnv("SERVER_PORT", ft_itoa(this->_serverPort));
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

std::string CGI::_findHeader( std::string index ) {
	std::map<std::string, std::string>::const_iterator it = this->_headers.find(index);

	if (it != this->_headers.end()) return (it->second);
	throw std::exception();
}