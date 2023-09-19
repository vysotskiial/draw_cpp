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

string::size_type FormulaProcessor::find_next_token(const string &formula,
                                                    const std::string &token,
                                                    string::size_type pos)
{
	auto token_pos = formula.find(token, pos + 1);

	while (token_pos != string::npos) {
		if (!inside_section(formula, token_pos))
			return token_pos;
		token_pos = formula.find(token, token_pos + 1);
	}

	return token_pos;
}

// Parsing operation arguments separated with specific token
// Works for operations with single argument if token_pos == npos
Operation FormulaProcessor::operation(OperationType type, const string &token,
                                      string::size_type token_pos,
                                      const string &formula)
{
	Operation op{type};
	string::size_type prev_pos = 0;
	op.operands.push_back(
	  operand(formula.substr(prev_pos, token_pos - prev_pos)));

	while (token_pos != string::npos) {
		prev_pos = token_pos + token.size();
		token_pos = find_next_token(formula, token, prev_pos);
		op.operands.push_back(
		  operand(formula.substr(prev_pos, token_pos - prev_pos)));
	}
	return op;
}

Operand FormulaProcessor::operand(const std::string &formula)
{
	// cerr << formula << '\n';
	if (formula[0] == '-')
		return operand("0" + formula);

	// Condition operators are supposed to be isolated for there is no
	// way of determining where they end
	auto pos = find_next_token(formula, "?");
	if (pos != string::npos) {
		auto pos1 = find_next_token(formula, ":");
		if (pos1 == string::npos)
			throw runtime_error("Incomplete ternary operator");
		auto cond_op = operand(formula.substr(0, pos));
		auto true_op = operand(formula.substr(pos + 1, pos1 - pos));
		auto false_op = operand(formula.substr(pos1 + 1));
		operations.push_back({OperationType::Tern, {cond_op, true_op, false_op}});
		return {OperandType::Result, operations.size() - 1};
	}

	pos = find_next_token(formula, "<");
	pos = (pos != string::npos) ? pos : find_next_token(formula, ">");
	if (pos != string::npos) {
		auto lhs = operand(formula.substr(0, pos));
		auto rhs = operand(formula.substr(pos + 1));
		operations.push_back({OType({formula[pos]}), {lhs, rhs}});
		return {OperandType::Result, operations.size() - 1};
	}

	auto add_arithmetic = [&formula, this](const std::string &token) {
		auto tpos = find_next_token(formula, token);
		if (tpos != string::npos) {
			operations.push_back(operation(OType(token), token, tpos, formula));
			return Operand{OperandType::Result, operations.size() - 1};
		}
		return Operand{OperandType::NONE};
	};

	for (auto &token : {"&&"s, "||"s, "+"s, "-"s, "*"s, "/"s, "^"s}) {
		auto op = add_arithmetic(token);
		if (op.type != OperandType::NONE)
			return op;
	}

	if (formula.front() == '(' && formula.back() == ')') {
		return operand(formula.substr(1, formula.size() - 2));
	}

	if (formula.front() == '|' && formula.back() == '|') {
		auto f = formula.substr(1, formula.size() - 2);
		operations.push_back(Operation{OperationType::Abs, {operand(f)}});

		return {OperandType::Result, operations.size() - 1};
	}

	if (formula.starts_with("sign(")) {
		auto f = formula.substr(5, formula.size() - 6); // strip sign()
		operations.push_back(Operation{OperationType::Sign, {operand(f)}});
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
		case OperationType::Or:
			results.push_back(false);
			for (auto operand : operands)
				results.back() = results.back() || value(operand);
			break;
		case OperationType::And:
			results.push_back(true);
			for (auto operand : operands)
				results.back() = results.back() && value(operand);
			break;
		case OperationType::Tern:
			results.push_back(value(operands[0]) ? value(operands[1]) :
			                                       value(operands[2]));
			break;
		case OperationType::Gr:
			results.push_back(value(operands[0]) > value(operands[1]));
			break;
		case OperationType::Ls:
			results.push_back(value(operands[0]) < value(operands[1]));
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
