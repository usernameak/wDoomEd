#include "canvas.h"

#include "map_editor.h"
#include "app.h"

#include <cmath>
#include <climits>

#include <GL/glu.h>

using namespace WDEdMapEditor;

WDEdMainCanvas::WDEdMainCanvas(wxWindow *parent) : wxGLCanvas::wxGLCanvas(parent, wxID_ANY, nullptr) {
    ctx = new wxGLContext(this);
    wxGetApp().frame->GetStatusBar()->SetStatusText(wxString::Format("Grid: %dx%d", gridSize, gridSize), WDED_SB_EL_GRID);
}

void WDEdMainCanvas::Render(wxPaintEvent& WXUNUSED(event)) {
    if(scale == 0.0f) scale = 1.0f;
    SetCurrent(*ctx);
    wxPaintDC(this);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, (GLint)GetSize().x, (GLint)GetSize().y);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, (GLint)GetSize().x, 0, (GLint)GetSize().y);
    glTranslatef((GLint)GetSize().x / 2, (GLint)GetSize().y / 2, 0.0f);

    glMatrixMode(GL_MODELVIEW);

    if(WDEdMapEditor::mapIsCurrentlyLoaded) {
        glLoadIdentity();
        glTranslatef(offsetX, -offsetY, 0.0f);
        glScalef(scale, scale, scale);

        glColor4f(0.2, 0.2, 0.2, 1.0);

        glBegin(GL_LINES);

            int step = gridSize;
            float initialX = -GetSize().x * 0.5 / scale - offsetX / scale;
            initialX = initialX - fmod(initialX, step) - step;
            float finalX = GetSize().x * 0.5 / scale - offsetX / scale;
            float initialY = -GetSize().y * 0.5 / scale + offsetY / scale;
            initialY = initialY - fmod(initialY, step) - step;
            float finalY = GetSize().y * 0.5 / scale + offsetY / scale;
            for(float x = initialX; x < finalX; x += step) {
                glVertex2f(x, initialY);
                glVertex2f(x, finalY);
            }
            for(float y = initialY; y < finalY; y += step) {
                glVertex2f(initialX, y);
                glVertex2f(finalX, y);
            }
        glEnd();

        glLoadIdentity();
        glTranslatef(offsetX, -offsetY, 0.0f);
        glScalef(scale, scale, scale);


        glColor4f(1.0, 1.0, 1.0, 1.0);

        glBegin(GL_LINES);
        for(wxVector<WDEdMapEditor::LineDef>::iterator it = WDEdMapEditor::mapLinedefs.begin();
            it != WDEdMapEditor::mapLinedefs.end();
            ++it) {
                if(currentTool == WDED_ME_TOOL_LINES && &*it == hoveredLinedef) {
                    glColor4f(1.0, 0.6, 0.0, 1.0);
                }
                glVertex2i(WDEdMapEditor::mapVertexes[it->beginVertex].x, WDEdMapEditor::mapVertexes[it->beginVertex].y);
                glVertex2i(WDEdMapEditor::mapVertexes[it->endVertex].x, WDEdMapEditor::mapVertexes[it->endVertex].y);
                if(currentTool == WDED_ME_TOOL_LINES && &*it == hoveredLinedef) {
                    glColor4f(1.0, 1.0, 1.0, 1.0);
                }
        }
        glEnd();

        glLoadIdentity();
        glTranslatef(offsetX, -offsetY, 0.0f);

        if(WDEdMapEditor::currentTool == WDED_ME_TOOL_VERTS) {
            glBegin(GL_QUADS);
            for(wxVector<WDEdMapEditor::Vertex>::iterator it = WDEdMapEditor::mapVertexes.begin();
            it != WDEdMapEditor::mapVertexes.end();
            ++it) {
                if(&*it == hoveredVertex) {
                    glColor4f(1.0, 0.6, 0.0, 1.0);
                }
                glVertex2i(it->x * scale - 2, it->y * scale - 2);
                glVertex2i(it->x * scale - 2, it->y * scale + 2);
                glVertex2i(it->x * scale + 2, it->y * scale + 2);
                glVertex2i(it->x * scale + 2, it->y * scale - 2);
                if(&*it == hoveredVertex) {
                    glColor4f(1.0, 1.0, 1.0, 1.0);
                }
            }
            glEnd();
        }
    }

    

    glFlush();
    SwapBuffers();
}

void WDEdMainCanvas::StartDragging(wxMouseEvent& event) {
    SetFocus();
    CaptureMouse();
    wxSetCursor(wxCURSOR_SIZING);
    dragging = true;
    wxPoint mouseOnScreen = wxGetMousePosition();
    mousePrevX = mouseOnScreen.x;
    mousePrevY = mouseOnScreen.y;
}

void WDEdMainCanvas::EndDragging(wxMouseEvent& event) {
    ReleaseMouse();
    wxSetCursor(wxCURSOR_ARROW);
    dragging = false;
}

