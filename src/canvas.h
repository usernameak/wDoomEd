#pragma once

#include <GL/glew.h>

#include <wx/wx.h>

#include <wx/glcanvas.h>

class WDEdMainCanvas : public wxGLCanvas {
    wxGLContext *ctx;
public:
    WDEdMainCanvas(wxWindow* parent);
    void Render(wxPaintEvent&);
    void RenderSectors();
    void RenderGrid();
    void RenderLines();
    void RenderVertices();
    void StartDragging(wxMouseEvent&);
    void Drag(wxMouseEvent&);
    void EndDragging(wxMouseEvent&);
    void MouseLeftDown(wxMouseEvent&);
    void MouseMove(wxMouseEvent&);
    void Scale(float);
    void KeyDown(wxKeyEvent &event);
    void OpenPropertiesMenu(wxMouseEvent &event);
protected:
    DECLARE_EVENT_TABLE()
private:
    wxPoint convertCoordsScreenToWorld(wxPoint&);
};