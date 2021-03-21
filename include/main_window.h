#pragma once

#include <wx/wx.h>
#include <wx/panel.h>

class ControlPanel : public wxPanel {
public:
	ControlPanel(wxPanel *p);
	void on_save(wxCommandEvent &e);
	void on_zoom(wxCommandEvent &e) {}
	wxButton *save_button;
	wxButton *zoom_button;
	wxPanel *parent;
};

class PicturePanel : public wxPanel {
public:
	PicturePanel(wxPanel *parent);
	void on_paint(wxPaintEvent &e);
};

class MainWindow : public wxFrame {
public:
	MainWindow(const wxString &title);
	ControlPanel *control_panel;
	PicturePanel *picture_panel;
	wxPanel *parent_panel;
};
