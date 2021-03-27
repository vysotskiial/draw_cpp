#include <exception>
#include <string>
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
	coords_text = new wxStaticText(this, -1, "", wxPoint(400, 10));
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
	draw_grid(dc);
	draw_graphs(dc);
	draw_zoom_rect(dc);
}

void PicturePanel::draw_zoom_rect(wxPaintDC &dc)
{
	if (zoom_size.IsFullySpecified()) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		auto pen = dc.GetPen();
		pen.SetStyle(wxPENSTYLE_SHORT_DASH);
		pen.SetColour("black");
		dc.SetPen(pen);
		dc.DrawRectangle(zoom_start, zoom_size);
	}
}

void PicturePanel::draw_grid(wxPaintDC &dc)
{
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawLine(convert_coords(grid.x_max, 0, dc),
	            convert_coords(grid.x_min, 0, dc));
	dc.DrawLine(convert_coords(0, grid.y_max, dc),
	            convert_coords(0, grid.y_min, dc));
}

wxPoint PicturePanel::convert_coords(double x, double y, const wxDC &dc)
{
	auto max_x = dc.GetSize().x;
	auto max_y = dc.GetSize().y;
	double new_x = (x - grid.x_min) * max_x / (grid.x_max - grid.x_min);
	double new_y = (y - grid.y_min) * max_y / (grid.y_max - grid.y_min);
	return wxPoint(new_x, new_y);
}

void PicturePanel::draw_graphs(wxPaintDC &dc)
{
	dc.DrawLine(50, 50, 100, 100);
}

void PicturePanel::on_left_up(wxMouseEvent &e)
{
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
		mw->control_panel->coords_text->SetLabel(to_string(e.GetX()) + "; " +
		                                         to_string(e.GetY()));
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

	hbox->Add(picture_panel, 1, wxStretch::wxEXPAND | wxALL, 5);
	hbox->Add(control_panel, 1, wxStretch::wxEXPAND | wxALL, 5);

	parent_panel->SetSizer(hbox);
}
