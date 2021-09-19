#pragma once
#include <Eigen/Dense>
#include <functional>
#include <algorithm>

template<int size>
using Vector = Eigen::Matrix<double, size, 1>;

template<int size>
using RightPart = std::function<Vector<size>(const Vector<size> &)>;

template<int size>
class EulerSolver {
	double step;
	int step_num;
	Vector<size> init_cond;
	RightPart<size> right_part;

public:
	EulerSolver(double s, int n, const Vector<size> &i, const RightPart<size> &r)
	  : step(s), step_num(n), init_cond(i), right_part(r)
	{
	}
	std::vector<std::array<double, size>> solve()
	{
		std::vector<std::array<double, size>> result;
		std::array<double, size> current_res;
		auto current_state = init_cond;
		for (auto i = 1; i <= step_num; i++) {
			for (auto j = 0; j < size; j++)
				current_res[j] = current_state[j];
			result.push_back(current_res);
			current_state = current_state + right_part(current_state) * step;
		}

		return result;
	}
};
