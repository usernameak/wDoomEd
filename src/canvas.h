#pragma once

#include <GL/glew.h>

#include <wx/wx.h>

#include <wx/glcanvas.h>

class WDEdMainCanvas : public wxGLCanvas {
    wxGLContext *ctx;
    GLuint vboSectors = 0xFFFFFFFF;
    bool glewInited = false;
public:
    WDEdMainCanvas(wxWindow* parent, const int * attribs);
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
    void OnMouseWheel(wxMouseEvent &event);
protected:
    DECLARE_EVENT_TABLE()
private:
    wxPoint convertCoordsScreenToWorld(wxPoint&);
};
