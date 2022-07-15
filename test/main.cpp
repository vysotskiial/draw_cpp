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
constexpr double k = 1.;
constexpr double Delta = 0.4;

int sign(double x)
{
	auto result = (x > 0) ? 1 : -1;
	return result;
}

vector<double> rp(const vector<double> x, double power)
{
	vector<double> res{0, 0};
	if (x[0] <= Delta) {
		res[0] = x[1];
		res[1] = (x[1] > 0) ? xi + mu : -xi - mu;
	}
	else {
		res[0] = x[1] - k * sign(x[0] - Delta) * pow(abs(x[0] - Delta), power);
		res[1] = xi * sign(res[0]) - mu * copysign(1., x[0] - Delta);
	}
	return res;
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	/*
	auto r_part = [](const vector<double> &x) { return rp(x, 0.5); };
	EulerSolver solver(0.001, 4000, {0, 4}, r_part);
	auto start = chrono::high_resolution_clock::now();
	auto solution = solver.solve();
	auto stop = chrono::high_resolution_clock::now();

	cout << "Modeling with cpp function took "
	     << chrono::duration_cast<Sec>(stop - start).count() << '\n';
	*/

	auto eq = "v1 = x2 - 10*sign(x1)*pow(abs(x1), 0.5)\n"
	          "v1\n"
	          "sign(v1) - 2.5 * sign(x1)\n";

	MainWindow window(nullptr, {{{eq, "0, 35"}}, {{eq, "35, 0"}}});
	window.show();
	return app.exec();
}
