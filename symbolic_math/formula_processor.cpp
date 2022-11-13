#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>
#include <sstream>
#include "formula_processor.h"

using namespace std;

FormulaProcessor::FormulaProcessor(const string &f, size_t args_num,
                                   VectorProcessor *o)
  : owner(o), state_size(args_num)
{
	auto formula = f;
	formula.erase(remove(formula.begin(), formula.end(), ' '), formula.end());
	auto operand = parse_formula(formula);
	// Means formula is trivial i.e. no operations there
	if (operand.type != OperandType::Result) {
		trivial = true;
		trivial_operand = operand;
	}
}

bool FormulaProcessor::inside_brackets(const string &f, string::size_type pos)
{
	auto opened = count(f.begin(), f.begin() + pos, '(');
	auto closed = count(f.begin(), f.begin() + pos, ')');

	return (opened != closed);
}

string::size_type FormulaProcessor::find_next_token(
  const string &formula, const std::string &token_set, string::size_type pos)
{
	auto token_pos = formula.find_first_of(token_set, pos + 1);

	while (token_pos != string::npos) {
		if (!inside_brackets(formula, token_pos))
			return token_pos;
		token_pos = formula.find_first_of(token_set, token_pos + 1);
	}

	return token_pos;
}

// Parsing operation arguments separated with specific token
// Works for operations with single argument if token_pos == npos
Operation FormulaProcessor::parse_separated(OperationType type,
                                            string::size_type token_pos,
                                            const string &formula)
{
	Operation op;
	op.type = type;
	results.push_back(0);
	op.result_idx = results.size() - 1;
	string::size_type prev_pos = 0;
	op.operands.push_back(
	  parse_formula(formula.substr(prev_pos, token_pos - prev_pos)));

	while (token_pos != string::npos) {
		prev_pos = token_pos + 1;
		token_pos = find_next_token(formula, {formula[token_pos]}, prev_pos);
		op.operands.push_back(
		  parse_formula(formula.substr(prev_pos, token_pos - prev_pos)));
	}
	return op;
}

Operand FormulaProcessor::parse_formula(const std::string &formula)
{
	// cerr << formula << '\n';
	auto pos = find_next_token(formula, "+-");
	if (pos != string::npos) {
		operations.push_back(
		  parse_separated(static_cast<OperationType>(formula[pos]), pos, formula));
		return {OperandType::Result, operations.back().result_idx};
	}
	pos = find_next_token(formula, "*/");
	if (pos != string::npos) {
		operations.push_back(
		  parse_separated(static_cast<OperationType>(formula[pos]), pos, formula));
		return {OperandType::Result, operations.back().result_idx};
	}

	if (formula.front() == '(' && formula.back() == ')') {
		return parse_formula(formula.substr(1, formula.size() - 2));
	}

	if (formula.starts_with("pow(")) {
		auto f = formula.substr(4, formula.size() - 5); // strip pow()
		pos = find_next_token(f, ",");
		operations.push_back(parse_separated(OperationType::Pow, pos, f));
		return {OperandType::Result, operations.back().result_idx};
	}

	if (formula.starts_with("abs(")) {
		auto f = formula.substr(4, formula.size() - 5); // strip abs()
		operations.push_back(parse_separated(OperationType::Abs, string::npos, f));
		return {OperandType::Result, operations.back().result_idx};
	}

	if (formula.starts_with("sign(")) {
		auto f = formula.substr(5, formula.size() - 6); // strip sign()
		operations.push_back(parse_separated(OperationType::Sign, string::npos, f));
		return {OperandType::Result, operations.back().result_idx};
	}

	if (formula.starts_with(variable)) {
		auto idx = stoul(formula.substr(1));
		if (idx > state_size)
			throw runtime_error("State component outside of range used");
		return {OperandType::Variable, idx - 1};
	}

	if (owner && owner->is_aux_var(formula))
		return {OperandType::AuxVariable, formula};

	// Should be just a number.
	return {OperandType::Number, stod(formula)};
}

