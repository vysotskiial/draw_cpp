#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>
#include <string>
#include <vector>

struct TextInst {
	std::string text;
	double x;
	double y;
};

class TextDialog : public wxDialog {
private:
	wxPanel *panel;
	wxDataViewListCtrl *list_ctrl;
	wxButton *cancel_button;
	wxButton *ok_button;
	wxButton *delete_button;
	wxButton *add_button;

public:
	TextDialog(const std::vector<TextInst> &texts);
	wxDataViewListCtrl *GetList() { return list_ctrl; }
};

class ControlPanel : public wxPanel {
private:
	std::vector<TextInst> texts;

public:
	ControlPanel(wxPanel *p);
	void on_save(wxCommandEvent &e);
	void on_text(wxCommandEvent &e);
	void on_set_coords(wxCommandEvent &event);
	void generate_text_bitmaps();

	wxButton *save_button;
	wxButton *text_button;
	wxStaticText *coords_text;
	wxPanel *parent;
};
