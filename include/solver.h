#pragma once
#include <Eigen/Dense>
#include <functional>

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
	std::vector<Vector<size>> solve()
	{
		std::vector<Vector<size>> result;
		result.push_back(init_cond);
		for (auto i = 1; i <= step_num; i++)
			result.push_back(result.back() + right_part(result.back()) * step);

		return result;
	}
};
