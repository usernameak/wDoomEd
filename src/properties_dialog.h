#pragma once

#include <wx/wx.h>
#include "map_editor.h"

class WDEdPropertiesDialog : public wxDialog {
private:
    wxTextCtrl *tcVertexX, *tcVertexY;
    WDEdMapEditor::WDEdMapEditorTool mode;
    WDEdMapEditor::WDEdAnyElement element;
    DECLARE_EVENT_TABLE()
public:
    WDEdPropertiesDialog(wxWindow *parent, WDEdMapEditor::WDEdMapEditorTool mode, WDEdMapEditor::WDEdAnyElement element);
    void ProcessButton(wxCommandEvent &event);
};