#pragma once

#include <wx/wx.h>
#include "canvas.h"

enum WDEdMainFrameIDs {
    ID_FILE_OPEN = 1
};

enum WDEdStatusBarElement {
    WDED_SB_EL_ZERO,
    WDED_SB_EL_GRID,
    WDED_SB_EL_COORDS,
    WDED_SB_EL_MAX
};

class WDEdMainFrame : public wxFrame {
public:
    WDEdMainFrame();
    void OpenFile(wxCommandEvent&);
    void Exit(wxCommandEvent&);
    WDEdMainCanvas *canvas;
    void Resize(wxSizeEvent& evt);
private:
    DECLARE_EVENT_TABLE()
};