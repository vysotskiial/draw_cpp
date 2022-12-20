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
[[maybe_unused]] constexpr double k = 1.;
constexpr double Delta = 1.;
constexpr double Epsilon = 0.0001;

int sign(double x)
{
	auto result = (x > 0) ? 1 : -1;
	return result;
}

vector<double> rp(const vector<double> x, double power)
{
	vector<double> res{0, 0};
	auto delta =
	  (abs(x[0]) < Delta) ? -x[0] - Epsilon * sign(x[0]) : -Delta * sign(x[0]);
	res[0] = x[1] - sign(x[0] + delta) * pow(abs(x[0] + delta), power);
	res[1] = xi * sign(res[0]) - mu * sign(x[0] + delta);
	return res;
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	auto r_part = [](const vector<double> &x) { return rp(x, 0.5); };
	EulerSolver solver(0.001, 10000, {0, 15}, r_part);
	auto solution = solver.solve();
	auto series = new QSplineSeries();
	auto pen = series->pen();
	pen.setWidth(2);
	pen.setColor("red");
	series->setPen(pen);
	for (auto i = 0; i < 10000; i++) {
		series->append({solution[i][0], solution[i][1]});
	}

	auto eq = "v1 = x2 - 10*sign(x1)*pow(abs(x1), 0.5)\n"
	          "v1\n"
	          "sign(v1) - 2.5 * sign(x1)\n";

	auto start = chrono::high_resolution_clock::now();
	EulerSolver solver1(0.001, 10000, {0, 15}, VectorProcessor(eq));
	auto solution1 = solver1.solve();
	auto stop = chrono::high_resolution_clock::now();

	cout << "Modeling with cpp function took "
	     << chrono::duration_cast<Sec>(stop - start).count() << '\n';

	MainWindow window(nullptr, {{series}}, {{{eq, "0, 35"}}});
	window.show();
	return app.exec();
}
