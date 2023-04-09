#include "Lexer.hpp"
#include <stdexcept>

Lexer::Lexer(const char *filePath) : FILEPATH(filePath), WHITESPACES(WHITESPACES_LITERAL), BRACKET_SEMICOLON(BRACKET_SEMICOLON_LITERAL)
{
	configFile.open(FILEPATH.c_str());	
}

void Lexer::validateFileOpen()
{
	if (!configFile.good())
		throw (std::invalid_argument(FILEPATH_EXCEPT_MSG));
}

bool Lexer::isNotDelimiter(char c)
{
    return (WHITESPACES.find(c) == std::string::npos && BRACKET_SEMICOLON.find(c) == std::string::npos);
}

bool Lexer::isWhiteSpaces(char c)
{
    return (WHITESPACES.find(c) != std::string::npos);
}

bool Lexer::isBracketOrSemicolon(Delimiter delimiter)
{
	return (BRACKET_SEMICOLON.find(delimiter) != std::string::npos);
}

void Lexer::processToken(Tokens &tokens)
{
	Lexeme			lexeme;
	std::filebuf	*fileBuf = configFile.rdbuf();
	char			currentChar = fileBuf->sgetc();

	while (isWhiteSpaces(currentChar))
		currentChar = fileBuf->snextc();
	while (currentChar != EOF && isNotDelimiter(currentChar))
	{
		lexeme.push_back(currentChar);
		currentChar = fileBuf->snextc();
	}
	fileBuf->sbumpc();
	if (currentChar == EOF)
		configFile.setstate(std::ios_base::eofbit);
	addToken(tokens, lexeme, Lexeme(1, currentChar));
}

void Lexer::addToken(Tokens &tokens, Lexeme lexeme, Lexeme delimiter)
{
	if (lexeme.size())		
		tokens.push_back(std::make_pair(evaluateLexeme(lexeme), lexeme));
	if (isBracketOrSemicolon(delimiter))
		tokens.push_back(std::make_pair(evaluateDelimiterLexeme(delimiter), ""));
}

Lexer::TokenType Lexer::evaluateLexeme(const Lexeme &lexeme)
{
	static Syntax		syntax(initializeSyntax());
	Syntax::iterator	pos = syntax.find(lexeme);

	if (pos != syntax.end())
		return (pos->second);
	return (TOK_ARGUMENT);
}

Lexer::Syntax	Lexer::initializeSyntax()
{
	Syntax	syntax;

	syntax[SERVER] = TOK_SERVER;
	syntax[LOCATION] = TOK_LOCATION;
	syntax[LISTEN] = TOK_DIRECTIVE_ONE_ARG;
	syntax[SERVER_NAME] = TOK_DIRECTIVE_MUL_ARGS;
	syntax[ERROR_PAGE] = TOK_DIRECTIVE_ERR_PAGE;
	syntax[CLIENT_MAX_BODY_SIZE] = TOK_DIRECTIVE_ONE_ARG;
	syntax[LIMIT_EXCEPT] = TOK_DIRECTIVE_MUL_ARGS;
	syntax[REDIRECT] = TOK_DIRECTIVE_ONE_ARG;
	syntax[ALIAS] = TOK_DIRECTIVE_ONE_ARG;
	syntax[AUTOINDEX] = TOK_DIRECTIVE_ONE_ARG;
	syntax[INDEX] = TOK_DIRECTIVE_MUL_ARGS;
	return (syntax);
}

Lexer::TokenType Lexer::evaluateDelimiterLexeme(const Delimiter &delimiter)
{
	if (delimiter == LEFT_BRACKET)
		return (TOK_LBRACKET);
	if (delimiter == RIGHT_BRACKET)
		return (TOK_RBRACKET);
	return (TOK_SEMICOLON);
}

Lexer::Tokens Lexer::tokenize()
{
	Tokens	tokens;

	validateFileOpen();
	while (!configFile.eof())
		processToken(tokens);
	configFile.close();
	return (tokens);
}
