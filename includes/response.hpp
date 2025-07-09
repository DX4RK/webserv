#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "config.hpp"
#include "request.hpp"
#include "method.hpp"
#include "Get.hpp"
#include "Delete.hpp"
#include "Post.hpp"
#include "GetError.hpp"
#include "utils.hpp"
#include "CGI.hpp"  // AJOUT

class Response {
	public:
		Response( void );
		Response( Request &request, Config *config );
		~Response( void );

		void addHeader( std::string headerName, std::string headerValue );

		/* GETTERS & SETTERS */
		int getResponseCode( void ) const;
		std::string getResponse( void ) const;

	private:
		Config *server_config;

		int _responseCode;
		std::string _response;
		std::string _headers;

		Method _processRequest( std::string method, Request &request );
		void _handleGithubCallback(Request &request);  // AJOUT
};

#endif
