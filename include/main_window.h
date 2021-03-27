#pragma once

#include <wx/wx.h>
#include <wx/panel.h>
#include <vector>

struct Grid {
	int x_max{10};
	int x_min{-10};
	int y_max{10};
	int y_min{-10};
	std::vector<int> x_ticks;
	std::vector<int> y_ticks;
};

class ControlPanel : public wxPanel {
public:
	ControlPanel(wxPanel *p);
	void on_save(wxCommandEvent &e);
	void on_zoom(wxCommandEvent &e);
	void on_set_coords(wxCommandEvent &event);
	wxButton *save_button;
	wxButton *zoom_button;
	wxStaticText *coords_text;
	wxPanel *parent;
};

class PicturePanel : public wxPanel {
private:
	wxPoint zoom_start;
	wxSize zoom_size;
	Grid grid;

	void draw_grid(wxPaintDC &);
	void draw_graphs(wxPaintDC &);
	void draw_zoom_rect(wxPaintDC &);

	wxPoint convert_coords(double x, double y, const wxDC &);

public:
	PicturePanel(wxPanel *parent);
	void on_paint(wxPaintEvent &e);
	void on_mouse_motion(wxMouseEvent &e);
	void on_mouse_leave(wxMouseEvent &e);
	void on_left_up(wxMouseEvent &e);
};

class MainWindow : public wxFrame {
public:
	MainWindow(const wxString &title);
	ControlPanel *control_panel;
	PicturePanel *picture_panel;
	wxPanel *parent_panel;
};
