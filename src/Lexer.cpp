#include <Lexer.hpp>

#include <iostream>
#include <set>

using std::string;
using std::vector;

const std::set<string, std::less<>> OPERATORS = {
	"+", "-", "*", "/",
	"=", "+=", "-=", "*=", "/=",
	"==", "<", ">", "<=", ">=",
	"=>", "(", ")",
	"[", "]", "&&", "||", ",",
	"=?", "!=?", "&&=?", "&&=", "||=",
	"^=", "->", "<-", "$", "~", "@",
	"||=?", "+?", "-?", "*?", "?",
	"+=?", "-=?", "*=?", "/=?",
	"==?", "<?", ">?", "<=?", ">=?",
	"=>?", "?"
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

bool can_continue(const string& op)
{
	auto begin = OPERATORS.lower_bound(op);

	for (auto it = begin; it != OPERATORS.end(); ++it)
	{
		if (!it->starts_with(op))
			break;

		if (*it != op)
			return true;
	}

	return false;
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
				types.push_back(IDENTIFIER);
			}

			continue;
		}

		if (is_separator(c))
		{
			if (!buffer.empty())
			{
				total.push_back(buffer);
				buffer.clear();
				types.push_back(IDENTIFIER);
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
	vector<Token> result;

	for (size_t i = 0; i < tokens.size();)
	{
		const Token& current = tokens[i];

		if (current.get_type() != OPERATOR)
		{
			result.push_back(current);
			i++;
			continue;
		}

		string op = current.get_atom();
		size_t end = i + 1;

		while (end < tokens.size())
		{
			const Token& next = tokens[end];

			if (next.get_type() != OPERATOR)
				break;

			string candidate = op + next.get_atom();

			if (is_operator(candidate))
			{
				op = candidate;
				end++;
				continue;
			}

			if (can_continue(candidate))
			{
				op = candidate;
				end++;
				continue;
			}

			break;
		}

		if (!is_operator(op))
		{
			// No debería ocurrir si tokenize() produjo operadores
			// válidos individualmente.
			result.push_back(current);
			i++;
			continue;
		}

		Token token(op);
		token.set_type(OPERATOR);
		result.push_back(token);

		i = end;
	}

	return result;
}

void Lexer::lex(const string& text)
{
	vector<Token> raw_tokens = tokenize(text);
	vector<Token> merged = this->merge_tokens(raw_tokens);

	this->tokens = merged;

//	this->fix_types();
}

void Lexer::print()
{
	for (const Token& token : tokens)
	{
		std::cout << std::to_string(token.get_type())
			<< " : " << token.get_atom() << "\n";
	}
}