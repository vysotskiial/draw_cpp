#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>
#include <sstream>
#include "formula_processor.h"

using namespace std;

FormulaProcessor::FormulaProcessor(const string &f, size_t args_num,
                                   VectorProcessor *o)
  : owner(o)
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
		operands.push_back(0);
		SingleOperation op;
		op.type = OperationType::opVariable;
		op.operand_indexes.push_back(idx - 1); // variables are numerated from 1
		op.result_idx = operands.size() - 1;
		operations.push_back(op);
		return;
	}

	if (formula.starts_with(default_aux_variable)) {
		auto idx = stoi(formula.substr(1));
		operands.push_back(0);
		SingleOperation op;
		op.type = OperationType::opAuxVariable;
		op.operand_indexes.push_back(idx);
		op.result_idx = operands.size() - 1;
		operations.push_back(op);
		return;
	}

	// Should be just a number.
	operands.push_back(0);
	SingleOperation op{OperationType::opNumber,
	                   {},
	                   operands.size() - 1,
	                   stod(formula)};
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
			operands[op.result_idx] = owner->variables.at(op.operand_indexes[0]);
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

	size_t eq_num = count_if(lines.begin(), lines.end(), [](const string &s) {
		return !s.starts_with(default_aux_variable);
	});

	for (auto &line : lines) {
		auto formula = line;
		formula.erase(remove(formula.begin(), formula.end(), ' '), formula.end());
		if (formula.starts_with(default_aux_variable)) {
			auto pos = formula.find("=");
			if (pos != string::npos) {
				auto v_num = formula.substr(1, pos - 1);
				variable_formulas[stoi(v_num)] = {formula.substr(pos + 1), eq_num,
				                                  this};
				continue;
			}
		}
		components.push_back({formula, eq_num, this});
	}
}

VectorProcessor::VectorProcessor(const VectorProcessor &other)
{
	for (auto &component : other.components) {
		components.push_back(component);
		components.back().set_owner(this);
	}

	for (auto &[idx, var_form] : other.variable_formulas) {
		variable_formulas[idx] = var_form;
		variable_formulas[idx].set_owner(this);
	}
}

vector<double> VectorProcessor::operator()(const vector<double> &args)
{
	vector<double> result;
	for (auto &[idx, aux_proc] : variable_formulas) {
		variables[idx] = aux_proc(args);
	}

	for (auto &component : components) {
		result.push_back(component(args));
	}

	return result;
}
