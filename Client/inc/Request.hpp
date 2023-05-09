#ifndef REQUEST_H
# define REQUEST_H

#include <map>
#include <list>
#include <string>


class Request
{
	public:
		typedef std::map<std::string, std::string>	HeaderFieldsMap, QueryStringMapType;
		typedef std::map<std::string, std::string>  CookieType;
		typedef std::string							HostType, MultipartFormDataIdType, PortType, CookieStringType;
		typedef std::list<std::string>				FieldValueListType, ChunkedListType, MultipartFormDataIdListType, UriListType;
		typedef size_t								ContentLengthType;
		typedef std::string						MethodType, ContentType;
		Request();

	private:
		//start line
		MethodType	method;
		std::string	uriPath;
		std::string	queryString;
		QueryStringMapType queryStringMap;
		std::string	httpVersion;

		//header fields
		HeaderFieldsMap	headerFields;//헤더필드 전체를 이곳에 우선 저장 후, 필요한 내용만 꺼내 requeset 구조체 멤버변수에 저장
		HostType	host;
		PortType	port;
		FieldValueListType	transferEncoding;
		bool				chunked;					// chunked가 포함되어 있는지 여부(default=false)
		bool				keepAlive;					// keep-alive의 on-off 여부(default=true)
		ContentLengthType	contentLength;
		ContentType contentType;
		CookieType			cookie;
		CookieStringType		cookieString;
		MultipartFormDataIdType multipartFormDataId; //upload시 사용됨. 이 id를 기준으로 계속 데이터가 들어옴.

		//body
		std::string	body; // /r/n/r/n 뒤를 전체 저장함.

		//error
		std::string errorCode;

	public:
		MethodType  getMethod() const;
		std::string getUriPath() const ;
		std::string getQueryString() const;
		std::string getHttpVersion() const;
		HeaderFieldsMap getHeaderFields() const;
		HostType getHost() const;
		FieldValueListType getTransferEncoding() const;
		size_t getContentLength() const;
		ContentType getContentType() const;
		CookieType getCookie() const;
		CookieStringType getCookieString() const;
		std::string getBody() const;
		std::string getErrorCode() const;
		PortType	getPort() const;
		bool	getChunked() const;
		bool	getKeepAlive() const;
		MultipartFormDataIdType getMultipartFormDataId() const;
		QueryStringMapType getQueryStringMap() const;

		void setMethod(MethodType method);
		void setUriPath(std::string uriPath);
		void setQueryString(std::string queryString);
		void setHttpVersion(std::string httpVersion);
		void setHeaderFields(std::string key, std::string value);
		void setHost(HostType host);
		void setTransferEncoding(FieldValueListType transferEncoding);
		void setContentLength(ContentLengthType contentLength);
		void setContentType(ContentType contentType);
		void setCookie(std::string key, std::string value);
		void setCookieString(std::string cookieString);
		void setBody(std::string body);
		void setErrorCode(std::string errorCode);
		void setPort(PortType port);
		void setChunked(bool chunked);
		void setKeepAlive(bool chunked);
		void setMultipartFormDataId(MultipartFormDataIdType multipartFormDataId);
		void setQueryStringMap(std::string key, std::string value);
		void requestInitialize();
	
};

#endif