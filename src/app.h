#pragma once

#include <GL/glew.h>
#include <wx/wx.h>
#include <wx/app.h>

#include "texture.h"
#include "main_frame.h"

class WDEdApp : public wxApp {
public:
    WDEdMainFrame *frame;
    virtual bool OnInit();
    WDEdDoomFlatHandler *flatHandler;
    WDEdDoomPatchHandler *patchHandler;
};

wxDECLARE_APP(WDEdApp);