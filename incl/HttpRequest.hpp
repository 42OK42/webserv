#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <sstream>

class HttpRequest
{
	public:
		HttpRequest(const std::string& rawRequest);

		const std::string& getMethod() const;
		const std::string& getUrl() const;
		const std::string& getHttpVersion() const;
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;
		std::string getHeader(const std::string& name) const;
		std::string getHost() const;
		std::string getUserAgent() const;
		std::string getConnection() const;
		std::string getAccept() const;

	private:
		std::string method;
		std::string url;
		std::string httpVersion;
		std::map<std::string, std::string> headers;
		std::string body;

		void parse(const std::string& rawRequest);
};

#endif