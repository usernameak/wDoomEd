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


BEGIN_EVENT_TABLE(WDEdMainFrame, wxFrame)
    EVT_MENU    (ID_FILE_OPEN, WDEdMainFrame::OpenFile)
    EVT_MENU    (wxID_EXIT, WDEdMainFrame::Exit)
END_EVENT_TABLE()