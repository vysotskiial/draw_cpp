#include <cmath>
#include <math.h>
#include "main_window.h"
#include "solver.h"
using namespace std;

constexpr double xi = 1.;
constexpr double mu = 2.;

Vector<2> right_part(const Vector<2> &x)
{
	Vector<2> res;
	res[0] = x[1] - copysign(1., x[0]) * sqrt(abs(x[0]));
	res[1] = xi * copysign(1., res[0]) - mu * copysign(1., x[0]);
	return res;
}

class SolveAndDraw : public wxApp {
	bool OnInit() override
	{
		MainWindow *main_window = new MainWindow("Picture");
		Vector<3> v;
		EulerSolver<2> solver(0.001, 100000, Vector<2>(5, 0),
		                      std::function(right_part));
		auto solution = solver.solve();
		std::vector<Point> points;
		for (auto &p : solution)
			points.push_back({p[0], p[1]});
		wxPen p("black", 2);
		Line l{p, points};
		main_window->picture_panel->add_line(l);
		main_window->Show();
		return true;
	}
};

IMPLEMENT_APP(SolveAndDraw)
