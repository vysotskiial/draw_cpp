#include <exception>
#include <string>
#include <cmath>
#include <wx/defs.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/pen.h>
#include "main_window.h"

using namespace std;

constexpr int ID_SAVE = 101;
constexpr int ID_ZOOM = 102;

constexpr int picture_size = 500;
constexpr int toolbar_size = 100;

ControlPanel::ControlPanel(wxPanel *p)
  : wxPanel(p, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_SUNKEN), parent(p)
{
	save_button = new wxButton(this, ID_SAVE, wxT("Save"), wxPoint(10, 10));
	zoom_button = new wxButton(this, ID_ZOOM, wxT("Zoom"), wxPoint(110, 10));
	coords_text = new wxStaticText(this, -1, "", wxPoint{0, 0} + GetSize(),
	                               {100, 30}, wxALIGN_RIGHT);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags flags;
	flags.Expand().Left().Shaped();
	hbox->Add(save_button, flags);
	hbox->Add(zoom_button, flags);
	hbox->AddStretchSpacer();
	flags.Right();
	hbox->Add(coords_text, flags);

	SetSizer(hbox);

	save_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ControlPanel::on_save, this);
	zoom_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ControlPanel::on_zoom, this);
}

void ControlPanel::on_save(wxCommandEvent &e)
{
	MainWindow *mw = dynamic_cast<MainWindow *>(parent->GetParent());
	if (!mw) {
		throw runtime_error("Something is wrong with inheritage");
	}
	wxWindowDC cdc(mw->picture_panel);

	int w = cdc.GetSize().x;
	int h = cdc.GetSize().y;

	wxBitmap bitmap(w, h, 24);
	wxMemoryDC mdc(bitmap);
	mdc.Blit(0, 0, w, h, &cdc, 0, 0);
	wxFileDialog file_location(this, ("File location"), "", "",
	                           "PNG files (*.png)|*.png",
	                           wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (file_location.ShowModal() == wxID_CANCEL)
		return;

	bitmap.SaveFile(file_location.GetPath(), wxBITMAP_TYPE_PNG);
}

void ControlPanel::on_zoom(wxCommandEvent &e)
{
}

PicturePanel::PicturePanel(wxPanel *p)
  : wxPanel(p, wxID_ANY, wxDefaultPosition, wxSize(picture_size, picture_size),
            wxBORDER_SUNKEN)
{
	Bind(wxEVT_PAINT, &PicturePanel::on_paint, this);
	Bind(wxEVT_MOTION, &PicturePanel::on_mouse_motion, this);
	Bind(wxEVT_LEAVE_WINDOW, &PicturePanel::on_mouse_leave, this);
	Bind(wxEVT_LEFT_DOWN,
	     [this](wxMouseEvent &e) { zoom_start = e.GetPosition(); });
	Bind(wxEVT_LEFT_UP, &PicturePanel::on_left_up, this);
}

void PicturePanel::on_paint(wxPaintEvent &e)
{
	wxPaintDC dc(this);
	finalize_grid();
	draw_grid(dc);
	update_lines();
	draw_graphs(dc);
	draw_zoom_rect(dc);
}

void PicturePanel::draw_zoom_rect(wxPaintDC &dc)
{
	if (zoom_size.IsFullySpecified()) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(*wxBLACK_DASHED_PEN);
		dc.DrawRectangle(zoom_start, zoom_size);
	}
}

void PicturePanel::draw_grid(wxPaintDC &dc)
{
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawLine(to_pixel({grid.x_max, 0}), to_pixel({grid.x_min, 0}));
	dc.DrawLine(to_pixel({0, grid.y_max}), to_pixel({0, grid.y_min}));
}

wxPoint PicturePanel::to_pixel(Point p)
{
	auto max_x = GetSize().x;
	auto max_y = GetSize().y;
	double new_x = (p.x - grid.x_min) * max_x / (grid.x_max - grid.x_min);
	double new_y = (grid.y_max - p.y) * max_y / (grid.y_max - grid.y_min);
	return wxPoint(new_x, new_y);
}

Point PicturePanel::to_point(wxPoint p)
{
	double max_x = GetSize().x;
	double max_y = GetSize().y;
	auto new_x = p.x * (grid.x_max - grid.x_min) / max_x + grid.x_min;
	auto new_y = p.y * (grid.y_min - grid.y_max) / max_y + grid.y_max;
	return Point{new_x, new_y};
}

void PicturePanel::draw_graphs(wxPaintDC &dc)
{
	for (auto &pixel_line : pixel_lines)
		dc.DrawLines(pixel_line.size(), pixel_line.data());
}

void PicturePanel::on_left_up(wxMouseEvent &e)
{
	Point rect_start = to_point(zoom_start);
	Point rect_end = to_point(zoom_start + zoom_size);

	grid.x_max = max(rect_start.x, rect_end.x);
	grid.x_min = min(rect_start.x, rect_end.x);
	grid.y_max = max(rect_start.y, rect_end.y);
	grid.y_min = min(rect_start.y, rect_end.y);
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
			if (p.x > grid.x_max)
				grid.x_max = p.x;
			if (p.x < grid.x_min)
				grid.x_min = p.x;
			if (p.y > grid.y_max)
				grid.y_max = p.y;
			if (p.y < grid.y_min)
				grid.y_min = p.y;
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

void PicturePanel::finalize_grid()
{
	if (!grid.manual_border) {
		auto max_y_shift = max(abs(grid.y_max), abs(grid.y_min)) * 0.05;
		grid.y_max += max_y_shift;
		grid.y_min -= max_y_shift;
	}

	if (grid.manual_ticks)
		return;

	grid.x_ticks.clear();
	grid.y_ticks.clear();

	// TODO some smart ticks algorithm
}

void PicturePanel::on_mouse_leave(wxMouseEvent &e)
{
	MainWindow *mw = dynamic_cast<MainWindow *>(GetParent()->GetParent());
	if (!mw) {
		throw runtime_error("Something is wrong with inheritage");
	}
	mw->control_panel->coords_text->SetLabel("");
}

void PicturePanel::on_mouse_motion(wxMouseEvent &e)
{
	if (!e.Dragging()) {
		MainWindow *mw = dynamic_cast<MainWindow *>(GetParent()->GetParent());
		if (!mw) {
			throw runtime_error("Something is wrong with inheritage");
		}
		mw->control_panel->coords_text->SetLabel(string(to_point(e.GetPosition())));
		return;
	}

	zoom_size = {e.GetX() - zoom_start.x, e.GetY() - zoom_start.y};
	Refresh();
}

MainWindow::MainWindow(const wxString &title)
  : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition,
            wxSize(picture_size, picture_size + toolbar_size))
{
	parent_panel = new wxPanel(this, wxID_ANY);
	wxBoxSizer *hbox = new wxBoxSizer(wxVERTICAL);

	control_panel = new ControlPanel(parent_panel);
	picture_panel = new PicturePanel(parent_panel);

	hbox->Add(picture_panel, 1, wxStretch::wxEXPAND | wxDirection::wxALL, 5);
	hbox->Add(control_panel, 0, wxDirection::wxALL | wxStretch::wxEXPAND, 5);

	parent_panel->SetSizer(hbox);
}