void WDEdMainCanvas::Drag(wxMouseEvent& event) {
    if(dragging) {
        wxPoint mouseOnScreen = wxGetMousePosition();
        offsetX += mouseOnScreen.x - mousePrevX;
        offsetY += mouseOnScreen.y - mousePrevY;
        mousePrevX = mouseOnScreen.x;
        mousePrevY = mouseOnScreen.y;
        Refresh();
        Update();
    } else {
        event.Skip();
    }
}

static double distanceFromPointToLine(double x1, double y1, double x2, double y2, double x3, double y3) {
    float px=x2-x1;
    float py=y2-y1;
    float temp=(px*px)+(py*py);
    float u=((x3 - x1) * px + (y3 - y1) * py) / (temp);
    if (u>1) {
        u=1;
    } else if (u<0) {
        u=0;
    }
    float x = x1 + u * px;
    float y = y1 + u * py;

    float dx = x - x3;
    float dy = y - y3;
    double dist = sqrt(dx*dx + dy*dy);
    return dist;
}

static double distance(double x1, double y1, double x2, double y2) {
    double square_difference_x = (x2 - x1) * (x2 - x1);
    double square_difference_y = (y2 - y1) * (y2 - y1);
    double sum = square_difference_x + square_difference_y;
    double value = sqrt(sum);
    return value;
}

void WDEdMainCanvas::MouseMove(wxMouseEvent& event) {
    wxPoint pos = event.GetPosition();
    int x = pos.x;
    int y = pos.y;
    x -= GetSize().x / 2;
    y -= GetSize().y / 2;
    x -= offsetX;
    y -= offsetY;
    x /= scale;
    y /= scale;
    y = -y;
    wxGetApp().frame->GetStatusBar()->SetStatusText(wxString::Format("(%d, %d)", x, y), WDED_SB_EL_COORDS);
    {
        LineDef *leastLine = nullptr;
        double leastDist = INFINITY;
        for(wxVector<WDEdMapEditor::LineDef>::iterator it = WDEdMapEditor::mapLinedefs.begin();
                it != WDEdMapEditor::mapLinedefs.end();
                ++it) {
                Vertex beginVert = mapVertexes[it->beginVertex];
                Vertex endVert = mapVertexes[it->endVertex];
                double dist = distanceFromPointToLine(beginVert.x, beginVert.y, endVert.x, endVert.y, x, y);

                if(dist < leastDist) {
                    leastDist = dist;
                    leastLine = &*it;
                }
            }
        if(leastDist > 5) leastLine = nullptr;
        if(hoveredLinedef != leastLine) {
            hoveredLinedef = leastLine;
            Refresh();
            Update();
        }
    }
    {
        Vertex *leastVert = nullptr;
        double leastDist = INFINITY;
        for(wxVector<WDEdMapEditor::Vertex>::iterator it = WDEdMapEditor::mapVertexes.begin();
            it != WDEdMapEditor::mapVertexes.end();
            ++it) {
                double dist = distance(it->x, it->y, x, y);
                if(dist < leastDist) {
                    leastDist = dist;
                    leastVert = &*it;
                }
            }
        if(leastDist > 5) leastVert = nullptr;
        if(hoveredVertex != leastVert) {
            hoveredVertex = leastVert;
            Refresh();
            Update();
        }
    }
}


void WDEdMainCanvas::Scale(float factor) {
    scale *= factor;
    Refresh();
    Update();
}

void WDEdMainCanvas::KeyDown(wxKeyEvent &event) {
    switch(event.GetKeyCode()) {
        case WXK_NUMPAD_ADD:
            Scale(2.0f);
        break;
        case WXK_NUMPAD_SUBTRACT:
            Scale(0.5f);
        break;
        case 91: // [
            gridSize /= 2;
            if(gridSize == 0) {
                gridSize = 1;
            }
            wxGetApp().frame->GetStatusBar()->SetStatusText(wxString::Format("Grid: %dx%d", gridSize, gridSize), WDED_SB_EL_GRID);
            Refresh();
            Update();
        break;
        case 93: // ]
            gridSize *= 2;
            wxGetApp().frame->GetStatusBar()->SetStatusText(wxString::Format("Grid: %dx%d", gridSize, gridSize), WDED_SB_EL_GRID);
            Refresh();
            Update();
        break;
        default:
            event.Skip();
        break;
    }
}

void WDEdMainCanvas::MouseLeftDown(wxMouseEvent& event) {
    SetFocus();
    event.Skip();
}

BEGIN_EVENT_TABLE(WDEdMainCanvas, wxGLCanvas)
    EVT_PAINT (WDEdMainCanvas::Render)
    EVT_MIDDLE_DOWN (WDEdMainCanvas::StartDragging)
    EVT_LEFT_DOWN (WDEdMainCanvas::MouseLeftDown)
    EVT_MOTION (WDEdMainCanvas::Drag)
    EVT_MOTION (WDEdMainCanvas::MouseMove)
    EVT_MIDDLE_UP (WDEdMainCanvas::EndDragging)
    EVT_KEY_DOWN (WDEdMainCanvas::KeyDown)
END_EVENT_TABLE()