#ifndef POST_HPP
#define POST_HPP

#include "method.hpp"
#include "request.hpp"
#include "response.hpp"
#include "config.hpp"
#include "CGI.hpp"
#include "utils.hpp"

class Response;

class Post : public Method {
	public:
		Post( void );
		Post( Request &request, Config *config );
		~Post( void );

		void process( Response &response, Request &request );

	private:
		std::string _fileName;
		std::string _filePath;
		Config *server_config;

		bool _handleFileUrl( Request &request, const std::string root );
		bool _isCgiRequest(Request &request);
		void _handleCgiRequest(Request &request);
		void _executeCgiScript(Request &request, const std::string &scriptPath, const std::string &postData);
		std::string _getSessionUser(Request &request);
};

#endif