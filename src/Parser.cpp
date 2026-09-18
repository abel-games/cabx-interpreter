#include <Parser.hpp>

#include <iostream>
#include <utility>
#include <set>

using std::string;
using std::vector;

std::set<string> Commands = {
	"log", "add", "sub", "div", "mul"
};

Expression::Expression(
	const string& atom,
	vector<std::unique_ptr<Expression>> children
)
	: atom(atom),
	  children(std::move(children))
{
}


const string& Expression::get_atom() const
{
	return atom;
}


vector<std::unique_ptr<Expression>>& Expression::get_children()
{
	return children;
}


void Expression::add_child(Expression child)
{
	children.push_back(
		std::make_unique<Expression>(
			std::move(child)
		)
	);
}


void Expression::free_children()
{
	children.clear();
}


void Expression::set_children(
	vector<std::unique_ptr<Expression>> new_children
)
{
	children = std::move(new_children);
}


void Expression::print(int depth) const
{
	for (int i = 0; i < depth; ++i)
	{
		std::cout << "\t";
	}

	std::cout << atom << "\n";

	for (const auto& child : children)
	{
		child->print(depth + 1);
	}
}


void Expression::print() const
{
	print(0);
}

Expression* Expression::get_parent(Expression& root)
{
	for (const Expression* child : root.get_children())
	{
		if (child.get() == this)
		{
			return root;
		}
		else
		{
			auto result = child->get_parent(root);
			if (result != nullptr)
			{
				return result;
			}
		}
	}
	return nullptr;
}
				
void Parser::make_ast(const vector<Token>& tks)
{
	Expression total("root");

	Expression* actual = &total;

	for (const Token& tk : tks)
	{
		const string& atom = tk.get_atom();
		TokenType type = tk.get_type();

		(void)type;
 if (atom == ";")
		{
			actual=actual->get_parent(root);
		}
		if (Commands.contains(atom))
		{
			actual->add_child(
				Expression(atom)
			);

			actual =
				actual->get_children().back().get();
		}
		else
		{
			actual->add_child(
				Expression(atom)
			);
		}
	}

	ast.push_back(
		std::move(total)
	);
}


void Parser::parse(const vector<Token>& tks)
{
	make_ast(tks);
}


vector<Expression> Parser::get_parsed()
{
	return std::move(ast);
}


void Parser::print() const
{
	for (const Expression& expression : ast)
	{
		expression.print();
	}
}