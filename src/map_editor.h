#pragma once

#include <wx/wx.h>

#include "wad.h"

namespace WDEdMapEditor {
    enum WDEdMapEditorTool {
        WDED_ME_TOOL_VERTS,
        WDED_ME_TOOL_LINES,
    };

    enum WDEdMapEditorDragType {
        WDED_DRAG_NONE,
        WDED_DRAG_MOVESCREEN,
        WDED_DRAG_MOVEELEM,
    };

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

    union WDEdAnyElement {
        void *elem;
        LineDef *line;
        Vertex *vertex;
        inline WDEdAnyElement() {};
        inline WDEdAnyElement(void * elem) : elem(elem) {};
        inline WDEdAnyElement(LineDef *line) : line(line) {};
        inline WDEdAnyElement(Vertex *vertex) : vertex(vertex) {};
    };

    extern WDEdMapEditorDragType dragging;
    extern WDEdAnyElement draggingElement;
    extern int mousePrevX, mousePrevY;
    
    extern float offsetX, offsetY;
    extern int gridSize;
    extern float scale;

    extern WDEdAnyElement hoveredElement;
    
    extern WDEdMapEditorTool currentTool;

    extern int pointedX, pointedY;

    extern wxVector<LineDef> mapLinedefs;
    extern wxVector<Vertex> mapVertexes;
    extern bool mapIsCurrentlyLoaded;


    void OpenArchive(wxString source);
    void SaveArchive(wxString target);
    void SetTool(WDEdMapEditorTool tool);
    bool IsElementHighlighted(const WDEdAnyElement &elem);
}