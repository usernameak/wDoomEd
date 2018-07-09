#pragma once

#include <wx/wx.h>

#include "wad.h"

namespace WDEdMapEditor {
    struct LineDef {
        uint16_t beginVertex;
        uint16_t endVertex;
        uint16_t flags;
        uint16_t linetype;
        uint16_t arg0;
        uint16_t frontSide;
        uint16_t backSide;
    };

    struct Vertex {
        int16_t x;
        int16_t y;
    };


    extern bool dragging;
    extern float offsetX, offsetY;
    extern int mousePrevX, mousePrevY;
    extern int gridSize;
    extern float scale;
    extern LineDef *hoveredLinedef;

    extern wxVector<LineDef> mapLinedefs;
    extern wxVector<Vertex> mapVertexes;
    extern bool mapIsCurrentlyLoaded;
    void OpenArchive(wxString source);
}