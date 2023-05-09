#include "Request.hpp"

Request::Request()
: chunked(false), keepAlive(true)
{
    
}

Request::MethodType Request::getMethod() const {
    return method;
}

void Request::setMethod(MethodType method) {
    this->method = method;
}


std::string Request::getUriPath() const {
    return uriPath;
}

void Request::setUriPath(std::string uriPath) {
    this->uriPath = uriPath;
}


std::string Request::getQueryString() const {
    return queryString;
}

void Request::setQueryString(std::string queryString) {
    this->queryString = queryString;
}


std::string Request::getHttpVersion() const {
    return httpVersion;
}

void Request::setHttpVersion(std::string httpVersion) {
    this->httpVersion = httpVersion;
}


Request::HeaderFieldsMap Request::getHeaderFields() const {
    return headerFields;
}

void Request::setHeaderFields(std::string key, std::string value) {
    this->headerFields[key] = value;
}


Request::HostType Request::getHost() const {
    return host;
}

void Request::setHost(HostType host) {
    this->host = host;
}

Request::FieldValueListType Request::getTransferEncoding() const {
    return transferEncoding;
}

void Request::setTransferEncoding(FieldValueListType transferEncoding) {
    this->transferEncoding.swap(transferEncoding);
}

size_t Request::getContentLength() const {
    return contentLength;
}

void Request::setContentLength(ContentLengthType contentLength) {
    this->contentLength = contentLength;
}

Request::CookieType Request::getCookie() const {
    return cookie;
}

void Request::setCookie(std::string key, std::string value) {
    this->cookie[key] = value;
}

std::string Request::getBody() const {
    return body;
}

void Request::setBody(std::string body) {
    this->body = body;
}

std::string Request::getErrorCode() const {
    return errorCode;
}

void Request::setErrorCode(std::string errorCode) {
    this->errorCode = errorCode;
}

Request::PortType Request::getPort() const {
    return port;
}

void Request::setPort(PortType port) {
    this->port = port;
}

void Request::setChunked(bool chunked)
{
    this->chunked = chunked;
}

bool Request::getChunked() const
{
    return chunked;
}

void Request::setKeepAlive(bool keepAlive)
{
    this->keepAlive = keepAlive;
}

bool Request::getKeepAlive() const
{
    return keepAlive;
}

void Request::setMultipartFormDataId(MultipartFormDataIdType multipartFormDataId)
{
    this->multipartFormDataId = multipartFormDataId;
}

Request::MultipartFormDataIdType Request::getMultipartFormDataId() const
{
    return multipartFormDataId;
}

void Request::setQueryStringMap(std::string key, std::string value)
{
    this->queryStringMap[key] = value;
}

Request::QueryStringMapType Request::getQueryStringMap() const
{
    return queryStringMap;
}

void Request::setCookieString(std::string cookieString)
{
    this->cookieString = cookieString;
}

Request::CookieStringType Request::getCookieString() const
{
    return cookieString;
}

void Request::setContentType(ContentType contentType)
{
    this->contentType = contentType;
}

Request::ContentType Request::getContentType() const
{
    return contentType;
}

void Request::requestInitialize()
{
    method = "";
    uriPath = "";
    queryString = "";
    queryStringMap.empty();
    httpVersion = "";
    headerFields.empty();
    host = "";
    port = "";
    transferEncoding.empty();
    chunked = false;
    keepAlive = true;    
    contentLength = 0;
    contentType = "";
    cookie.empty();
    cookieString = "";
    multipartFormDataId = "";
    body = "";
    errorCode = "";
}