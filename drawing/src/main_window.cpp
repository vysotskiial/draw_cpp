#include <exception>
#include <string>
#include <cmath>
#include <klfbackend/klfbackend.h>
#include <QFile>
#include <QApplication>
#include "main_window.h"

using namespace std;

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

void ControlPanel::generate_text_bitmaps()
{
	int argc = 0;
	QCoreApplication app(argc, nullptr);

	MainWindow *mw = dynamic_cast<MainWindow *>(GetParent()->GetParent());
	if (!mw) {
		throw runtime_error("Something is wrong with inheritage");
	}
	mw->picture_panel->text_map.clear();

	for (auto &text : texts) {
		KLFBackend::klfSettings settings;
		bool ok = KLFBackend::detectSettings(&settings);
		if (!ok) {
			// vital program not found
			throw("error in your system: are latex,dvips and gs installed?");
			return;
		}

		KLFBackend::klfInput input;
		input.latex = text.text.c_str();
		input.fontsize = 7;
		input.mathmode = "\\begin{equation*} ... \\end{equation*}";
		input.preamble = "\\usepackage{amsmath}\n";
		input.dpi = 300;

		KLFBackend::klfOutput out = KLFBackend::getLatexFormula(input, settings);

		if (out.status != 0) {
			// an error occurred. an appropriate error string is in out.errorstr
			cerr << out.errorstr.toStdString();
			return;
		}

		QFile fpng(".temp.png");
		fpng.open(QIODevice::WriteOnly);
		fpng.write(out.pngdata);
		fpng.close();

		wxBitmap bmap;
		bmap.LoadFile(".temp.png", wxBITMAP_TYPE_PNG);
		bmap.SetMask(new wxMask(bmap, "White"));

		mw->picture_panel->text_map[{text.x, text.y}] = bmap;
		mw->picture_panel->Refresh();
	}
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
