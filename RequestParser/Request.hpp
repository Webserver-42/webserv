#ifndef REQUEST_H
# define REQUEST_H

#include <map>
#include <list>
#include <string>


class Request
{
	public:
		typedef std::map<std::string, std::string>	HeaderFieldsMap;		
		typedef std::map<std::string, std::string>  CookieType;
		typedef std::string													HostType;
		typedef	int																	PortType;
		typedef std::list<std::string>							FieldValueListType;

		enum	method_enum
		{
			GET = 0,
			POST = 1,
			DELETE = 2
		};

		Request();

	private:
		//start line
		method_enum	method;
		std::string	uriPath;
		std::string	queryString;
		std::string	httpVersion;

		//header fields
		HeaderFieldsMap	headerFields;//헤더필드 전체를 이곳에 우선 저장 후, 필요한 내용만 꺼내 requeset 구조체 멤버변수에 저장
		HostType	host;
		PortType	port;
		FieldValueListType	transferEncoding;
		bool			chunked;
		uint32_t	contentLength;
		CookieType	cookie;

		//body
		std::string	body; // /r/n/r/n 뒤를 전체 저장함.

		//error
		std::string errorCode;

	public:
		method_enum getMethod() const;
		std::string getUriPath() const ;
		std::string getQueryString() const;
		std::string getHttpVersion() const;
		HeaderFieldsMap getHeaderFields() const;
		HostType getHost() const;
		FieldValueListType getTransferEncoding() const;
		uint32_t getContentLength() const;
		CookieType getCookie() const;
		std::string getBody() const;
		std::string getErrorCode() const;
		PortType	getPort() const;
		bool	getChunked() const;

		void setMethod(method_enum method);
		void setUriPath(std::string uriPath);
		void setQueryString(std::string queryString);
		void setHttpVersion(std::string httpVersion);
		void setHeaderFields(std::string key, std::string value);
		void setHost(HostType host);
		void setTransferEncoding(FieldValueListType transferEncoding);
		void setContentLength(uint32_t contentLength);
		void setCookie(std::string key, std::string value);
		void setBody(std::string body);
		void setErrorCode(std::string errorCode);
		void setPort(PortType port);
		void setChunked(bool chunked);
	
};

#endif