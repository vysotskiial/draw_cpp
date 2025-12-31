#include <algorithm>
#include <cassert>
#include <cmath>
#include <print>
#include "formula_processor.h"

using namespace std;

FormulaProcessor::FormulaProcessor(string formula, VectorProcessor *vp)
  : owner(vp)
{
	formula.erase(remove(formula.begin(), formula.end(), ' '), formula.end());
	auto op = operand(formula);
	// Means formula is trivial i.e. no operations there
	if (op.type != OperandType::Result)
		trivial_operand = op;
}

FormulaProcessor &FormulaProcessor::operator=(string formula)
{
	operations.clear();
	formula.erase(remove(formula.begin(), formula.end(), ' '), formula.end());
	auto op = operand(formula);
	// Means formula is trivial i.e. no operations there
	if (op.type != OperandType::Result)
		trivial_operand = op;
	return *this;
}

// Is position inside either () or ||
bool FormulaProcessor::inside_section(string_view f, string::size_type pos)
{
	auto opened_cnt = count(f.begin(), f.begin() + pos, '(');
	auto closed_cnt = count(f.begin(), f.begin() + pos, ')');
	auto modulus_cnt = count(f.begin(), f.begin() + pos, '|');

	return (opened_cnt != closed_cnt) || (modulus_cnt % 2);
}

string::size_type FormulaProcessor::find_next_token(string_view formula,
                                                    string_view token,
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
Operation FormulaProcessor::operation(OperationType type, string_view token,
                                      string::size_type token_pos,
                                      string_view formula)
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

Operand FormulaProcessor::operand(string_view formula)
{
	if (formula[0] == '-') {
		// -formula == 0 - formula => create that operation
		Operand zero = {.type = OperandType::Number, .value = 0.};
		Operand other = operand(formula.substr(1));
		operations.push_back({OperationType::Minus, {zero, other}});
		return Operand{OperandType::Result, operations.size() - 1};
	}

	// Condition operators are supposed to be isolated for there is no
	// way of determining where they end
	auto pos_question = find_next_token(formula, "?");
	if (pos_question != string::npos) {
		auto pos_colon = find_next_token(formula, ":");
		if (pos_colon == string::npos)
			throw runtime_error("Incomplete ternary operator");
		auto cond_op = operand(formula.substr(0, pos_question));
		auto true_op =
		  operand(formula.substr(pos_question + 1, pos_colon - pos_question - 1));
		auto false_op = operand(formula.substr(pos_colon + 1));
		operations.push_back({OperationType::Tern, {cond_op, true_op, false_op}});
		return {OperandType::Result, operations.size() - 1};
	}

	auto pos_cmp = find_next_token(formula, "<");
	pos_cmp = (pos_cmp != string::npos) ? pos_cmp : find_next_token(formula, ">");
	if (pos_cmp != string::npos) {
		auto lhs = operand(formula.substr(0, pos_cmp));
		auto rhs = operand(formula.substr(pos_cmp + 1));
		operations.push_back({OType(formula.substr(pos_cmp, 1)), {lhs, rhs}});
		return {OperandType::Result, operations.size() - 1};
	}

	auto add_arithmetic = [&formula, this](string_view token) {
		auto tpos = find_next_token(formula, token);
		if (tpos != string::npos) {
			operations.push_back(operation(OType(token), token, tpos, formula));
			return Operand{OperandType::Result, operations.size() - 1};
		}
		return Operand{OperandType::NONE};
	};

	for (auto &token : {"&&", "||", "+", "-", "*", "/", "^"}) {
		auto op = add_arithmetic(token);
		if (op.type != OperandType::NONE)
			return op;
	}

	if (formula.front() == '(' && formula.back() == ')')
		return operand(formula.substr(1, formula.size() - 2));

	if (formula.front() == '|' && formula.back() == '|') {
		auto f = formula.substr(1, formula.size() - 2);
		operations.push_back(Operation{OperationType::Abs, {operand(f)}});

		return {.type = OperandType::Result, .idx = operations.size() - 1};
	}

	// TODO more functions with some cool template/std::func mappings
	if (formula.starts_with("sign(")) {
		auto f = formula.substr(5, formula.size() - 6); // strip sign()
		operations.push_back(Operation{OperationType::Sign, {operand(f)}});
		return {.type = OperandType::Result, .idx = operations.size() - 1};
	}

	auto idx = is_component(formula);
	if (idx >= 0)
		return {.type = OperandType::Variable, .idx = size_t(idx)};

	char *num_end;
	auto num_value = strtod(begin(formula), &num_end);
	if (num_end == end(formula))
		return {.type = OperandType::Number, .value = num_value};

	if (!isalpha(formula[0]))
		throw std::invalid_argument("Invalid operand: " + string{formula});

	return {.type = OperandType::AuxVariable, .aux_variable = string{formula}};
}

double FormulaProcessor::operator()(const vector<double> &args)
{
	vector<double> results;
	auto value = [&args, &results, this](const Operand &o) {
		switch (o.type) {
		case OperandType::AuxVariable:
			if (!is_aux_var(o.aux_variable))
				throw runtime_error("Aux variable failure for " + o.aux_variable);
			if (owner->calculated_aux.contains(o.aux_variable))
				return owner->calculated_aux[o.aux_variable];
			if (owner->current_aux.contains(o.aux_variable))
				throw runtime_error("Circular definition of aux variable " +
				                    o.aux_variable);
			owner->current_aux.insert(o.aux_variable);
			owner->calculated_aux[o.aux_variable] =
			  owner->aux_variables[o.aux_variable](args);
			return owner->calculated_aux[o.aux_variable];
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
	current_aux.clear();
	calculated_aux.clear();

	for (auto &component : components)
		result.push_back(component(args));

	return result;
}

FormulaProcessor &VectorProcessor::operator[](size_t i)
{
	assert(i && "Vector processor uses indexing with i > 0");
	if (components.size() < i)
		components.resize(i, {""s, this});
	return components[i - 1];
}

FormulaProcessor &VectorProcessor::operator[](const std::string &name)
{
	if (!aux_variables.contains(name))
		aux_variables.insert({name, {"", this}});
	return aux_variables[name];
}

int FormulaProcessor::is_component(string_view name) const
{
	if (!name.starts_with(default_variable))
		return -1;

	auto num_sv = name.substr(sizeof(default_variable) - 1);
	char *num_end;
	auto num = strtol(begin(num_sv), &num_end, 10);
	if (num_end != end(num_sv))
		return -1;
	return num - 1;
}

bool FormulaProcessor::is_aux_var(const string &name) const
{
	if (!owner)
		return false;
	return owner->is_aux_var(name);
}
