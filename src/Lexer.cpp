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
	bool is_escaped = false;
	bool in_one_quote = false;
	char quote_type = '\0';

	vector<string> total;
	vector<TokenType> types;

	string buffer;

	for (char c : text)
	{
		// ==============================
		// Escape
		// ==============================

		if (is_escaped)
		{
			buffer += '\\';
			buffer += c;

			is_escaped = false;
			continue;
		}

		if (c == '\\' && in_quote)
		{
			is_escaped = true;
			continue;
		}

		// ==============================
		// Dentro de string/char
		// ==============================

		if (in_quote)
		{
			if (c == quote_type)
			{
				in_quote = false;

				total.push_back(buffer);
				types.push_back(STRING);

				buffer.clear();
				continue;
			}

			buffer += c;
			continue;
		}

		// ==============================
		// Inicio de string/char
		// ==============================

		if (c == '"' || c == '\'')
		{
			in_quote = true;
			quote_type = c;

			buffer.clear();
			continue;
		}

		// ==============================
		// Espacios
		// ==============================

		if (is_space(c))
		{
			if (!buffer.empty())
			{
				total.push_back(buffer);
				types.push_back(IDENTIFIER);
				buffer.clear();
			}

			continue;
		}

		// ==============================
		// Separadores
		// ==============================

		if (is_separator(c))
		{
			if (!buffer.empty())
			{
				total.push_back(buffer);
				types.push_back(IDENTIFIER);
				buffer.clear();
			}

			total.push_back(string(1, c));
			types.push_back(OPERATOR);

			continue;
		}

		// ==============================
		// Texto normal
		// ==============================

		buffer += c;
	}

	// ==============================
	// Buffer final
	// ==============================

	if (!buffer.empty())
	{
		total.push_back(buffer);
		types.push_back(IDENTIFIER);
	}

	vector<Token> tokens;

	for (size_t i = 0; i < total.size(); i++)
	{
		Token tk(total[i]);
		tk.set_type(types[i]);
		tokens.push_back(tk);
	}

	return tokens;
}

vector<Token> Lexer::merge_tokens(const vector<Token>& tokens)
{
	vector<Token> merged_total;

	for (size_t i = 0; i < tokens.size(); i++)
	{
		const Token& current = tokens[i];

		if (
			(current.get_atom() == "-" || current.get_atom() == "+") &&
			i + 1 < tokens.size()
		)
		{
			const string& next = tokens[i + 1].get_atom();

			bool valid_number_part = !next.empty();

			for (char c : next)
			{
				if (
					(c < '0' || c > '9') &&
					c != '.' &&
					c != '_'
				)
				{
					valid_number_part = false;
					break;
				}
			}

			if (valid_number_part)
			{
				Token tk(current.get_atom() + next);
				tk.set_type(IDENTIFIER);

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
	this->fix_types();
}

bool is_number(const string& m)
{
	bool marked = false;
	bool negative = false;
	bool still_num = true;
	for (size_t i = 0; i < m.size(); i++)
	{
		const char c = m[i];
		if (i == 0)
		{
			if (c == '-')
			{
				negative = true;
			}
			if (c == '+')
			{
				negative = false;
			}
			marked = true;
			continue;
		}
		if ((c >= '0' && c <= '9') || c == '.' || c == '_')
		{
			if (still_num)
			{
				still_num = true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			still_num = false;
		}
	}
	return still_num;
}

void Lexer::fix_types()
{
	for (Token& tk : this->tokens)
	{
		const string& atom = tk.get_atom();
		if (is_number(atom))
		{
			tk.set_type(VALUE);
		}
	}
}

void Lexer::print()
{
	for (const Token& token : tokens)
	{
		std::cout << std::to_string(token.get_type()) << " : " << token.get_atom() << "\n";
	}
}

