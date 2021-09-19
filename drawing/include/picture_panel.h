#pragma once

#include <wx/wx.h>
#include <wx/dcgraph.h>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <sstream>

constexpr int picture_size = 500;
constexpr int toolbar_size = 100;

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

inline bool operator<(const Point &lhs, const Point &rhs)
{
	if (lhs.x < rhs.x)
		return true;
	if (lhs.x > rhs.x)
		return false;
	return lhs.y < rhs.y;
}

struct Grid {
	Point max_grid;
	Point min_grid;

	Point max_lines;
	Point min_lines;

	bool finalized_border{false};
	bool manual_border{false};
	bool manual_ticks{false};
	std::vector<double> x_ticks;
	std::vector<double> y_ticks;
	void set_ticks() {}
};

struct Line {
	wxPen pen;
	std::vector<Point> points;
};

class PicturePanel : public wxPanel {
private:
	wxPoint zoom_start;
	wxSize zoom_size;
	Grid grid;
	std::vector<Line> lines;                       // always the same
	std::vector<std::vector<wxPoint>> pixel_lines; // changes on resize
	                                               // and zoom

	void draw_grid(wxGCDC &);
	void draw_graphs(wxGCDC &);
	void draw_zoom_rect(wxGCDC &);
	void draw_text(wxGCDC &);
	void update_lines();
	void finalize_border();
	void finalize_ticks();

	wxPoint to_pixel(Point p);
	Point to_point(wxPoint p);

public:
	std::map<Point, wxBitmap> text_map;

	PicturePanel(wxPanel *parent);
	void on_paint(wxPaintEvent &e);
	void on_mouse_motion(wxMouseEvent &e);
	void on_mouse_leave(wxMouseEvent &e);
	void on_left_up(wxMouseEvent &e);
	void add_line(const Line &l);
};

