#pragma once

#include <wx/wx.h>
#include <wx/app.h> 

#include "main_frame.h"

class WDEdApp : public wxApp {
public:
    WDEdMainFrame *frame;
    virtual bool OnInit();
};

wxDECLARE_APP(WDEdApp);