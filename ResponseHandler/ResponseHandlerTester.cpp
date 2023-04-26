#include "ResponseHandler.hpp"
#include "Response.hpp"
#include <iostream>
#include <stdio.h>

void responseTestSetting(Response &response)
{
	response.setStatusCode("200");
	response.setBody("this is test Body message.");
	response.setLocation("/testLocation");
	response.setCookie("id", "johnson");
	response.setCookie("pw", "abcd1234");
}

int main()
{
	Response response;
	ResponseHandler::ResponseMessageType responseString;

	responseTestSetting(response);
	responseString = ResponseHandler::createResponseMessage(response);
	std::cout << responseString << std::endl;

	return (0);
}

//c++ -Wall -Wextra -Werror -std=c++98 ResponseHandlerTester.cpp Response.cpp ResponseHandler.cpp StatusText.cpp -o responseHandlerTester
