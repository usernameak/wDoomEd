#include "map_editor.h"

#include <wx/wfstream.h>

#include "app.h"

using namespace WDEdMapEditor;

bool WDEdMapEditor::mapIsCurrentlyLoaded = false;
wxVector<LineDef> WDEdMapEditor::mapLinedefs;
wxVector<Vertex> WDEdMapEditor::mapVertexes;

bool WDEdMapEditor::dragging = false;
float WDEdMapEditor::offsetX = 0, WDEdMapEditor::offsetY = 0;
int WDEdMapEditor::mousePrevX, WDEdMapEditor::mousePrevY;
int WDEdMapEditor::gridSize = 64;
float WDEdMapEditor::scale = 1.0f;
LineDef *WDEdMapEditor::hoveredLinedef = nullptr;
Vertex *WDEdMapEditor::hoveredVertex = nullptr;
WDEdMapEditorTool WDEdMapEditor::currentTool = WDED_ME_TOOL_VERTS;


int WDEdMapEditor::pointedX = 0, WDEdMapEditor::pointedY = 0;

void WDEdMapEditor::OpenArchive(wxString source) {
    mapIsCurrentlyLoaded = false;
    hoveredLinedef = nullptr;
    WDEdWADInputStream wis(new wxFileInputStream(source));
    while(wxArchiveEntry *entry = wis.GetNextEntry()) {
        if(entry->GetName().IsSameAs("MAP01")) {
            bool verticesDefined = false;
            bool linedefsDefined = false;
            while(wxArchiveEntry *entry = wis.GetNextEntry()) {
                if(!linedefsDefined && entry->GetName().IsSameAs("LINEDEFS")) {
                    mapLinedefs.clear();
                    LineDef linedef;
                    while(!wis.Eof()) {
                        wis.Read(&linedef, sizeof(LineDef));
                        mapLinedefs.push_back(linedef);
                    }
                    linedefsDefined = true;
                    wxPrintf("Defining linedefs\n");
                } else if(!verticesDefined && entry->GetName().IsSameAs("VERTEXES")) {
                    mapVertexes.clear();
                    Vertex vertex;
                    while(!wis.Eof()) {
                        wis.Read(&vertex, sizeof(Vertex));
                        mapVertexes.push_back(vertex);
                    }
                    verticesDefined = true;
                    wxPrintf("Defining verts\n");
                }
                if(verticesDefined && linedefsDefined) {
                    mapIsCurrentlyLoaded = true;
                    wxGetApp().frame->canvas->Refresh();
                    wxGetApp().frame->canvas->Update();
                }
            }
            break;
        }
    }
}