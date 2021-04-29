#include "main_window.h"

class DrawApp : public wxApp {
public:
	bool OnInit() override
	{
		MainWindow *main_window = new MainWindow(wxT("Picture"));
		std::vector<Point> points{{-1, -1}, {0, 0}, {3, 2}};
		Line l{*wxBLACK_PEN, points};
		main_window->picture_panel->add_line(l);
		main_window->Show();

		return true;
	}
};
IMPLEMENT_APP(DrawApp)
