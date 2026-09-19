#include <Lexer.hpp>

#include <iostream>
#include <set>

using std::string;
using std::vector;

const std::set<string, std::less<>> OPERATORS = {
	"+", "-", "*", "/",
	"=", "+=", "-=", "/=",
	"==", "<", ">", "<=", ">=",
	"=>", "(", ")",
	"[", "]", "&&", "||", ",",
	"=?", "!=?", "&&=?", "&&=", "||=",
	"^=", "->", "<-", "$", "~", "@",
	"||=?", "+?", "-?", "?",
	"+=?", "-=?", "=?", "/=?",
	"==?", "<?", ">?", "<=?", ">=?",
	"=>?", ";", "?", "+?", "-?", "*?", "/?"
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
	auto it = OPERATORS.lower_bound(op);

	if (it == OPERATORS.end())
		return false;

	if (*it == op)
		++it;

	return it != OPERATORS.end() && it->starts_with(op);
}

bool is_mergeable_op(const string& op)
{
	return is_operator(op);
}

vector<Token> Lexer::tokenize(const string& text)
{
	bool in_quote = false;
	bool is_escaped = false;
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

	for (size_t i = 0; i < tokens.size();)
	{
		const Token& current = tokens[i];

		// ==================================================
		// Max munch de operadores
		// ==================================================

		if (current.get_type() == OPERATOR)
		{
			string candidate = current.get_atom();

			size_t j = i + 1;

			string last_valid;

			if (is_operator(candidate))
				last_valid = candidate;

			while (j < tokens.size())
			{
				const Token& next = tokens[j];

				if (next.get_type() != OPERATOR)
					break;

				string next_candidate =
					candidate + next.get_atom();

				// Si deja de ser prefijo de algún operador,
				// ya no podemos continuar.
				if (!is_operator(next_candidate) &&
					!can_continue(next_candidate))
				{
					break;
				}

				candidate = next_candidate;

				if (is_operator(candidate))
					last_valid = candidate;

				j++;
			}

			// --------------------------------------------------
			// Nos quedamos con el último operador válido.
			// Esto es el corazón del max munch.
			// --------------------------------------------------

			if (!last_valid.empty())
			{
				Token tk(last_valid);
				tk.set_type(OPERATOR);

				merged_total.push_back(tk);

				// Cada carácter del operador proviene de
				// un token individual.
				size_t consumed = last_valid.size();

				i += consumed;
				continue;
			}
		}

		merged_total.push_back(current);
		i++;
	}

	// ==================================================
	// Unir + / - con números
	// ==================================================

	vector<Token> result;

	for (size_t i = 0; i < merged_total.size(); i++)
	{
		const Token& current = merged_total[i];

		if (
			(current.get_atom() == "-" ||
			 current.get_atom() == "+") &&
			current.get_type() == OPERATOR &&
			i + 1 < merged_total.size()
		)
		{
			const Token& next = merged_total[i + 1];

			if (next.get_type() == IDENTIFIER)
			{
				const string& atom = next.get_atom();

				bool possible_number = !atom.empty();

				for (char c : atom)
				{
					if (
						(c < '0' || c > '9') &&
						c != '.' &&
						c != '_'
					)
					{
						possible_number = false;
						break;
					}
				}

				if (possible_number)
				{
					Token tk(
						current.get_atom() + atom
					);

					tk.set_type(IDENTIFIER);

					result.push_back(tk);

					i++;
					continue;
				}
			}
		}

		result.push_back(current);
	}

	return result;
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
	if (m.empty())
		return false;

	size_t i = 0;

	// ==============================
	// Signo
	// ==============================

	if (m[i] == '+' || m[i] == '-')
	{
		i++;

		if (i == m.size())
			return false;
	}

	bool has_digit = false;
	bool has_dot = false;
	bool has_exponent = false;
	bool exponent_has_digit = false;

	for (; i < m.size(); i++)
	{
		char c = m[i];

		// ==============================
		// Dígito
		// ==============================

		if (c >= '0' && c <= '9')
		{
			has_digit = true;

			if (has_exponent)
				exponent_has_digit = true;

			continue;
		}

		// ==============================
		// Separador _
		// ==============================

		if (c == '_')
		{
			// No puede estar al inicio,
			// al final ni repetido.
			if (
				i == 0 ||
				i + 1 == m.size() ||
				m[i - 1] == '_' ||
				m[i + 1] == '_'
			)
			{
				return false;
			}

			continue;
		}

		// ==============================
		// Punto decimal
		// ==============================

		if (c == '.')
		{
			if (has_dot || has_exponent)
				return false;

			has_dot = true;
			continue;
		}

		// ==============================
		// Exponente
		// ==============================

		if (c == 'e' || c == 'E')
		{
			if (has_exponent || !has_digit)
				return false;

			has_exponent = true;
			exponent_has_digit = false;

			// e/E debe tener algo después.
			if (i + 1 == m.size())
				return false;

			// Permitir signo del exponente.
			if (
				m[i + 1] == '+' ||
				m[i + 1] == '-'
			)
			{
				i++;

				if (i + 1 == m.size())
					return false;
			}

			continue;
		}

		return false;
	}

	if (!has_digit)
		return false;

	if (has_exponent && !exponent_has_digit)
		return false;

	return true;
}

void Lexer::fix_types()
{
	for (Token& tk : this->tokens)
	{
		if (tk.get_type() != IDENTIFIER)
			continue;

		const string& atom = tk.get_atom();

		if (is_number(atom))
		{
			tk.set_type(VALUE);
		}
	}
}

vector<Token> Lexer::get_tokens()
{
	return this->tokens;
}

void Lexer::print()
{
	for (const Token& token : tokens)
	{
		std::cout
			<< std::to_string(token.get_type())
			<< " : "
			<< token.get_atom()
			<< "\n";
	}
}
