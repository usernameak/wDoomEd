#include "properties_dialog.h"

using namespace WDEdMapEditor;

WDEdPropertiesDialog::WDEdPropertiesDialog(wxWindow *parent, WDEdMapEditorTool mode, WDEdAnyElement element)
    : wxDialog(parent, wxID_ANY, "Properties"), mode(mode), element(element) {
        switch(mode) {
            case WDED_ME_TOOL_LINES:
                SetTitle(wxString::Format("Properties for linedef"));
            break;
            case WDED_ME_TOOL_VERTS:
                SetTitle(wxString::Format("Properties for vertex"));
            break;
        }
        SetSizer(CreateButtonSizer(wxOK | wxCANCEL));
        wxPanel *panel = new wxPanel(this);
        wxGridSizer *sizer = new wxGridSizer(2, 5, 5);
        panel->SetSizer(sizer);
        GetSizer()->Add(panel);
        switch(mode) {
            case WDED_ME_TOOL_VERTS:
                sizer->Add(new wxStaticText(this, wxID_ANY, "X coordinate"));
                sizer->Add(tcVertexX = new wxTextCtrl(this, wxID_ANY, wxString::Format("%d", element.vertex->x)));
                sizer->Add(new wxStaticText(this, wxID_ANY, "Y coordinate"));
                sizer->Add(tcVertexY = new wxTextCtrl(this, wxID_ANY, wxString::Format("%d", element.vertex->y)));
            break;
        }
    }

void WDEdPropertiesDialog::ProcessButton(wxCommandEvent &event) {
    if(event.GetId() == wxID_OK) {
        switch(mode) {
            case WDED_ME_TOOL_VERTS:
                long newx, newy;
                if(!tcVertexX->GetValue().ToLong(&newx) || !tcVertexY->GetValue().ToLong(&newy)) {
                    wxMessageDialog(this, "Please enter valid values or press Cancel", "Error", wxOK | wxCENTER | wxICON_ERROR).ShowModal();
                    return;
                }
                element.vertex->x = newx;
                element.vertex->y = newy;
            break;
        }
        EndModal(wxID_OK);
    } else if(event.GetId() == wxID_CANCEL) {
        EndModal(wxID_CANCEL);
    }
}

BEGIN_EVENT_TABLE(WDEdPropertiesDialog, wxDialog)
    EVT_BUTTON(wxID_OK, WDEdPropertiesDialog::ProcessButton)
    EVT_BUTTON(wxID_CANCEL, WDEdPropertiesDialog::ProcessButton)
END_EVENT_TABLE()