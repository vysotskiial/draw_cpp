#include "widgets.h"
#include <QApplication>
#include <chrono>
#include <iostream>
#include <cmath>
#include "solver.h"
#include <formula_processor.h>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainWindow window(nullptr, {});
	window.show();
	return app.exec();
}
