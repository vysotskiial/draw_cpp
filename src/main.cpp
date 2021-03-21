#include <wx/wx.h>
#include <wx/panel.h>

class Line : public wxFrame {
public:
	Line(const wxString &title)
	  : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(250, 150))
	{
		Connect(wxEVT_PAINT, wxPaintEventHandler(Line::OnPaint));
		wxPanel *panel = new wxPanel(this, wxID_ANY);
		wxButton *button =
		  new wxButton(panel, wxID_EXIT, wxT("Quit"), wxPoint(20, 20));
		Connect(wxID_EXIT, wxEVT_COMMAND_BUTTON_CLICKED,
		        wxCommandEventHandler(Line::OnButton));
		button->SetFocus();
	}
	void OnPaint(wxPaintEvent &event)
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
	void OnButton(wxCommandEvent &event)
	{
		wxWindowDC cdc(this);

		int w = cdc.GetSize().x;
		int h = cdc.GetSize().y;

		wxBitmap bitmap(w, h, 24);
		wxMemoryDC mdc(bitmap);
		mdc.Blit(0, 0, w, h, &cdc, 0, 0);
		mdc.SelectObject(wxNullBitmap);
		bitmap.SaveFile("/home/vysotskiial/pictures/test.png", wxBITMAP_TYPE_PNG);
		Close(true);
	}
};

class MyApp : public wxApp {
public:
	bool OnInit() override
	{
		Line *simple = new Line(wxT("Simple"));
		simple->Show();
		return true;
	}
};
IMPLEMENT_APP(MyApp)
