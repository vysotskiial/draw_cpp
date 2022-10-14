#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>
#include <sstream>
#include "formula_processor.h"

using namespace std;

FormulaProcessor::FormulaProcessor(const string &f, int args_num,
                                   VectorProcessor *o)
  : owner(o), state_size(args_num)
{
	auto formula = f;
	formula.erase(remove(formula.begin(), formula.end(), ' '), formula.end());
	operands.resize(args_num); // space for arguments
	parse_formula(formula);
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
void FormulaProcessor::parse_separated(OperationType type,
                                       string::size_type token_pos,
                                       const string &formula)
{
	SingleOperation op;
	op.type = type;
	operands.push_back(0); // result
	op.result_idx = operands.size() - 1;
	string::size_type prev_pos = 0;
	op.operand_indexes.push_back(operands.size());
	parse_formula(formula.substr(prev_pos, token_pos - prev_pos));

	while (token_pos != string::npos) {
		prev_pos = token_pos + 1;
		token_pos = find_next_token(formula, {formula[token_pos]}, prev_pos);
		op.operand_indexes.push_back(operands.size());
		parse_formula(formula.substr(prev_pos, token_pos - prev_pos));
	}
	operations.push_back(op);
}

void FormulaProcessor::parse_formula(const std::string &formula)
{
	// cerr << formula << '\n';
	auto pos = find_next_token(formula, "+-");
	if (pos != string::npos) {
		parse_separated(static_cast<OperationType>(formula[pos]), pos, formula);
		return;
	}
	pos = find_next_token(formula, "*/");
	if (pos != string::npos) {
		parse_separated(static_cast<OperationType>(formula[pos]), pos, formula);
		return;
	}

	if (formula.front() == '(' && formula.back() == ')') {
		parse_formula(formula.substr(1, formula.size() - 2));
		return;
	}

	if (formula.starts_with("pow(")) {
		auto f = formula.substr(4, formula.size() - 5); // strip pow()
		pos = find_next_token(f, ",");
		parse_separated(OperationType::opPow, pos, f);
		return;
	}

	if (formula.starts_with("abs(")) {
		auto f = formula.substr(4, formula.size() - 5); // strip abs()
		parse_separated(OperationType::opAbs, string::npos, f);
		return;
	}

	if (formula.starts_with("sign(")) {
		auto f = formula.substr(5, formula.size() - 6); // strip sign()
		parse_separated(OperationType::opSign, string::npos, f);
		return;
	}

	if (formula.starts_with(variable)) {
		auto idx = stoi(formula.substr(1));
		if (idx > state_size)
			throw runtime_error("State component outside of range used");
		operands.push_back(0);
		SingleOperation op;
		op.type = OperationType::opVariable;
		op.operand_indexes.push_back(idx - 1); // variables are numerated from 1
		op.result_idx = operands.size() - 1;
		operations.push_back(op);
		return;
	}

	if (owner && owner->is_aux_var(formula)) {
		operands.push_back(0);
		SingleOperation op;
		op.type = OperationType::opAuxVariable;
		op.result_idx = operands.size() - 1;
		op.aux_var_name = formula;
		operations.push_back(op);
		return;
	}

	// Should be just a number.
	operands.push_back(0);
	SingleOperation op;
	op.type = OperationType::opNumber;
	op.result_idx = operands.size() - 1;
	op.num = stod(formula);
	operations.push_back(op);
}

double FormulaProcessor::operator()(const vector<double> &args)
{
	copy(args.begin(), args.end(), operands.begin());
	for (auto op : operations) {
		// cerr << static_cast<int>(op.type) << " " << op.operand1_idx << '\n';
		switch (op.type) {
		case OperationType::opPlus:
			operands[op.result_idx] = 0;
			for (auto i : op.operand_indexes)
				operands[op.result_idx] += operands[i];
			break;
		case OperationType::opMinus:
			operands[op.result_idx] = operands[op.operand_indexes[0]];
			for (auto i = 1u; i < op.operand_indexes.size(); i++)
				operands[op.result_idx] -= operands[op.operand_indexes[i]];
			break;
		case OperationType::opTimes:
			operands[op.result_idx] = operands[op.operand_indexes[0]];
			for (auto i = 1u; i < op.operand_indexes.size(); i++)
				operands[op.result_idx] *= operands[op.operand_indexes[i]];
			break;
		case OperationType::opDiv:
			operands[op.result_idx] = operands[op.operand_indexes[0]];
			for (auto i = 1u; i < op.operand_indexes.size(); i++)
				operands[op.result_idx] /= operands[op.operand_indexes[i]];
			break;
		case OperationType::opVariable:
			operands[op.result_idx] = operands[op.operand_indexes[0]];
			break;
		case OperationType::opPow:
			operands[op.result_idx] =
			  pow(operands[op.operand_indexes[0]], operands[op.operand_indexes[1]]);
			break;
		case OperationType::opAbs:
			operands[op.result_idx] = abs(operands[op.operand_indexes[0]]);
			break;
		case OperationType::opSign:
			operands[op.result_idx] = (operands[op.operand_indexes[0]] > 0) ? 1 : -1;
			break;
		case OperationType::opNumber:
			operands[op.result_idx] = op.num;
			break;
		case OperationType::opAuxVariable:
			operands[op.result_idx] = owner->variables.at(op.aux_var_name);
			break;
		}
		// cout << operands[op.result_idx] << '\n';
	}
	return operands[operations.back().result_idx];
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
