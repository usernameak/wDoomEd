#pragma once

#include <wx/wx.h>

#include <wx/glcanvas.h>

class WDEdMainCanvas : public wxGLCanvas {
    wxGLContext *ctx;
public:
    WDEdMainCanvas(wxWindow* parent);
    void Render(wxPaintEvent&);
    void StartDragging(wxMouseEvent&);
    void Drag(wxMouseEvent&);
    void EndDragging(wxMouseEvent&);
    void MouseLeftDown(wxMouseEvent&);
    void MouseMove(wxMouseEvent&);
    void Scale(float);
    void KeyDown(wxKeyEvent &event);
protected:
    DECLARE_EVENT_TABLE()
private:
    wxPoint convertCoordsScreenToWorld(wxPoint&);
};