#pragma once
#include <vector>
#include <string>
#include <map>
#include <variant>

constexpr char arithmetic_operations[] = "+-*/";

enum class OperationType {
	Plus = '+',
	Minus = '-',
	Times = '*',
	Div = '/',
	Pow = '^',
	Abs,
	Sign,
};

enum class OperandType {
	Result, // Result of previous operation
	Number, // Number
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
	std::vector<Operand> operands;
	size_t result_idx;
};

constexpr char default_variable = 'x';

class VectorProcessor;

class FormulaProcessor {
private:
	VectorProcessor *owner;
	size_t state_size;
	char variable{default_variable};
	// If processor is trivial it contains just one operand
	bool trivial{false};
	Operand trivial_operand;
	std::vector<Operation> operations;
	std::vector<double> results;

	Operand parse_formula(const std::string &formula);
	Operation parse_separated(OperationType op, std::string::size_type token_set,
	                          const std::string &formula);
	std::string::size_type find_next_token(const std::string &formula,
	                                       const std::string &token_set,
	                                       std::string::size_type pos = 0);
	bool inside_brackets(const std::string &formula, std::string::size_type pos);

public:
	void set_owner(VectorProcessor *o) { owner = o; }
	FormulaProcessor(const std::string &formula, size_t args_num,
	                 VectorProcessor *o = nullptr);
	FormulaProcessor(){};
	double operator()(const std::vector<double> &);
};

class VectorProcessor {
private:
	size_t state_size;
	std::vector<FormulaProcessor> components;
	std::map<std::string, FormulaProcessor> aux_variables;

public:
	bool is_aux_var(const std::string &name) { return aux_variables.count(name); }
	std::map<std::string, double> variables; // Auxilliary stuff
	std::vector<double> operator()(const std::vector<double> &);
	VectorProcessor(const std::string &str);
	VectorProcessor(const VectorProcessor &other);
};
