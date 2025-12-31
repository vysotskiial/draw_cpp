#pragma once
#include <set>
#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <string_view>
#include <format>

using namespace std::string_literals;

constexpr char arithmetic_operations[] = "+-*/";

enum class OperationType {
	Plus = '+',
	Minus = '-',
	Times = '*',
	Div = '/',
	Pow = '^',
	Abs = '|',
	Tern = '?', // ternary operator: a ? b : c
	Gr = '>',
	Ls = '<',
	And = 'A', // logical, &&
	Or = 'O',  // logical, ||
	Sign = 'S',
};

inline OperationType OType(const std::string_view &token)
{
	if (token.size() == 1)
		return static_cast<OperationType>(token[0]);

	if (token == "&&")
		return OperationType::And;
	if (token == "||")
		return OperationType::Or;
	throw std::runtime_error(std::format("Unknown operation token: {}", token));
}

enum class OperandType {
	NONE = -1,
	Result, // Result of previous operation
	Number,
	Variable,
	AuxVariable,
};

struct Operand {
	OperandType type;
	size_t idx{};
	double value{};
	std::string aux_variable{};
};

struct Operation {
	OperationType type;
	// Indexes in operands vector
	std::vector<Operand> operands{};
};

class VectorProcessor;

constexpr char default_variable[] = "x";

class FormulaProcessor {
private:
	VectorProcessor *owner;
	int is_component(std::string_view name) const;
	bool is_aux_var(const std::string &name) const;
	// If processor is trivial it contains just one operand
	Operand trivial_operand;

	std::vector<Operation> operations;

	Operand operand(std::string_view formula);
	Operation operation(OperationType op, std::string_view token,
	                    std::string::size_type pos, std::string_view formula);
	std::string::size_type find_next_token(std::string_view formula,
	                                       std::string_view token,
	                                       std::string::size_type pos = 0);

	bool inside_section(std::string_view formula, std::string::size_type pos);

public:
	FormulaProcessor(std::string formula, VectorProcessor * = nullptr);
	FormulaProcessor &operator=(std::string formula);
	FormulaProcessor() = default;
	double operator()(const std::vector<double> &);
};

class VectorProcessor {
	friend class FormulaProcessor;

private:
	std::vector<FormulaProcessor> components;
	std::map<std::string, FormulaProcessor> aux_variables;

	// we need to avoid circular aux dependencies
	// if we need unevaluated aux variable we add it to this set and evaluate
	// if it already is in the set => we have a circular dependency => error

	std::set<std::string> current_aux;
	std::map<std::string, double> calculated_aux;
	bool is_aux_var(std::string name) { return aux_variables.contains(name); }

public:
	std::vector<double> operator()(const std::vector<double> &);
	FormulaProcessor &operator[](size_t i);
	FormulaProcessor &operator[](const std::string &name);

	VectorProcessor() = default;
	VectorProcessor(const VectorProcessor &) = default;
	VectorProcessor &operator=(const VectorProcessor &other) = default;
};
