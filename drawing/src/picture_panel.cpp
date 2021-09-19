#include <cmath>
#include <wx/graphics.h>
#include <wx/dcgraph.h>
#include "picture_panel.h"

using namespace std;
PicturePanel::PicturePanel(wxPanel *p)
  : wxPanel(p, wxID_ANY, wxDefaultPosition, wxSize(picture_size, picture_size),
            wxBORDER_SUNKEN)
{
	SetBackgroundColour("White");
	Bind(wxEVT_PAINT, &PicturePanel::on_paint, this);
	Bind(wxEVT_MOTION, &PicturePanel::on_mouse_motion, this);
	Bind(wxEVT_LEAVE_WINDOW, &PicturePanel::on_mouse_leave, this);
	Bind(wxEVT_LEFT_DOWN,
	     [this](wxMouseEvent &e) { zoom_start = e.GetPosition(); });
	Bind(wxEVT_LEFT_UP, &PicturePanel::on_left_up, this);
}

void PicturePanel::on_paint(wxPaintEvent &e)
{
	wxGCDC dc(this);
	dc.SetBackground(*wxWHITE_BRUSH);
	if (!grid.finalized_border)
		finalize_border();
	draw_grid(dc);
	update_lines();
	draw_graphs(dc);
	draw_zoom_rect(dc);
	draw_text(dc);
}

void PicturePanel::draw_zoom_rect(wxGCDC &dc)
{
	if (zoom_size.IsFullySpecified()) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(*wxBLACK_DASHED_PEN);
		dc.DrawRectangle(zoom_start, zoom_size);
	}
}

void PicturePanel::draw_grid(wxGCDC &dc)
{
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawLine(to_pixel({grid.max_grid.x, 0}), to_pixel({grid.min_grid.x, 0}));
	dc.DrawLine(to_pixel({0, grid.max_grid.y}), to_pixel({0, grid.min_grid.y}));
}

void PicturePanel::draw_text(wxGCDC &dc)
{
	for (auto &[p, bmap] : text_map)
		dc.DrawBitmap(bmap, to_pixel(p), true);
}

wxPoint PicturePanel::to_pixel(Point p)
{
	auto max_x = GetSize().x;
	auto max_y = GetSize().y;
	double new_x =
	  (p.x - grid.min_grid.x) * max_x / (grid.max_grid.x - grid.min_grid.x);
	double new_y =
	  (grid.max_grid.y - p.y) * max_y / (grid.max_grid.y - grid.min_grid.y);
	return wxPoint(new_x, new_y);
}

Point PicturePanel::to_point(wxPoint p)
{
	double max_x = GetSize().x;
	double max_y = GetSize().y;
	auto new_x =
	  p.x * (grid.max_grid.x - grid.min_grid.x) / max_x + grid.min_grid.x;
	auto new_y =
	  p.y * (grid.min_grid.y - grid.max_grid.y) / max_y + grid.max_grid.y;
	return Point{new_x, new_y};
}

void PicturePanel::draw_graphs(wxGCDC &dc)
{
	auto i = 0;
	for (auto &pixel_line : pixel_lines) {
		dc.SetPen(lines[i++].pen);
		dc.DrawLines(pixel_line.size(), pixel_line.data());
	}
}

void PicturePanel::on_left_up(wxMouseEvent &e)
{
	Point rect_start = to_point(zoom_start);
	Point rect_end = to_point(zoom_start + zoom_size);

	grid.max_grid.x = max(rect_start.x, rect_end.x);
	grid.min_grid.x = min(rect_start.x, rect_end.x);
	grid.max_grid.y = max(rect_start.y, rect_end.y);
	grid.min_grid.y = min(rect_start.y, rect_end.y);
	if (!grid.manual_ticks)
		grid.set_ticks();

	zoom_size = {-1, -1};
	update_lines();
	Refresh();
}

void PicturePanel::add_line(const Line &l)
{
	lines.push_back(l);
	pixel_lines.push_back({});
	for (auto &p : l.points) {
		if (!grid.manual_border) {
			if (p.x > grid.max_lines.x)
				grid.max_lines.x = p.x;
			if (p.x < grid.min_lines.x)
				grid.min_lines.x = p.x;
			if (p.y > grid.max_lines.y)
				grid.max_lines.y = p.y;
			if (p.y < grid.min_lines.y)
				grid.min_lines.y = p.y;
		}
		pixel_lines.back().push_back(to_pixel(p));
	}
}

void PicturePanel::update_lines()
{
	pixel_lines.clear();
	for (auto &line : lines) {
		pixel_lines.push_back({});
		for (auto &p : line.points)
			pixel_lines.back().push_back(to_pixel(p));
	}
}

void PicturePanel::finalize_border()
{
	grid.finalized_border = true;
	if (grid.manual_border)
		return;
	grid.max_grid = grid.max_lines;
	grid.min_grid = grid.min_lines;

	auto max_y_shift = max(abs(grid.max_lines.y), abs(grid.min_lines.y)) * 0.05;
	grid.max_grid.y += max_y_shift;
	grid.min_grid.y -= max_y_shift;
}

