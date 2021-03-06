#pragma once

#include <GL/glew.h>
#include <wx/wx.h>

#include "texture.h"

// #include "wad.h"

#include <vector>

#include "polygon_2d.h"

#include <cstring>

#include <map>

namespace WDEdMapEditor {
    enum WDEdMapEditorTool {
        WDED_ME_TOOL_VERTS,
        WDED_ME_TOOL_LINES,
		WDED_ME_TOOL_SECTORS,
    };

    enum WDEdMapEditorDragType {
        WDED_DRAG_NONE,
        WDED_DRAG_MOVESCREEN,
        WDED_DRAG_MOVEELEM,
		WDED_DRAG_DRAWLINE,
    };

    enum LinedefFlag {
        WDED_LINEFLAG_IMPASSABLE = 0x1
    };

    struct DoomSideDef {
        int16_t xOffset;
        int16_t yOffset;
        char upperTexture[8], lowerTexture[8], middleTexture[8];
        uint16_t sec;
    };

    struct DoomSector {
        int16_t floorHeight;
        int16_t ceilingHeight;
        char floorTexture[8], ceilingTexture[8];
        uint16_t lightLevel;
        uint16_t special;
        uint16_t tag;
    };

    struct DoomLineDef {
        uint16_t beginVertex;
        uint16_t endVertex;
        uint16_t flags;
        uint16_t linetype;
        uint16_t arg0;
        uint16_t frontSide;
        uint16_t backSide;
    };

    struct DoomVertex {
        int16_t x;
        int16_t y;
    };

    struct LineDef;
    struct Sector;
    struct SideDef;

    extern std::map<wxString, WDEdTexture2D *> wadTextures;
    extern std::map<wxString, wxImage *> wadPatches;

    struct Sector : public DoomSector {
        private:
            std::vector<SideDef *> sectorSides;
        public:
            inline std::vector<SideDef *> &connectedSides() {return sectorSides;}
            inline WDEdTexture2D * floorTex() {
                char name[9] = {0};
                for(int i = 0; i < 8; i++) {
                    name[i] = floorTexture[i];
                }
                if(!wadTextures.count(wxString(name))) {
                    return nullptr;
                }
                return wadTextures[wxString(name)];
            }
            void Split();
            PolygonGroup2D group;
    };

    struct Vertex : public DoomVertex {
    public:
    	std::vector<LineDef *> vertexLines;
    	inline Vertex() {} // @suppress("Class members should be properly initialized")
    	inline Vertex(int16_t x, int16_t y) {
    		this->x = x;
    		this->y = y;
    	}
    };

    extern std::vector<Sector> mapSectors;

    struct SideDef : public DoomSideDef {
    	inline SideDef(LineDef *prnt) {
    		xOffset = 0;
    		yOffset = 0;
    		strncpy(upperTexture, "-", 8);
    		strncpy(lowerTexture, "-", 8);
    		strncpy(middleTexture, "STONE2", 8);
    		sec = 0xFFFF;
    		parent = prnt;
    	}
        LineDef *parent;
        inline SideDef() : parent(nullptr) {}
        inline Sector *sector() {return sec == 0xFFFF ? nullptr : &mapSectors[sec];}
    };

    extern std::vector<Vertex> mapVertexes;
    extern std::vector<SideDef> mapSidedefs;

    struct LineDef : public DoomLineDef {
        inline Vertex *v1() {return &mapVertexes[beginVertex];}
        inline Vertex *v2() {return &mapVertexes[endVertex];}
        inline SideDef *s1() {return frontSide == 0xFFFF ? nullptr : &mapSidedefs[frontSide];}
        inline SideDef *s2() {return backSide == 0xFFFF ? nullptr : &mapSidedefs[backSide];}
        inline bool doubleSector() {return s1() && s2() && s1()->sec == s2()->sec && s2()->sec != 0xFFFF;}
    };

    union WDEdAnyElement {
        void *elem;
        LineDef *line;
        Vertex *vertex;
        Sector *sector;
        inline WDEdAnyElement() {};
        inline WDEdAnyElement(void *elem) : elem(elem) {};
        inline WDEdAnyElement(LineDef *line) : line(line) {};
        inline WDEdAnyElement(Vertex *vertex) : vertex(vertex) {};
        inline WDEdAnyElement(Sector *sector) : sector(sector) {};
    };

    extern WDEdMapEditorDragType dragging;
    extern WDEdAnyElement draggingElement;
    extern int mousePrevX, mousePrevY;
    
    extern float offsetX, offsetY;
    extern int gridSize;
    extern float scale;

    extern WDEdAnyElement hoveredElement;
    extern Vertex *hoveredVertex;
    
    extern WDEdMapEditorTool currentTool;

    extern int pointedX, pointedY, rawPointedX, rawPointedY;

    extern std::vector<LineDef> mapLinedefs;
    extern bool mapIsCurrentlyLoaded;

    void OpenArchive(wxString source);
    void SaveArchive(wxString target);
    void SetTool(WDEdMapEditorTool tool);
    bool IsElementHighlighted(const WDEdAnyElement &elem);
    void DeleteVertex(const uint16_t);
}
