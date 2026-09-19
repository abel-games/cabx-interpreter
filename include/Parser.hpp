#pragma once

#include <string>
#include <vector>
#include <memory>

#include <Lexer.hpp>

class Expression
{
private:
	std::string atom;
	std::vector<std::unique_ptr<Expression>> children;

public:
	Expression(
		const std::string& atom,
		std::vector<std::unique_ptr<Expression>> children = {}
	);

	const std::string& get_atom() const;

	std::vector<std::unique_ptr<Expression>>& get_children();

	Expression* get_parent(Expression& root);
	
	void add_child(Expression child);

	void free_children();

	void set_children(
		std::vector<std::unique_ptr<Expression>> children
	);

	void print() const;

private:
	void print(int depth) const;
};

class Parser
{
private:
	std::vector<Expression> ast;

public:
	void make_ast(const std::vector<Token>& tks);

	void print() const;

	void parse(const std::vector<Token>& tks);

	std::vector<Expression> get_parsed();
};