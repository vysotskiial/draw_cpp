#include "main_window.h"

class DrawApp : public wxApp {
public:
	bool OnInit() override
	{
		MainWindow *main_window = new MainWindow(wxT("Picture"));
		main_window->Show();

		return true;
	}
};
IMPLEMENT_APP(DrawApp)
