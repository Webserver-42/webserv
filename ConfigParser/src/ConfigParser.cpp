#include "ConfigParser.hpp"
#include <sstream>
#include <string>
#include <limits>

Config ConfigParser::parse(const Tokens &tokens)
{
	Config			config;
	const_iterator	cIt = tokens.begin();
	const_iterator	cItEnd = tokens.end();

	while (cIt != cItEnd)
		parseCurrentToken(config, cIt, cItEnd);
	config.setUndeclaredDirectives();
	return (config);
}

ConfigParser::Number ConfigParser::convertToNumber(const NumericString &str)
{
	static std::stringstream	ss;
	Number						res;

	for (NumericString::const_iterator cIt = str.begin(); cIt != str.end(); cIt++)
		checkIfNumber(*cIt);
	ss << str;
	ss >> res;
	ss.str("");
	ss.clear();
	return (res);
}

void ConfigParser::checkIfNumberBetween(Number number, Number lower, Number upper)
{
	if (number < lower || upper < number)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
}

void ConfigParser::checkIfNumber(const char &c)
{
	checkIfNumberBetween(c, NUMBER_ZERO, NUMBER_NINE);
}

void ConfigParser::parseCurrentToken(Config &config, const_iterator &cIt, const_iterator &cItEnd)
{
	switch (cIt->first)
	{
		case TOKEN_SERVER:
			return (parseServer(config, ++cIt, cItEnd));
		case TOKEN_ERROR_PAGE:
			return (parseErrorPage(config, ++cIt));
		case TOKEN_CLIENT_MAX_BODY_SIZE:
			return(parseClientMaxBodySize(config, ++cIt));
		case TOKEN_AUTOINDEX:
			return (parseAutoIndex(config, ++cIt));
		case TOKEN_INDEX:
			return (parseIndex(config, ++cIt));
	}
	throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
}

ConfigParser::Argument ConfigParser::parseDirectiveOne(const_iterator &cIt)
{
	Argument	arg;

	if (cIt->first & TOKEN_ARGUMENT)
		arg.append((cIt++)->second);
	if (arg.empty() || (cIt++)->first != TOKEN_SEMICOLON)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	return (arg);
}

ConfigParser::ArgumentList ConfigParser::parseDirectiveMult(const_iterator &cIt)
{
	ArgumentList	list;

	while (cIt->first & TOKEN_ARGUMENT)
		list.push_back((cIt++)->second);
	if (list.empty() || (cIt++)->first != TOKEN_SEMICOLON)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	return (list);
}

void ConfigParser::parseServer(Config &config, const_iterator &cIt, const_iterator &cItEnd)
{
	ConfigServer	server;
	Key				key;
	
	if ((cIt++)->first != TOKEN_LBRACKET)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	while (cIt->first != TOKEN_RBRACKET && cIt != cItEnd)
		parseServerCurrentToken(server, cIt, cItEnd, key);
	if (cIt == cItEnd)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	cIt++;
	config.addConfigServer(key.serverNames, std::make_pair(key.host, key.port), server);
}

void ConfigParser::parseServerCurrentToken(ConfigServer &server, const_iterator &cIt, const_iterator &cItEnd, Key &key)
{
	switch (cIt->first)
	{
		case TOKEN_LOCATION:
			return (parseLocation(server, ++cIt, cItEnd));
		case TOKEN_SERVER_NAME:
			return (parseServerName(key, ++cIt));
		case TOKEN_LISTEN:
			return (parseListen(key, ++cIt));
		case TOKEN_REDIRECT:
			return (parseRedirect(server, ++cIt));
		case TOKEN_ERROR_PAGE:
			return (parseErrorPage(server, ++cIt));
		case TOKEN_CLIENT_MAX_BODY_SIZE:
			return (parseClientMaxBodySize(server, ++cIt));
		case TOKEN_AUTOINDEX:
			return (parseAutoIndex(server, ++cIt));
		case TOKEN_INDEX:
			return (parseIndex(server, ++cIt));
	}
	throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
}

void ConfigParser::parseLocation(ConfigServer &server, const_iterator &cIt, const_iterator &cItEnd)
{
	ConfigLocation	location;
	Route			route;
	
	if (cIt->first & TOKEN_ARGUMENT)
		route.append((cIt++)->second);
	if (route.empty() || (cIt++)->first != TOKEN_LBRACKET)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	while (cIt->first != TOKEN_RBRACKET && cIt != cItEnd)
		parseLocationCurrentToken(location, cIt);
	if (cIt == cItEnd)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	cIt++;
	server.addConfigLocation(route, location);
}

