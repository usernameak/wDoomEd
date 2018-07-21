#include "properties_dialog.h"

using namespace WDEdMapEditor;

WDEdPropertiesDialog::WDEdPropertiesDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, "Properties") { // @suppress("Class members should be properly initialized")
    SetExtraStyle(GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
    rootSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(rootSizer);
}

WDEdPropertiesDialog *WDEdPropertiesDialog::Finish() {
    AddNoBorderGroup();
    sizer->Add(new wxButton(panel, wxID_OK, "OK"), wxGBPosition(sizer->GetRows(), 0));
    sizer->Add(new wxButton(panel, wxID_CANCEL, "Cancel"), wxGBPosition(sizer->GetRows() - 1, 1));
    Fit();
    return this;
}

void WDEdPropertiesDialog::ProcessButton(wxCommandEvent &event) {
    if(event.GetId() == wxID_OK) {
        if(!Validate()) {
            wxMessageBox("Please input valid values.", "Error", wxOK|wxCENTER, this);
            return;
        }
        TransferDataFromWindow();
        EndModal(wxID_OK);
    } else if(event.GetId() == wxID_CANCEL) {
        EndModal(wxID_CANCEL);
    }
}

BEGIN_EVENT_TABLE(WDEdPropertiesDialog, wxDialog)
    EVT_BUTTON(wxID_OK, WDEdPropertiesDialog::ProcessButton)
    EVT_BUTTON(wxID_CANCEL, WDEdPropertiesDialog::ProcessButton)
END_EVENT_TABLE()
