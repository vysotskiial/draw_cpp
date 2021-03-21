#include <exception>
#include "main_window.h"

using namespace std;

constexpr int ID_SAVE = 101;
constexpr int ID_ZOOM = 102;

ControlPanel::ControlPanel(wxPanel *p)
  : wxPanel(p, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_SUNKEN), parent(p)
{
	save_button = new wxButton(this, ID_SAVE, wxT("Save"), wxPoint(10, 10));
	zoom_button = new wxButton(this, ID_ZOOM, wxT("Zoom"), wxPoint(10, 50));
	Connect(ID_SAVE, wxEVT_COMMAND_BUTTON_CLICKED,
	        wxCommandEventHandler(ControlPanel::on_save));
	Connect(ID_ZOOM, wxEVT_COMMAND_BUTTON_CLICKED,
	        wxCommandEventHandler(ControlPanel::on_zoom));
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
	mdc.SelectObject(wxNullBitmap);
	bitmap.SaveFile("/home/vysotskiial/pictures/test.png", wxBITMAP_TYPE_PNG);
}

PicturePanel::PicturePanel(wxPanel *p)
  : wxPanel(p, wxID_ANY, wxDefaultPosition, wxSize(270, 150), wxBORDER_SUNKEN)
{
	Connect(wxEVT_PAINT, wxPaintEventHandler(PicturePanel::on_paint));
}

void PicturePanel::on_paint(wxPaintEvent &e)
{
	wxPaintDC dc(this);
	wxCoord x1 = 50;
	wxCoord y1 = 50;
	wxCoord x2 = 100;
	wxCoord y2 = 100;
	auto pen = dc.GetPen();
	pen.SetColour(255, 0, 0);
	dc.SetPen(pen);
	dc.DrawLine(x1, y1, x2, y2);
}

MainWindow::MainWindow(const wxString &title)
  : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(290, 150))
{
	parent_panel = new wxPanel(this, wxID_ANY);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	control_panel = new ControlPanel(parent_panel);
	picture_panel = new PicturePanel(parent_panel);

	hbox->Add(control_panel, 1, wxEXPAND | wxALL, 5);
	hbox->Add(picture_panel, 1, wxEXPAND | wxALL, 5);

	parent_panel->SetSizer(hbox);
}
