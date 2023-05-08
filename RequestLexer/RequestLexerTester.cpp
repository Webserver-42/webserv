#include "RequestLexer.hpp"
#include "Request.hpp"
#include <iostream>
#include <stdio.h>

void printTokens(RequestLexer::Tokens tokens)
{
	RequestLexer::Tokens::iterator it = tokens.begin();
	int cnt = 0;
	for (;it != tokens.end(); ++it)
	{
		printf("token[%d](Lexeme, value)>> %d, \"%s\"\n", cnt, (*it).first, (*it).second.c_str());
		cnt++;
	}
}

int main()
{
	std::string test = "GET /index.html HTTP/1.1\r\nHost: github.co.kr\r\nConnection: keep-alive\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/111.0.0.0 Safari/537.36\r\nAccept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8\r\nReferer: http://github.co.kr/\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7\r\n\r\n[Full request URI: http://github.co.kr/index.html][HTTP request 1/1][Response in frame: 24808]";
	// std::string test = "ERRORTEST";
	RequestLexer::Tokens tokens = RequestLexer::httpTokenize(test);
	printTokens(tokens);
	return (0);
}