#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>
#include <sstream>
#include "formula_processor.h"

using namespace std;

FormulaProcessor::FormulaProcessor(const string &f)
{
	auto formula = f;
	formula.erase(remove(formula.begin(), formula.end(), ' '), formula.end());
	auto op = operand(formula);
	// Means formula is trivial i.e. no operations there
	if (op.type != OperandType::Result)
		trivial_operand = op;
}

// Is position inside either () or ||
bool FormulaProcessor::inside_section(const string &f, string::size_type pos)
{
	auto opened_cnt = count(f.begin(), f.begin() + pos, '(');
	auto closed_cnt = count(f.begin(), f.begin() + pos, ')');
	auto modulus_cnt = count(f.begin(), f.begin() + pos, '|');

	return (opened_cnt != closed_cnt) || (modulus_cnt % 2);
}

string::size_type FormulaProcessor::find_next_token(
  const string &formula, const std::string &token_set, string::size_type pos)
{
	auto token_pos = formula.find_first_of(token_set, pos + 1);

	while (token_pos != string::npos) {
		if (!inside_section(formula, token_pos))
			return token_pos;
		token_pos = formula.find_first_of(token_set, token_pos + 1);
	}

	return token_pos;
}

// Parsing operation arguments separated with specific token
// Works for operations with single argument if token_pos == npos
Operation FormulaProcessor::operation(OperationType type,
                                      string::size_type token_pos,
                                      const string &formula)
{
	Operation op{type};
	string::size_type prev_pos = 0;
	op.operands.push_back(
	  operand(formula.substr(prev_pos, token_pos - prev_pos)));

	while (token_pos != string::npos) {
		prev_pos = token_pos + 1;
		token_pos = find_next_token(formula, {formula[token_pos]}, prev_pos);
		op.operands.push_back(
		  operand(formula.substr(prev_pos, token_pos - prev_pos)));
	}
	return op;
}

Operand FormulaProcessor::operand(const std::string &formula)
{
	// cerr << formula << '\n';
	auto pos = find_next_token(formula, "+-");
	if (pos != string::npos) {
		operations.push_back(
		  operation(static_cast<OperationType>(formula[pos]), pos, formula));
		return {OperandType::Result, operations.size() - 1};
	}
	pos = find_next_token(formula, "*/");
	if (pos != string::npos) {
		operations.push_back(
		  operation(static_cast<OperationType>(formula[pos]), pos, formula));
		return {OperandType::Result, operations.size() - 1};
	}
	pos = find_next_token(formula, "^");
	if (pos != string::npos) {
		operations.push_back(operation(OperationType::Pow, pos, formula));
		return {OperandType::Result, operations.size() - 1};
	}

	if (formula.front() == '(' && formula.back() == ')') {
		return operand(formula.substr(1, formula.size() - 2));
	}

	if (formula.front() == '|' && formula.back() == '|') {
		auto f = formula.substr(1, formula.size() - 2);
		operations.push_back(operation(OperationType::Abs, string::npos, f));

		return {OperandType::Result, operations.size() - 1};
	}

	if (formula.starts_with("sign(")) {
		auto f = formula.substr(5, formula.size() - 6); // strip sign()
		operations.push_back(operation(OperationType::Sign, string::npos, f));
		return {OperandType::Result, operations.size() - 1};
	}

	if (formula.starts_with(variable)) {
		auto idx = stoul(formula.substr(1));
		return {OperandType::Variable, idx - 1};
	}

	// Should be a number or aux variable.
	try {
		return {OperandType::Number, 0, stod(formula)};
	}
	catch (std::exception &e) {
	}

	return {OperandType::AuxVariable, 0, 0, formula};
}

double FormulaProcessor::operator()(const vector<double> &args,
                                    VectorProcessor *vp)
{
	vector<double> results;
	auto value = [&args, &results, &vp](const Operand &o) {
		switch (o.type) {
		case OperandType::AuxVariable:
			if (!vp || !vp->is_aux_var(o.variable)) {
				cerr << o.variable << '\n';
				throw runtime_error("Aux variable failure");
			}
			return vp->variables.at(o.variable);
		case OperandType::Number:
			return o.value;
		case OperandType::Result:
			return results[o.idx];
		case OperandType::Variable:
			return args.at(o.idx);
		default:
			throw runtime_error("Bad operand");
		}
	};

	if (!operations.size())
		return value(trivial_operand);

	for (auto operation : operations) {
		// cerr << static_cast<int>(op.type) << " " << op.operand1_idx << '\n';
		auto &operands = operation.operands;
		switch (operation.type) {
		case OperationType::Plus:
			results.push_back(0);
			for (auto operand : operands)
				results.back() += value(operand);
			break;
		case OperationType::Minus:
			results.push_back(value(operands[0]));
			for (auto j = 1u; j < operands.size(); j++)
				results.back() -= value(operands[j]);
			break;
		case OperationType::Times:
			results.push_back(1);
			for (auto operand : operands)
				results.back() *= value(operand);
			break;
		case OperationType::Div:
			results.push_back(value(operands[0]));
			for (auto j = 1u; j < operands.size(); j++)
				results.back() /= value(operands[j]);
			break;
		case OperationType::Pow:
			results.push_back(pow(value(operands[0]), value(operands[1])));
			break;
		case OperationType::Abs:
			results.push_back(abs(value(operands[0])));
			break;
		case OperationType::Sign:
			results.push_back(value(operands[0]) > 0 ? 1 : -1);
			break;
		}
		// cout << operands[op.result_idx] << '\n';
	}
	return results.back();
}

vector<double> VectorProcessor::operator()(const vector<double> &args)
{
	vector<double> result;
	for (auto &[name, aux_proc] : aux_variables) {
		variables[name] = aux_proc(args, this);
	}

	for (auto &component : components) {
		result.push_back(component(args, this));
	}

	return result;
}
