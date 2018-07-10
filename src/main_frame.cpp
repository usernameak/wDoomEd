#include "main_frame.h"

#include "canvas.h"
#include "map_editor.h"
#include "app.h"

WDEdMainFrame::WDEdMainFrame() : wxFrame::wxFrame(nullptr, wxID_ANY, "wDoomEd") {
    wxGetApp().frame = this;

    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_FILE_OPEN, "&Open...\tCtrl-O");
    menuFile->Append(wxID_EXIT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");

    SetMenuBar(menuBar);

    wxStatusBar *statusBar = new wxStatusBar(this, wxID_ANY, wxSTB_DEFAULT_STYLE);
    statusBar->SetFieldsCount(WDED_SB_EL_MAX, new int[WDED_SB_EL_MAX]{-1, 100, 100});
    statusBar->SetStatusText("(0, 0)", WDED_SB_EL_COORDS);
    SetStatusBar(statusBar);

    wxToolBar *toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_DEFAULT_STYLE);
    {
        wxImage img("res/icon/verts.png");
        toolBar->AddRadioTool(ID_TOOL_VERTS, "Vertexes", img);
    }
    {
        wxImage img("res/icon/lines.png");
        toolBar->AddRadioTool(ID_TOOL_LINEDEFS, "Linedefs", img);
    }
    SetToolBar(toolBar);

    canvas = new WDEdMainCanvas(this);
}

void WDEdMainFrame::Exit(wxCommandEvent & WXUNUSED(event)) {
    Close(true);
}

void WDEdMainFrame::OpenFile(wxCommandEvent & WXUNUSED(event)) {
    wxFileDialog openFileDialog(this, _("Open file"), "", "",
                       "WAD files (*.wad)|*.wad", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if(openFileDialog.ShowModal() != wxID_CANCEL) {
        WDEdMapEditor::OpenArchive(openFileDialog.GetPath());
    }
}

void WDEdMainFrame::ChangeTool(wxCommandEvent &event) {
    int id = event.GetId();
    switch(id) {
        case ID_TOOL_VERTS:
            WDEdMapEditor::SetTool(WDEdMapEditor::WDED_ME_TOOL_VERTS);
        break;
        case ID_TOOL_LINEDEFS:
            WDEdMapEditor::SetTool(WDEdMapEditor::WDED_ME_TOOL_LINES);
        break;
    }
    canvas->Refresh();
    canvas->Update();
}


BEGIN_EVENT_TABLE(WDEdMainFrame, wxFrame)
    EVT_MENU      (ID_FILE_OPEN, WDEdMainFrame::OpenFile)
    EVT_MENU      (wxID_EXIT, WDEdMainFrame::Exit)
    EVT_MENU_RANGE(ID_TOOL_VERTS, ID_TOOL_MAX, WDEdMainFrame::ChangeTool)
END_EVENT_TABLE()