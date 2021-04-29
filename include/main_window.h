#pragma once

#include <wx/gdicmn.h>
#include <wx/wx.h>
#include <wx/panel.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

struct Grid {
	double x_max{};
	double x_min{};
	double y_max{};
	double y_min{};

	bool manual_border{false};
	bool manual_ticks{false};
	std::vector<double> x_ticks;
	std::vector<double> y_ticks;
	void set_ticks() {}
};

struct Point {
	double x;
	double y;
	operator std::string()
	{
		std::stringstream ss;
		ss << std::setprecision(3) << x << ";" << y;
		return ss.str();
	}
};

struct Line {
	wxPen pen;
	std::vector<Point> points;
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
	std::vector<Line> lines;                       // always the same
	std::vector<std::vector<wxPoint>> pixel_lines; // changes on resize and zoom

	void draw_grid(wxPaintDC &);
	void draw_graphs(wxPaintDC &);
	void draw_zoom_rect(wxPaintDC &);
	void update_lines();
	void finalize_grid();

	wxPoint to_pixel(Point p);
	Point to_point(wxPoint p);

public:
	PicturePanel(wxPanel *parent);
	void on_paint(wxPaintEvent &e);
	void on_mouse_motion(wxMouseEvent &e);
	void on_mouse_leave(wxMouseEvent &e);
	void on_left_up(wxMouseEvent &e);
	void add_line(const Line &l);
};

class MainWindow : public wxFrame {
public:
	MainWindow(const wxString &title);
	ControlPanel *control_panel;
	PicturePanel *picture_panel;
	wxPanel *parent_panel;
};
