#include <Lexer.hpp>

#include <iostream>
#include <string>

int main()
{
	std::string texto;

	while (std::getline(std::cin, texto))
	{
		Lexer lexer;

		lexer.lex(texto);
		lexer.print();
	}

	return 0;
}