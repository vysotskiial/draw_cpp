#include "control_panel.h"
#include <wx/defs.h>

using namespace std;

constexpr int ID_SAVE = 101;
constexpr int ID_ZOOM = 102;

ControlPanel::ControlPanel(wxPanel *p)
  : wxPanel(p, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_SUNKEN), parent(p)
{
	save_button = new wxButton(this, ID_SAVE, wxT("Save"), wxPoint(10, 10));
	text_button = new wxButton(this, ID_ZOOM, wxT("Text"), wxPoint(110, 10));
	coords_text = new wxStaticText(this, -1, "", wxPoint{0, 0} + GetSize(),
	                               {100, 30}, wxALIGN_RIGHT);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags flags;
	flags.Expand().Left().Shaped();
	hbox->Add(save_button, flags);
	hbox->Add(text_button, flags);
	hbox->AddStretchSpacer();
	flags.Right();
	hbox->Add(coords_text, flags);

	SetSizer(hbox);

	save_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ControlPanel::on_save, this);
	text_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ControlPanel::on_text, this);
}

void ControlPanel::on_text(wxCommandEvent &e)
{
	TextDialog d(texts);
	if (d.ShowModal() != wxID_OK)
		return;

	texts.clear();
	auto lst = d.GetList();
	for (auto i = 0; i < lst->GetItemCount(); i++)
		texts.push_back({lst->GetTextValue(i, 0).ToStdString(),
		                 stod(lst->GetTextValue(i, 1).ToStdString()),
		                 stod(lst->GetTextValue(i, 2).ToStdString())});

	generate_text_bitmaps();
}

TextDialog::TextDialog(const vector<TextInst> &texts)
  : wxDialog(NULL, -1, "Text", wxDefaultPosition, wxSize(600, 600))
{
	panel = new wxPanel(this, -1);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

	list_ctrl = new wxDataViewListCtrl(panel, wxID_ANY, wxDefaultPosition,
	                                   wxSize(600, 350));
	list_ctrl->AppendTextColumn("Text", wxDATAVIEW_CELL_EDITABLE, 400);
	list_ctrl->AppendTextColumn("Coord X", wxDATAVIEW_CELL_EDITABLE, 100);
	list_ctrl->AppendTextColumn("Coord Y", wxDATAVIEW_CELL_EDITABLE, 100);

	for (auto &inst : texts) {
		wxVector<wxVariant> data;
		data.push_back(wxVariant(inst.text));
		data.push_back(wxVariant(to_string(inst.x)));
		data.push_back(wxVariant(to_string(inst.y)));
		list_ctrl->AppendItem(data);
	}

	add_button = new wxButton(panel, wxID_ANY, "ADD");
	delete_button = new wxButton(panel, wxID_ANY, "DELETE");
	ok_button = new wxButton(panel, wxID_OK, "OK");
	cancel_button = new wxButton(panel, wxID_CANCEL, "CANCEL");

	add_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &e) {
		wxVector<wxVariant> data;
		data.push_back(wxVariant("Text"));
		data.push_back(wxVariant("0.0"));
		data.push_back(wxVariant("0.0"));
		list_ctrl->AppendItem(data);
		list_ctrl->SelectRow(list_ctrl->GetItemCount() - 1);
	});

	delete_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &e) {
		if (list_ctrl->HasSelection())
			list_ctrl->DeleteItem(list_ctrl->GetSelectedRow());
	});

	sizer->Add(list_ctrl, 0, wxEXPAND | wxALL, 10);
	sizer->Add(add_button, 0, wxEXPAND | wxALL, 10);
	sizer->Add(delete_button, 0, wxEXPAND | wxALL, 10);
	sizer->Add(ok_button, 0, wxEXPAND | wxALL, 10);
	sizer->Add(cancel_button, 0, wxEXPAND | wxALL, 10);
	panel->SetSizer(sizer);
}
