#pragma once
#include <vector>
#include <string>
#include <map>
#include <variant>

constexpr char arithmetic_operations[] = "+-*/";
// TODO unary minus, perhaps time variable

enum class OperationType {
	Plus = '+',
	Minus = '-',
	Times = '*',
	Div = '/',
	Pow = '^',
	Abs = '|',
	Sign,
};

enum class OperandType {
	Result, // Result of previous operation
	Number,
	Variable,
	AuxVariable,
};

struct Operand {
	OperandType type;
	size_t idx{};
	double value{};
	std::string variable{};
};

struct Operation {
	OperationType type;
	// Indexes in operands vector
	std::vector<Operand> operands{};
};

constexpr char default_variable = 'x';

class VectorProcessor;

class FormulaProcessor {
private:
	char variable{default_variable};
	// If processor is trivial it contains just one operand
	Operand trivial_operand;

	std::vector<Operation> operations;

	Operand operand(const std::string &formula);
	Operation operation(OperationType op, std::string::size_type token_set,
	                    const std::string &formula);
	std::string::size_type find_next_token(const std::string &formula,
	                                       const std::string &token_set,
	                                       std::string::size_type pos = 0);
	bool inside_section(const std::string &formula, std::string::size_type pos);

public:
	FormulaProcessor(const std::string &formula);
	FormulaProcessor() = default;
	double operator()(const std::vector<double> &, VectorProcessor * = nullptr);
};

class VectorProcessor {
private:
	std::vector<FormulaProcessor> components;
	std::map<std::string, FormulaProcessor> aux_variables;

public:
	bool is_aux_var(const std::string &name) { return aux_variables.count(name); }
	std::map<std::string, double> variables; // Auxilliary stuff
	std::vector<double> operator()(const std::vector<double> &);
	void add_comp(const std::string &formula) { components.push_back(formula); }
	FormulaProcessor &operator[](const std::string &name)
	{
		return aux_variables[name];
	}

	VectorProcessor() = default;
	VectorProcessor(const VectorProcessor &) = default;
	VectorProcessor &operator=(const VectorProcessor &other) = default;
};
