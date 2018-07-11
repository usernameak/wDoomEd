#pragma once

#include <GL/glew.h>

#include <wx/wx.h>
#include "canvas.h"

enum WDEdMainFrameIDs {
    ID_FILE_OPEN = 1,
    ID_TOOL_VERTS,
    ID_TOOL_LINEDEFS,
    ID_TOOL_MAX
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
    void ChangeTool(wxCommandEvent& evt);
private:
    DECLARE_EVENT_TABLE()
};