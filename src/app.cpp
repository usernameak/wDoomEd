#include "app.h"

#include <wx/wx.h>

#include "main_frame.h"

bool WDEdApp::OnInit() {
    wxInitAllImageHandlers();
    new WDEdMainFrame();
    frame->Maximize();
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(WDEdApp);