double FormulaProcessor::operator()(const vector<double> &args)
{
	auto value = [&args, this](const Operand &o) {
		switch (o.type) {
		case OperandType::AuxVariable:
			return owner->variables.at(get<string>(o.value));
		case OperandType::Number:
			return get<double>(o.value);
		case OperandType::Result:
			return results[get<size_t>(o.value)];
		case OperandType::Variable:
			return args[get<size_t>(o.value)];
		}
		throw runtime_error("Nono");
	};

	if (trivial)
		return value(trivial_operand);

	for (auto operation : operations) {
		// cerr << static_cast<int>(op.type) << " " << op.operand1_idx << '\n';
		auto &operands = operation.operands;
		switch (operation.type) {
		case OperationType::Plus:
			results[operation.result_idx] = 0;
			for (auto operand : operands)
				results[operation.result_idx] += value(operand);
			break;
		case OperationType::Minus:
			results[operation.result_idx] = value(operands[0]);
			for (auto i = 1u; i < operands.size(); i++)
				results[operation.result_idx] -= value(operands[i]);
			break;
		case OperationType::Times:
			results[operation.result_idx] = 1;
			for (auto operand : operands)
				results[operation.result_idx] *= value(operand);
			break;
		case OperationType::Div:
			results[operation.result_idx] = value(operands[0]);
			for (auto i = 1u; i < operands.size(); i++)
				results[operation.result_idx] /= value(operands[i]);
			break;
		case OperationType::Pow:
			results[operation.result_idx] =
			  pow(value(operands[0]), value(operands[1]));
			break;
		case OperationType::Abs:
			results[operation.result_idx] = abs(value(operands[0]));
			break;
		case OperationType::Sign:
			results[operation.result_idx] = value(operands[0]) > 0 ? 1 : -1;
			break;
		}
		// cout << operands[op.result_idx] << '\n';
	}
	return results[operations.back().result_idx];
}

VectorProcessor::VectorProcessor(const string &str)
{
	stringstream sstream(str);
	std::vector<string> lines;
	while (sstream.good()) {
		lines.push_back("");
		getline(sstream, lines.back());
		lines.back().erase(remove(lines.back().begin(), lines.back().end(), ' '),
		                   lines.back().end());
	}
	if (lines.back() == "")
		lines.pop_back();

	state_size = count_if(lines.begin(), lines.end(), [](const std::string &l) {
		return l.find("=") == string::npos;
	});
	if (!state_size)
		throw runtime_error("No state equations provided");

	for (auto &line : lines) {
		auto formula = line;
		formula.erase(remove(formula.begin(), formula.end(), ' '), formula.end());
		auto pos = formula.find("=");
		if (pos != string::npos) { // auxilliary variable definition
			auto var_name = formula.substr(0, pos);
			if (!isalpha(var_name[0]))
				throw runtime_error("Variable name is not an identifier"s + var_name);
			if (aux_variables.count(var_name))
				throw runtime_error("Variable is defined twice"s + var_name);
			aux_variables[var_name] = {formula.substr(pos + 1), state_size, this};
		}
		else {
			components.push_back({formula, state_size, this});
		}
	}
}

VectorProcessor::VectorProcessor(const VectorProcessor &other)
{
	for (auto &component : other.components) {
		components.push_back(component);
		components.back().set_owner(this);
	}

	for (auto &[name, var_form] : other.aux_variables) {
		aux_variables[name] = var_form;
		aux_variables[name].set_owner(this);
	}
}

vector<double> VectorProcessor::operator()(const vector<double> &args)
{
	vector<double> result;
	for (auto &[name, aux_proc] : aux_variables) {
		variables[name] = aux_proc(args);
	}

	for (auto &component : components) {
		result.push_back(component(args));
	}

	return result;
}
