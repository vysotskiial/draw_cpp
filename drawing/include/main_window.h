#pragma once

#include <wx/wx.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "control_panel.h"
#include "picture_panel.h"

class MainWindow : public wxFrame {
public:
	MainWindow(const wxString &title);
	ControlPanel *control_panel;
	PicturePanel *picture_panel;
	wxPanel *parent_panel;
};
