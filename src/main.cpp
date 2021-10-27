#include "widgets.h"
#include <QApplication>
#include <QLineSeries>
#include <QSplineSeries>
#include <QValueAxis>
#include <chrono>
#include <iostream>
#include <qnamespace.h>
#include "solver.h"

using namespace QtCharts;
using namespace std;
using Sec = chrono::duration<double>;

constexpr double xi = 1.;
constexpr double mu = 2.;

Vector<2> right_part(const Vector<2> &x)
{
	Vector<2> res;
	res[0] = x[1] - copysign(1., x[0]) * sqrt(abs(x[0]));
	res[1] = xi * copysign(1., res[0]) - mu * copysign(1., x[0]);
	return res;
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	EulerSolver<2> solver(0.001, 100000, {0, 5}, std::function(right_part));
	auto solution = solver.solve();
	auto chart = new QChart();
	chart->legend()->hide();
	auto series = new QSplineSeries();

	auto pen = series->pen();
	pen.setWidth(2);
	series->setPen(pen);

	for (auto &value : solution)
		series->append({value[0], value[1]});

	chart->addSeries(series);

	auto x_axis = new QValueAxis();
	x_axis->setLinePen(series->pen());

	auto y_axis = new QValueAxis;
	y_axis->setLinePen(series->pen());

	chart->addAxis(x_axis, Qt::AlignBottom);
	chart->addAxis(y_axis, Qt::AlignLeft);

	series->attachAxis(x_axis);
	series->attachAxis(y_axis);
	x_axis->applyNiceNumbers();
	y_axis->applyNiceNumbers();

	MainWindow window(nullptr, chart);
	window.show();
	return app.exec();
}
