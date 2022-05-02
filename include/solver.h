#pragma once
#include <concepts>
#include <type_traits>
#include <vector>

// clang-format off
template<typename F>
concept right_part =
	std::invocable<F, std::vector<double>> &&
	std::is_same_v<std::vector<double>,
	               std::invoke_result_t<F &, std::vector<double>>>;

// clang-format on

template<right_part RightPart>
class EulerSolver {
	double step;
	int step_num;
	std::vector<double> init_cond;
	RightPart rp;

public:
	EulerSolver(double s, int n, const std::vector<double> &i, const RightPart &r)
	  : step(s), step_num(n), init_cond(i), rp(r)
	{
	}
	std::vector<std::vector<double>> solve()
	{
		std::vector<std::vector<double>> result;
		result.push_back(init_cond);
		for (auto i = 1; i <= step_num; i++) {
			auto next{result.back()};
			auto deriv = rp(result.back());
			for (auto i = 0u; i < next.size(); i++) {
				next[i] += deriv[i] * step;
			}
			result.push_back(next);
		}

		return result;
	}
};
