#include <Lexer.hpp>
#include <Parser.hpp>

#include <iostream>
#include <string>

int main()
{
	std::string texto;

	while (std::getline(std::cin, texto))
	{
		Lexer lexer;
		Parser parser;
		lexer.lex(texto);
		auto tok = lexer.get_tokens();
		parser.parse(tok);
		lexer.print();
		parser.print();
	}

	return 0;
}