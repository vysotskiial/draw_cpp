#pragma once
#include <vector>
#include <concepts>
#include <array>
#include <string>
#include <map>

constexpr char arithmetic_operations[] = "+-*/";

enum class OperationType {
	opPlus = '+',
	opMinus = '-',
	opTimes = '*',
	opDiv = '/',
	opVariable, // Just result to one of the input variables
	opPow,
	opAbs,
	opSign,
	opNumber,      // Result is a number
	opAuxVariable, // One of auxilliary variables
};

struct SingleOperation {
	OperationType type;
	// Indexes in operands vector
	std::vector<size_t> operand_indexes;
	size_t result_idx;
	std::string aux_var_name;
	double num{0}; // Just for opNumber
};

constexpr char default_variable = 'x';

class VectorProcessor;

class FormulaProcessor {
private:
	VectorProcessor *owner;
	int state_size;
	char variable{default_variable};
	std::vector<SingleOperation> operations;
	std::vector<double> operands;

	// void is_correct(const std::string &formula); // TODO maybe class? maybe in
	// main parser? It really throws some exception if formula is incorrect
	// so perhaps fine as is
	void parse_formula(const std::string &formula); // Assuming formula is correct
	void parse_separated(OperationType op, std::string::size_type token_set,
	                     const std::string &formula);
	std::string::size_type find_next_token(const std::string &formula,
	                                       const std::string &token_set,
	                                       std::string::size_type pos = 0);
	bool inside_brackets(const std::string &formula, std::string::size_type pos);

public:
	void set_owner(VectorProcessor *o) { owner = o; }
	FormulaProcessor(const std::string &formula, int args_num,
	                 VectorProcessor *o = nullptr);
	FormulaProcessor() {}
	double operator()(const std::vector<double> &);
};

class VectorProcessor {
private:
	int state_size;
	std::vector<FormulaProcessor> components;
	std::map<std::string, FormulaProcessor> aux_variables;

public:
	bool is_aux_var(const std::string &name) { return aux_variables.count(name); }
	std::map<std::string, double> variables; // Auxilliary stuff
	std::vector<double> operator()(const std::vector<double> &);
	VectorProcessor(const std::string &str);
	VectorProcessor(const VectorProcessor &other);
};
