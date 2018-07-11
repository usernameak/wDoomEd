#include "map_editor.h"

#include <wx/wfstream.h>

#include "app.h"

using namespace WDEdMapEditor;

WDEdMapEditorDragType WDEdMapEditor::dragging = WDED_DRAG_NONE;
WDEdAnyElement WDEdMapEditor::draggingElement = {(void*)nullptr};
int WDEdMapEditor::mousePrevX, WDEdMapEditor::mousePrevY;

float WDEdMapEditor::offsetX = 0, WDEdMapEditor::offsetY = 0;
int WDEdMapEditor::gridSize = 64;
float WDEdMapEditor::scale = 1.0f;

WDEdAnyElement WDEdMapEditor::hoveredElement = {(void*)nullptr};
WDEdMapEditorTool WDEdMapEditor::currentTool = WDED_ME_TOOL_VERTS;

int WDEdMapEditor::pointedX = 0, WDEdMapEditor::pointedY = 0;

bool WDEdMapEditor::mapIsCurrentlyLoaded = false;
std::vector<LineDef> WDEdMapEditor::mapLinedefs;
std::vector<Vertex> WDEdMapEditor::mapVertexes;
std::vector<SideDef> WDEdMapEditor::mapSidedefs;
std::vector<Sector> WDEdMapEditor::mapSectors;

std::map<wxString, WDEdTexture2D *> WDEdMapEditor::wadTextures;

wxString ioTemporaryWadFile = wxFileName::CreateTempFileName("wDoomEd_");


void WDEdMapEditor::OpenArchive(wxString source) {
    mapIsCurrentlyLoaded = false;
    hoveredElement.elem = nullptr;

    bool verticesDefined = false;
    bool linedefsDefined = false;
    bool sidedefsDefined = false;
    bool sectorsDefined = false;

    WDEdWADInputStream wis(new wxFileInputStream(source));
    while(wxArchiveEntry *entry = wis.GetNextEntry()) {
        if(entry->GetName().IsSameAs("MAP01")) {
            while(wxArchiveEntry *entry2 = wis.GetNextEntry()) {
                wxString entry2Name = entry2->GetName();
                if(!linedefsDefined && entry2Name.IsSameAs("LINEDEFS")) {
                    mapLinedefs.clear();
                    LineDef linedef;
                    while(!wis.Eof()) {
                        wis.Read(&linedef, sizeof(DoomLineDef));
                        mapLinedefs.push_back(linedef);
                    }
                    linedefsDefined = true;
                    wxPrintf("Defining linedefs\n");
                } else if(!verticesDefined && entry2Name.IsSameAs("VERTEXES")) {
                    mapVertexes.clear();
                    Vertex vertex;
                    while(!wis.Eof()) {
                        wis.Read(&vertex, sizeof(Vertex));
                        mapVertexes.push_back(vertex);
                    }
                    verticesDefined = true;
                    wxPrintf("Defining verts\n");
                } else if(!sidedefsDefined && entry2Name.IsSameAs("SIDEDEFS")) {
                    mapSidedefs.clear();
                    SideDef side;
                    while(!wis.Eof()) {
                        wis.Read(&side, sizeof(DoomSideDef));
                        mapSidedefs.push_back(side);
                    }
                    sidedefsDefined = true;
                    wxPrintf("Defining sides\n");
                } else if(!sectorsDefined && entry2Name.IsSameAs("SECTORS")) {
                    mapSectors.clear();
                    Sector sec;
                    while(!wis.Eof()) {
                        wis.Read(&sec, sizeof(DoomSector));
                        mapSectors.push_back(sec);
                    }
                    sectorsDefined = true;
                    wxPrintf("Defining sectors\n");
                } else if(!entry2Name.IsSameAs("THINGS") &&
                          !entry2Name.IsSameAs("SEGS") &&
                          !entry2Name.IsSameAs("SSECTORS") &&
                          !entry2Name.IsSameAs("NODES") &&
                          !entry2Name.IsSameAs("REJECT") &&
                          !entry2Name.IsSameAs("BLOCKMAP")) {
                    break;
                }
            }
        } else if(entry->GetName().IsSameAs("F_START")) {
            while(wxArchiveEntry *entry2 = wis.GetNextEntry()) {
                if(entry2->GetName().IsSameAs("F_END")) {
                    break;
                } else {
                    if(entry2->GetSize() == 0) continue;
                    wadTextures[entry2->GetName()] = new WDEdTexture2D(new wxImage(wis), entry2->GetName());
                }
            }
        } else if(entry->GetName().IsSameAs("PLAYPAL")) {
            unsigned char *buf = new unsigned char[768];
            if(!wis.ReadAll(buf, 768)) {
                wxPrintf("PLAYPAL seems to be too small. That's not okay.\n");
            }
            wxGetApp().flatHandler->SetPalette(buf);
            
        }
    }

    if(verticesDefined && linedefsDefined && sidedefsDefined && sectorsDefined) {
        for(std::vector<SideDef>::iterator it = mapSidedefs.begin(); it != mapSidedefs.end(); it++) {
            if(it->sector()) {
                it->sector()->connectedSides().push_back(&*it);
            }
        }
        for(std::vector<LineDef>::iterator it = mapLinedefs.begin(); it != mapLinedefs.end(); it++) {
            if(it->s1()) {
                if(it->s1()->parent)
                    wxPrintf("WARNING: Two linedefs reference one sidedef");
                else
                    it->s1()->parent = &*it;
            }
            if(it->s2()) {
                if(it->s2()->parent)
                    wxPrintf("WARNING: Two linedefs reference one sidedef");
                else
                    it->s2()->parent = &*it;
            }
        }
        PolygonSplitter splitter;
        for(std::vector<Sector>::iterator it = mapSectors.begin(); it != mapSectors.end(); it++) {
            splitter.clear();
            splitter.openSector(&*it);
            PolygonGroup2D polyGroup;
            if(splitter.doSplitting(&polyGroup)) {
                it->nTriangles = polyGroup.countVertices();
                it->triangles = new WDEdMathUtil::Point[it->nTriangles];
                polyGroup.triangulate(it->triangles);
            } else {
                it->nTriangles = 0;
            }

        }
        mapIsCurrentlyLoaded = true;
        wxGetApp().frame->canvas->Refresh();
        wxGetApp().frame->canvas->Update();
    }
    if(mapIsCurrentlyLoaded) wxCopyFile(source, ioTemporaryWadFile);
}

void WDEdMapEditor::SaveArchive(wxString target) {
    if(!mapIsCurrentlyLoaded) return;
    
    WDEdWADInputStream wis(new wxFileInputStream(ioTemporaryWadFile));
    WDEdWADOutputStream wos(new wxFileOutputStream(target), wis.GetWadType());
}

void WDEdMapEditor::SetTool(WDEdMapEditorTool tool) {
    hoveredElement.elem = nullptr;
    draggingElement.elem = nullptr;
    dragging = WDED_DRAG_NONE;
    currentTool = tool;
}

bool WDEdMapEditor::IsElementHighlighted(const WDEdAnyElement &elem) {
    return (dragging == WDED_DRAG_NONE && elem.elem == hoveredElement.elem) || elem.elem == draggingElement.elem;
}