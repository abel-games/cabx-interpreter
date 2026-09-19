#pragma once

#include <string>
#include <vector>

enum TokenType {
	UKNOWN,
	IDENTIFIER,
	VALUE,
	STRING,
	OPERATOR
};

class Token
{
	private:
	std::string atom;
	TokenType type = UKNOWN;
	public:
	Token(const std::string& tatom)
	{
		this->atom = tatom;
	}

	const std::string& get_atom() const
	{
		return atom;
	}

	const TokenType get_type() const
	{
		return type;
	}
	void set_type(TokenType ttype)
	{
		this->type = ttype;
	}
};

class Lexer
{
private:
	std::vector<Token> tokens;

public:
	std::vector<Token> tokenize(const std::string& text);
	std::vector<Token> merge_tokens(
		const std::vector<Token>& tok
	);

	void lex(const std::string& text);
	void print();
	void fix_types();
	std::vector<Token> get_tokens();
};
