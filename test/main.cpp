#include "widgets.h"
#include <QApplication>
#include <chrono>
#include <iostream>
#include <cmath>
#include "solver.h"
#include <formula_processor.h>

using namespace QtCharts;
using namespace std;
using Sec = chrono::duration<double>;

constexpr double xi = 1.;
constexpr double mu = 2.5;
constexpr double k = 10.;

int sign(double x)
{
	return (x > 0) ? 1 : -1;
}

vector<double> rp(const vector<double> x, double power)
{
	vector<double> res{0, 0};
	res[0] = x[1] - k * sign(x[0]) * pow(abs(x[0]), power);
	res[1] = xi * sign(res[0]) - mu * copysign(1., x[0]);
	return res;
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	auto r_part = [](const vector<double> &x) { return rp(x, 0.5); };
	EulerSolver solver(0.001, 100000, {0, 45}, r_part);
	auto start = chrono::high_resolution_clock::now();
	auto solution = solver.solve();
	auto stop = chrono::high_resolution_clock::now();

	cout << "Modeling with cpp function took "
	     << chrono::duration_cast<Sec>(stop - start).count() << '\n';

	auto chart = make_chart(solution, 0, 1);

	auto eq = "v1 = x2 - 10*sign(x1)*pow(abs(x1), 0.5)\n"
	          "v1\n"
	          "sign(v1) - 2.5 * sign(x1)\n";
	VectorProcessor vp(eq);
	EulerSolver solver2(0.001, 100000, {0, 45}, vp);
	start = chrono::high_resolution_clock::now();
	auto solution2 = solver2.solve();
	stop = chrono::high_resolution_clock::now();
	cout << "Modeling with fancy shmancy symbolic function took "
	     << chrono::duration_cast<Sec>(stop - start).count() << '\n';

	auto second = make_chart(solution2, 0, 1);

	MainWindow window(nullptr, {{chart}, {second, eq}});
	window.show();
	return app.exec();
}
