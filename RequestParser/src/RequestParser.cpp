#include "RequestParser.hpp"

Request RequestParser::request;
RequestParser::BodyType RequestParser::bodyTemp;
const RequestParser::Space RequestParser::SPACE = SP_LITERAL;
const RequestParser::HttpVersion RequestParser::HTTPVERSION = HTTP_VERSION;


//util__
std::string RequestParser::ft_toLower(std::string str)
{
  for (unsigned int i = 0; i < str.length(); i++) {
    str[i] = std::tolower(str[i]);
  }
	return str;
}

std::string RequestParser::trimAll(std::string &str)
{
  std::string::size_type pos = str.find_last_not_of(" \t\r\n");
  if(pos != std::string::npos)
  {
      str.erase(pos + 1);
      pos = str.find_first_not_of(" \t\r\n");
      if(pos != std::string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
  return str;
}

//util__end



Request::FieldValueListType RequestParser::splitValue(std::string input, char delimiter) {
    Request::FieldValueListType result;
    std::stringstream ss(input);
    std::string temp;
 
    while (getline(ss, temp, delimiter)) {
      result.push_back(temp);
    }
    return result;
}

bool RequestParser::isMethod(std::string str)
{
  return (str == "GET" || str == "POST" || str == "DELETE");
}

bool RequestParser::isSpace(std::string str)
{
    return (SPACE.find(str) != std::string::npos);
}

bool RequestParser::isHttpVersion(std::string str)
{
    return (HTTPVERSION.find(str) != std::string::npos);
}

Request::MethodType  RequestParser::settingMethod(std::string str)
{
  if (str == "GET")
    return "GET";
  else if (str == "POST")
    return "POST";
  return "DELETE";
}

void RequestParser::queryStringSyntaxCheck(std::string uri)
{
  Request::UriListType uriList, queryStringList, keyValueList;
  std::string queryString;

  uriList = splitValue(uri, '?');
  if (uriList.size() != 2)
    throwError("400", "uri syntax error!");

  queryString = *(--uriList.end());
  if (queryString.find("=") == std::string::npos)
    throwError("400", "uri syntax error!");
  
  queryStringList = splitValue(queryString, '&');
  Request::UriListType::iterator it = queryStringList.begin();
  for(; it != queryStringList.end(); ++it)
  {
    keyValueList = splitValue((*it), '=');
    if (keyValueList.size() != 2 || (*keyValueList.begin()).length() == 0 || (*(++keyValueList.begin())).length() == 0)
      throwError("400", "queryString syntax error!");
    request.setQueryStringMap(*keyValueList.begin(), *(++keyValueList.begin()));
  }
  request.setUriPath(uri.substr(0, uri.length() - queryString.length() - 1));
  request.setQueryString(queryString);
}

bool RequestParser::isQueryString(std::string str)
{
  if (str.find("?") != std::string::npos)
    return true;
  else
    return false;  
}

void RequestParser::startLineValidity(Tokens &tokens)
{
  Token method = *(tokens.begin());
  tokens.pop_front();
  Token sp1 = *(tokens.begin());
  tokens.pop_front();
  Token uri = *(tokens.begin());
  tokens.pop_front();
  Token sp2 = *(tokens.begin());
  tokens.pop_front();
  Token httpVersion = *(tokens.begin());
  tokens.pop_front();
  if (!isMethod(method.second))
    throwError("400", "startline method syntax error!");
  if (!isSpace(sp1.second) || !isSpace(sp2.second))
    throwError("400", "startline SP syntax error!");
  if (isQueryString(uri.second))
    queryStringSyntaxCheck(uri.second);
  else
    request.setUriPath(uri.second);
  if (!isHttpVersion(httpVersion.second))
    throwError("505", "startline http version syntax error!");
  
  request.setMethod(settingMethod(method.second));
  request.setHttpVersion(httpVersion.second);
}

bool RequestParser::isFieldNameSpace(FieldName fieldName)
{
  SizeType pos = fieldName.find(" ");
  return pos != std::string::npos;
}

void RequestParser::hostPortCheck(Port port)
{
  int portNum = atoi(port.c_str());
  if (portNum == 0 || (portNum < 1024 || portNum > 49151))
    throwError("400", "host port syntax error!");
  request.setPort(port);
}

void RequestParser::hostValidity(FieldValue fieldValue)
{
  if (fieldValue.empty()) //fieldValue 비었는지 확인함.
    throwError("400", "field value empty!");
  if (request.getHost().size() > 0) //Host 2개 이상 들어왔는지 체크, 이미 Host에 넣었으면 size가 0이상
    throwError("400", "host field is allowed only one!");
  SizeType pos = fieldValue.find(":");
  SizeType len = fieldValue.length(); //naver.com:8000
  if (pos != std::string::npos)
    hostPortCheck(fieldValue.substr(pos + 1, len));
  std::string host = fieldValue.substr(0, pos);
  host = trimAll(host);
  request.setHost(host);
}

void RequestParser::transferEncodingValidity(FieldValue fieldValue)
{
  Request::FieldValueListType transferEncodingValue;
  transferEncodingValue = splitValue(fieldValue, ',');
  if (!transferEncodingValue.size())
    return ;
  Request::FieldValueListType::iterator it = transferEncodingValue.begin();
  int cnt = 1;
  int len = transferEncodingValue.size();
  int chunked = 0;
  std::string currStr;
  
  for (; it != transferEncodingValue.end(); ++it, cnt++)
  {
    currStr = trimAll((*it));
    if (cnt == 1 && currStr == "chunked") //처음에 오면 괜찮음.
      chunked = 1;
    if (chunked == 0 && (cnt > 1 && cnt < len)) // 중간에 chunked 나오면 에러처리
      throwError("400", "chunked syntax error!");
    if (chunked == 1 && cnt > 1 && currStr.length() > 0) //chunked가 1개가 아니거나, chunked이후에 다른 것이 나오면 문법오류
      throwError("400", "chunked syntax error!");
    if (chunked == 0 && cnt == len && currStr == "chunked") //마지막에 chunked있으면 ok
      chunked = 1;
  }
  if (chunked == 1)
    request.setChunked(true);
  request.setTransferEncoding(transferEncodingValue);
}

void RequestParser::contentLengthValidity(FieldValue fieldValue)
{
  Request::FieldValueListType contentLengthValue;
  contentLengthValue = splitValue(fieldValue, ',');
  if (!contentLengthValue.size())
    return ;
  Request::FieldValueListType::iterator it = contentLengthValue.begin();
  size_t contentLength = 0;
  size_t checkNum = atoi((*it).c_str());

  for (; it != contentLengthValue.end(); ++it)
  {
    contentLength = atoi((*it).c_str());
    if (!contentLength) //숫자 외에 다른 것이 써져 있으면 문법 오류
      throwError("400", "content length syntax error!");
    if (contentLength != checkNum) //뒤 숫자가 앞과 다르면 문법 오류
      throwError("400", "content length syntax error!");
  }
  request.setContentLength(contentLength);
  request.setHeaderFields("content-length","content-length_value");
}

void RequestParser::cookieValidity(FieldValue fieldValue)
{
  Request::FieldValueListType cookieValue, cookieKeyValue;
  cookieValue = splitValue(fieldValue, ';');
  if (!cookieValue.size())
    return ;
  Request::FieldValueListType::iterator it = cookieValue.begin();
  for (; it != cookieValue.end(); ++it)
  {
    cookieKeyValue = splitValue((*it), '=');
    if (cookieKeyValue.size() != 2) //=이 없거나, 1개 이상이면 문법 오류
      throwError("400", "cookie syntax error!");
    request.setCookie(trimAll(*(cookieKeyValue.begin())), trimAll(*(--cookieKeyValue.end())));
  }
  request.setCookieString(trimAll(fieldValue));
}

void RequestParser::extractMultipartFormDataId(FieldValue fieldValue)
{
  SizeType pos = fieldValue.find("boundary=");
  request.setMultipartFormDataId(fieldValue.substr(pos + 9, fieldValue.length()));
}

void RequestParser::contentTypeValidity(FieldValue fieldValue, HeaderType fieldValueType)
{
  if (fieldValueType == MULTIPART_FORM_DATA)
  {
    extractMultipartFormDataId(fieldValue);
    request.setChunked(false);
  }
  request.setContentType(trimAll(fieldValue));
}

void RequestParser::connectionValidity(FieldValue fieldValue)
{
  if (ft_toLower(trimAll(fieldValue)) == "close")
    request.setKeepAlive(false);
}

void RequestParser::mandatoryHeaderValidity(HeaderType fieldNameType, FieldName fieldName, HeaderType fieldValueType, FieldValue fieldValue)
{
  if (isFieldNameSpace(fieldName))
    throwError("400", "fieldname is not allowed SP!");
  switch (fieldNameType)
  {
    case HOST:
      hostValidity(fieldValue);
      break;
    case TRANSFER_ENCODING:
      transferEncodingValidity(fieldValue);
      break;
    case CONTENT_LENGTH:
      contentLengthValidity(fieldValue);
      break;
    case COOKIE:
      cookieValidity(fieldValue);
      break;
    case CONTENT_TYPE:
      contentTypeValidity(fieldValue, fieldValueType);
      break;
    case CONNECTION:
      connectionValidity(fieldValue);
      break;
    default:
      break;
  }
}

void RequestParser::nonMandatoryHeaderValidity(FieldName fieldName, FieldValue fieldValue)
{
  if (isFieldNameSpace(fieldName))
    throwError("400", "fieldname is not allowed SP!");
  request.setHeaderFields(fieldName, trimAll(fieldValue));
}

void RequestParser::headerCheck(Token &fieldNameToken, Token &fieldValueToken)
{
  if (HOST <= fieldNameToken.first && fieldNameToken.first <= CONTENT_TYPE)
    mandatoryHeaderValidity(fieldNameToken.first, fieldNameToken.second, fieldValueToken.first ,fieldValueToken.second);
  else
    nonMandatoryHeaderValidity(fieldNameToken.second, fieldValueToken.second);
}

bool RequestParser::isInMandatoryHeader(Tokens &tokens)
{
  int host = 0;

  Tokens::iterator it = tokens.begin();
  for(; it != tokens.end(); ++it)
  {
    if ((*it).first == HOST)
      host = 1;
  }
  if (host == 0)
    throwError("400", "host must be only one!");
  
  return true;
}

void RequestParser::chunkedContentLengthOverlapCheck(Tokens &tokens)
{
  int currTokenType;
  Tokens::iterator it = tokens.begin();
  if (!request.getChunked()) // chunked 아니면 early return;
    return ;
  for (; it != tokens.end(); ++it) // chunked이고 CONTENT_LENGTH가 있는 대상은 connection을 keep-alive가 아닌 close를 처리함.
  {
    currTokenType = (*it).first;
    if (currTokenType == CONTENT_LENGTH) {
      request.setKeepAlive(false);
    }
  }
}

void RequestParser::headerLineValidity(Tokens &tokens)
{
  Tokens::iterator fieldNameTokenIt, fieldValueTokenIt;
  int currTokenType;

  isInMandatoryHeader(tokens);
  Tokens::iterator it = tokens.begin();
  for (; it != tokens.end(); ++it)
  {
    currTokenType = (*it).first;
    if (FIELDNAME <= currTokenType && currTokenType <= CONTENT_TYPE){
      fieldNameTokenIt = (it);
      fieldValueTokenIt = (++it);
      if (fieldValueTokenIt == tokens.end())
        throwError("400", "header field syntax error!");
      headerCheck(*fieldNameTokenIt, *fieldValueTokenIt);
    }
  }
  chunkedContentLengthOverlapCheck(tokens);
}

void RequestParser::inputBodyData(Tokens &tokens)
{
  int currTokenType;

  Tokens::iterator it = tokens.begin();
  for(; it != tokens.end(); ++it)
  {
    currTokenType = (*it).first;
    if (currTokenType == BODY)
      request.setBody((*it).second);
  }
}

Request::ChunkedListType RequestParser::splitBodyData(std::string input) {
    Request::ChunkedListType result;
    std::stringstream ss(input);
    std::string temp;
    char checkN;
 
    while (getline(ss, temp, '\r')) {
      ss.get(checkN);
      if (checkN != '\n')
        throwError("400", "body syntax error!");
      result.push_back(temp);
    }
    return result;
}

double RequestParser::chunkedLengthConvert(std::string str)
{
  char *end;
  double num = strtol(str.c_str(), &end, 16);
  
  if (*end != '\0' && *end != ';')
    throwError("400", "chunked body number syntax error!");
  return num;
}


std::string	RequestParser::getBodyLine()
{	
	std::string::size_type pos = bodyTemp.find("\r\n");
	std::string	line;
	
	if (pos == std::string::npos)
	  return ("");
	line = bodyTemp.substr(0, pos);
  bodyTemp = bodyTemp.substr(pos + 2, bodyTemp.length());
	return line;
}


void RequestParser::chunkedProcess()
{
  double chunkedLength;
  bodyTemp = request.getBody();
  std::string bodyConverted;
  std::string line;

  while ((line = getBodyLine()).size() > 0)
  {  
    chunkedLength = chunkedLengthConvert(line);    
    if (chunkedLength == 0)
      break;
    line = getBodyLine();
    if ((line).length() != chunkedLength)
      throwError("400", "chunked body syntax error!");
    bodyConverted.append(line);
  }
  request.setBody(bodyConverted);
}

void RequestParser::multipartFormDataIdProcess()
{
  std::string bodyConverted, id, contentDisposition, contentType, emptyArea, bodyPiece, multipartFormDataId;

  bodyTemp = request.getBody();
  multipartFormDataId = request.getMultipartFormDataId();
  while ((id = getBodyLine()).size() > 0)
  {
    if (id == "--" + multipartFormDataId + "--") // end point
      break;

    if (id != "--" + multipartFormDataId)
      throwError("400", "multipart form data id error!");
    
    contentDisposition = getBodyLine();
    if (id.length() == 0 || ft_toLower(contentDisposition).find("content-disposition") != 0)
      throwError("400", "content-disposition error!");
    
    contentType = getBodyLine();
    if (contentType.length() == 0 || ft_toLower(contentType).find("content-type") != 0)
      throwError("400", "content-type error!");
 
    emptyArea = getBodyLine();
    if (!emptyArea.empty())
      throwError("400", "body sytax(multipart form) error!");
    
    bodyPiece = getBodyLine();
    if (bodyPiece.length() != 0)
      bodyConverted.append(bodyPiece);
  }
  request.setBody(bodyConverted);
}

void RequestParser::bodyLineValidity(Tokens &tokens)
{
  inputBodyData(tokens);
  if (request.getMultipartFormDataId().length() > 0)
    multipartFormDataIdProcess();
  if (request.getChunked())
    chunkedProcess();
  if (request.getHeaderFields().count("content-length") == 0)
    request.setContentLength(request.getBody().length());
}

void RequestParser::throwError(const char *code, std::string errorReason)
{
  std::cout << "Error : " << errorReason << std::endl;
  std::cout << "Error code : " << code << std::endl;
  throw code;
}

void RequestParser::errorHandling(const char *code)
{
  request.setErrorCode(code);
}

void RequestParser::previousErrorCheck(Tokens &tokens)
{
  if ((*tokens.begin()).first == ERROR)
    throwError((*tokens.begin()).second.c_str(), "previous(httpLexer) syntax error!");
}

Request RequestParser::httpParser(Tokens &tokens)
{
  Request requestNew;

  request = requestNew;
  try
  {
    previousErrorCheck(tokens);
    startLineValidity(tokens);
    headerLineValidity(tokens);
    bodyLineValidity(tokens);
  }
  catch(const char *code)
  {
    errorHandling(code);
  }

  return request;
}

Request RequestParser::startLineHeaderLineParsing(Tokens &tokens)
{
//   Request requestNew;

  // request = Request();	//TODO: check
  request.requestInitialize();
  try
  {
    previousErrorCheck(tokens);
    startLineValidity(tokens);
    headerLineValidity(tokens);
  }
  catch(const char *code)
  {
    errorHandling(code);
  }
  return request;
}

Request RequestParser::bodyLineParsing(Tokens &tokens, Request &inputRequest)
{
  request = inputRequest;

  try
  {
    previousErrorCheck(tokens);
    bodyLineValidity(tokens);
  }
  catch(const char *code)
  {
    errorHandling(code);
  }

  return request;
}

