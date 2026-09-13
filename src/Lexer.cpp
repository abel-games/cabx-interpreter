#include <Lexer.hpp>

#include <iostream>
#include <set>

using std::string;
using std::vector;

const std::set<string> OPERATORS = {
	"+", "-", "*", "/",
	"=", "+=", "-=", "*=", "/=",
	"==", "<", ">", "<=", ">=",
	"=>", "(", ")",
	"[", "]", "&&", "||", ","
};

bool is_space(char c)
{
	return c == ' ';
}

bool is_operator(const string& c)
{
	return OPERATORS.contains(c);
}

bool is_separator(char c)
{
	static const std::set<char> separators = []()
	{
		std::set<char> result;

		for (const string& op : OPERATORS)
		{
			result.insert(op[0]);
		}

		return result;
	}();

	return separators.contains(c);
}

bool is_mergeable_op(const string& op)
{
	return is_operator(op);
}

vector<Token> Lexer::tokenize(const string& text)
{
	bool in_quote = false;
	bool in_one_quote = false;
	bool is_scaped = false;

	vector<string> total;
	vector<TokenType> types;
	string buffer;

	for (char c : text)
	{
		if (is_scaped)
		{
			buffer += c;
			is_scaped = false;
			continue;
		}

		if (c == '\\')
		{
			is_scaped = true;
			continue;
		}

		if (c == '"' && !in_one_quote)
		{
			in_quote = !in_quote;
			continue;
		}

		if (c == '\'' && !in_quote)
		{
			in_one_quote = !in_one_quote;
			continue;
		}

		if (in_quote || in_one_quote)
		{
			buffer += c;
			continue;
		}

		if (is_space(c))
		{
			if (!buffer.empty())
			{
				total.push_back(buffer);
				buffer.clear();
				if (in_quote || in_one_quote)
				{
					types.push_back(VALUE);
				}
				else
				{
					types.push_back(IDENTIFIER);
				}
			}

			continue;
		}

		if (is_separator(c))
		{
			if (!buffer.empty())
			{
				total.push_back(buffer);
				buffer.clear();
				if (in_quote || in_one_quote)
				{
					types.push_back(VALUE);
				}
				else
				{
					types.push_back(IDENTIFIER);
				}
			}

			buffer += c;
			total.push_back(buffer);
			buffer.clear();
			types.push_back(OPERATOR);

			continue;
		}

		buffer += c;
	}

	if (is_scaped)
	{
		std::cout << "Escapaste un caracter en blanco\n";
		return vector<Token>();
	}

	if (in_quote || in_one_quote)
	{
		std::cout << "Comillas sin cerrar\n";
		return vector<Token>();
	}

	if (!buffer.empty())
	{
		total.push_back(buffer);
		types.push_back(IDENTIFIER);
	}

    #ifndef NDEBUG
    	std::cout << "Aca ya termino el formado de tokens y sus tipos\n";
    #endif

    vector<Token> result;
    for (size_t i = 0; i < total.size(); i++)
    {
    	Token token(total[i]);
    	token.set_type(types[i]);

    	result.push_back(token);
    }

    return result;
}

vector<Token> Lexer::merge_tokens(const vector<Token>& tokens)
{
	vector<Token> merged_total;

	for (size_t i = 0; i < tokens.size(); i++)
	{
		const Token& current = tokens[i];
		const string& atom = current.get_atom();
		const TokenType type = current.get_type();

		if (i + 1 < tokens.size())
		{
			string combined = atom + tokens[i + 1].get_atom();
			TokenType newt = tokens[i + 1].get_type();
			
			if (is_mergeable_op(combined))
			{
			    Token tk(combined);
			    tk.set_type(newt);
				merged_total.push_back(tk);
				i++;
				continue;
			}
		}

		merged_total.push_back(current);
	}

	return merged_total;
}

void Lexer::lex(const string& text)
{
	vector<Token> raw_tokens = tokenize(text);
	vector<Token> merged_tokens = this->merge_tokens(raw_tokens);

	this->tokens = merged_tokens;
}

void Lexer::print()
{
	for (const Token& token : tokens)
	{
		std::cout << std::to_string(token.get_type()) << " : " << token.get_atom() << "\n";
	}
}