void ConfigParser::parseLocationCurrentToken(ConfigLocation &location, const_iterator &cIt)
{
	switch (cIt->first)
	{
		case TOKEN_ERROR_PAGE:
			return (parseErrorPage(location, ++cIt));
		case TOKEN_CLIENT_MAX_BODY_SIZE:
			return (parseClientMaxBodySize(location, ++cIt));
		case TOKEN_LIMIT_EXCEPT:
			return (parseLimitExcept(location, ++cIt));
		case TOKEN_REDIRECT:
			return (parseRedirect(location, ++cIt));
		case TOKEN_AUTOINDEX:
			return (parseAutoIndex(location, ++cIt));
		case TOKEN_INDEX:
			return (parseIndex(location, ++cIt));
		case TOKEN_ALIAS:
			return (parseAlias(location, ++cIt));
	}
	throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
}

void ConfigParser::parseServerName(Key &key, const_iterator &cIt)
{
	parseDirectiveMult(cIt).swap(key.serverNames);
}

// 1024 ~ 49151 are reserved for user server application.
// https://www.ibm.com/docs/en/ztpf/2020?topic=overview-port-numbers
void ConfigParser::parseListen(Key &key, const_iterator &cIt)
{
	std::stringstream	ss(parseDirectiveOne(cIt));
	std::string			host;
	std::string			port;

	std::getline(ss, host, COLON);
	checkHost(host);
	key.host = host;
	std::getline(ss, port, COLON);
	checkIfNumberBetween(convertToNumber(port), PORT_LOWER, PORT_UPPER);
	key.port = port;
}

void ConfigParser::checkHost(Host host)
{
	std::stringstream	ss(host);
	std::string			buf;
	int					count = 0;

	if (host == LOCALHOST_STR || host == ASTERISK_STR)
		return;
	for (; std::getline(ss, buf, DOT); count++)
		checkIfNumberBetween(convertToNumber(buf), OCTET_LOWER, OCTET_UPPER);
	if (count != 4)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
}

// nginx 기준 동일한 메소드가 여러 번 들어오는 걸 허용한다.
// 사용할 때 std::set으로 만들어서 사용하자.
void ConfigParser::parseLimitExcept(ConfigLocation &location, const_iterator &cIt)
{
	ArgumentList	list = parseDirectiveMult(cIt);
	
	for (ArgumentList::const_iterator cIt = list.begin(); cIt != list.end(); cIt++)
		checkLimitExcept(*cIt);
	location.setLimitExcept(list);
}

void ConfigParser::checkLimitExcept(const Argument &argument)
{
	if (argument != METHOD_GET && argument != METHOD_POST && argument != METHOD_DELETE)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
}

void ConfigParser::parseAlias(ConfigLocation &location, const_iterator &cIt)
{
	location.setAlias(parseDirectiveOne(cIt));
}

void ConfigParser::parseErrorPage(BaseBlock &block, const_iterator &cIt)
{
	ArgumentList					list = parseDirectiveMult(cIt);
	ArgumentList::const_iterator	pageIt = --(list.end());

	if (list.size() < 2)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	for (ArgumentList::const_iterator cIt = list.begin(); cIt != pageIt; cIt++)
	{
		checkIfNumberBetween(convertToNumber(*cIt), STATUS_CODE_LOWER, STATUS_CODE_UPPER);
		block.setErrorPage(*cIt, *pageIt);
	}
}

// std::stringstream으로 unsigned long long 타입에 값을 넣으면, 해당 타입의 범위를 넘은 값이 들어갈 경우 타입의 최대값을 넣는다.
void ConfigParser::parseClientMaxBodySize(BaseBlock &block, const_iterator &cIt)
{
	size_t	size = convertToNumber(parseDirectiveOne(cIt));

	if (size > std::numeric_limits<size_t>::max())
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	block.setClientMaxBodySize(size);
}

void ConfigParser::parseRedirect(InterBlock &block, const_iterator &cIt)
{
	ArgumentList	args = parseDirectiveMult(cIt);

	if (args.size() < 2)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	checkIfNumberBetween(convertToNumber(args[0]), REDIRECT_LOWER, REDIRECT_UPPER);
	if (args[1].find(HTTP) != 0 && args[1].find(HTTP_SECURE) != 0)
		throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
	block.setRedirect(args);
}

void ConfigParser::parseAutoIndex(BaseBlock &block, const_iterator &cIt)
{
	Argument	arg = parseDirectiveOne(cIt);

	if (arg == AUTOINDEX_ON)
		return (block.setAutoIndex(true));
	if (arg == AUTOINDEX_OFF)
		return (block.setAutoIndex(false));
	throw (std::invalid_argument(FILE_FORMAT_EXCEPT_MSG));
}

void ConfigParser::parseIndex(BaseBlock &block, const_iterator &cIt)
{
	block.setIndex(parseDirectiveMult(cIt));
}

ConfigParser::Key::Key() : host(ASTERISK_STR), port(PORT_DEFAULT_STR)
{
	serverNames.push_back("");
}
