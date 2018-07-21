#include "map_editor.h"

#include <wx/wfstream.h>
#include <wx/mstream.h>

#include "app.h"

#include "w_wad.h"

using namespace WDEdMapEditor;

WDEdMapEditorDragType WDEdMapEditor::dragging = WDED_DRAG_NONE;
WDEdAnyElement WDEdMapEditor::draggingElement = { (void*) nullptr };
int WDEdMapEditor::mousePrevX, WDEdMapEditor::mousePrevY;

float WDEdMapEditor::offsetX = 0, WDEdMapEditor::offsetY = 0;
int WDEdMapEditor::gridSize = 64;
float WDEdMapEditor::scale = 1.0f;

WDEdAnyElement WDEdMapEditor::hoveredElement = { (void*) nullptr };
Vertex *WDEdMapEditor::hoveredVertex = nullptr;
WDEdMapEditorTool WDEdMapEditor::currentTool = WDED_ME_TOOL_VERTS;

int WDEdMapEditor::pointedX = 0, WDEdMapEditor::pointedY = 0;

bool WDEdMapEditor::mapIsCurrentlyLoaded = false;
std::vector<LineDef> WDEdMapEditor::mapLinedefs;
std::vector<Vertex> WDEdMapEditor::mapVertexes;
std::vector<SideDef> WDEdMapEditor::mapSidedefs;
std::vector<Sector> WDEdMapEditor::mapSectors;

std::map<wxString, WDEdTexture2D *> WDEdMapEditor::wadTextures;
std::map<wxString, wxImage *> WDEdMapEditor::wadPatches;

// wxString ioTemporaryWadFile = wxFileName::CreateTempFileName("wDoomEd_");

void WDEdMapEditor::OpenArchive(wxString source) {
	mapIsCurrentlyLoaded = false;
	hoveredElement.elem = nullptr;

	bool verticesDefined = false;
	bool linedefsDefined = false;
	bool sidedefsDefined = false;
	bool sectorsDefined = false;

	const char *filenames[] = { (const char *) source.c_str(), nullptr };

	wxPrintf("Loading WADs:\n");

	W_InitMultipleFiles((char**) &filenames);

	{
		int mapnum = W_GetNumForName("MAP01");
		{
			int linenum = W_CheckNextNumForName(mapnum, "LINEDEFS");
			mapLinedefs.clear();
			int lumplen = W_LumpLength(linenum);
			char lumpdata[lumplen];
			W_ReadLump(linenum, (void *) &lumpdata);
			for (int i = 0; i < lumplen / sizeof(DoomLineDef); i++) {
				LineDef linedef;
				*((DoomLineDef *) &linedef) = ((DoomLineDef *) &lumpdata)[i];
				mapLinedefs.push_back(linedef);
			}
			linedefsDefined = true;
			wxPrintf("Defining linedefs\n");

		}
		{
			int vertnum = W_CheckNextNumForName(mapnum, "VERTEXES");
			mapVertexes.clear();
			int lumplen = W_LumpLength(vertnum);
			char lumpdata[lumplen];
			W_ReadLump(vertnum, (void *) &lumpdata);
			for (int i = 0; i < lumplen / sizeof(Vertex); i++) {
				Vertex vertex;
				vertex = ((Vertex *) &lumpdata)[i];
				mapVertexes.push_back(vertex);
			}
			verticesDefined = true;
			wxPrintf("Defining verts\n");

		}
		{
			int sidenum = W_CheckNextNumForName(mapnum, "SIDEDEFS");
			mapSidedefs.clear();
			int lumplen = W_LumpLength(sidenum);
			char lumpdata[lumplen];
			W_ReadLump(sidenum, (void *) &lumpdata);
			for (int i = 0; i < lumplen / sizeof(DoomSideDef); i++) {
				SideDef sideDef;
				*((DoomSideDef *) &sideDef) = ((DoomSideDef *) &lumpdata)[i];
				mapSidedefs.push_back(sideDef);
			}
			sidedefsDefined = true;
			wxPrintf("Defining sides\n");
		}
		{
			int secnum = W_CheckNextNumForName(mapnum, "SECTORS");
			mapSectors.clear();
			int lumplen = W_LumpLength(secnum);
			char lumpdata[lumplen];
			W_ReadLump(secnum, (void *) &lumpdata);
			for (int i = 0; i < lumplen / sizeof(DoomSector); i++) {
				Sector sec;
				*((DoomSector *) &sec) = ((DoomSector *) &lumpdata)[i];
				mapSectors.push_back(sec);
			}
			sectorsDefined = true;
			wxPrintf("Defining sectors\n");
		}
	}
	{
		unsigned char *buf = new unsigned char[768];
		W_ReadLumpN(W_GetNumForName((char *) "PLAYPAL"), (void *) buf, 768);
		wxGetApp().flatHandler->SetPalette(buf);
		wxGetApp().patchHandler->SetPalette(buf);
	}
	{
		int fsnum = W_GetNumForName("F_START") + 1;
		int fenum = W_GetNumForName("F_END");
		for (int i = fsnum; i < fenum; i++) {
			if (W_LumpLength(i) == 0)
				continue;
			unsigned char buf[W_LumpLength(i)];
			lumpinfo_t *lmp = lumpinfo + i;
			char s[9] = { 0 };
			strncpy(s, lmp->name, 8);
			W_ReadLump(i, buf);
			wxString name = s;
			wxMemoryInputStream mis(buf, sizeof(buf));
			wadTextures[name] = new WDEdTexture2D(new wxImage(mis), name);

		}
	}
	{
		char name[9] = {0};
		char *names = (char *) W_CacheLumpName("PNAMES");
		int nummappatches = *((int *) names);
		char *name_p = names + 4;

		for (int i = 0; i < nummappatches; i++) {
			strncpy(name, name_p + i * 8, 8);
			int lump = W_GetNumForName(name);
			unsigned char buf[W_LumpLength(lump)];
			W_ReadLump(lump, buf);
			wxMemoryInputStream mis(buf, sizeof buf);
			wxImage *img = new wxImage;
			wxGetApp().patchHandler->LoadFile(img, mis);
			wxString wname = name;
			wadPatches[wname] = img;
		}
	}

	W_CleanCache();

	/*if(testPatchTex) delete testPatchTex;
	testPatchTex = new WDEdTexture2D(wadPatches["RW47_1"]);*/

	if (verticesDefined && linedefsDefined && sidedefsDefined
			&& sectorsDefined) {
		for (std::vector<SideDef>::iterator it = mapSidedefs.begin();
				it != mapSidedefs.end(); it++) {
			if (it->sector()) {
				it->sector()->connectedSides().push_back(&*it);
			}
		}
		for (std::vector<LineDef>::iterator it = mapLinedefs.begin();
				it != mapLinedefs.end(); it++) {
			if (it->s1()) {
				if (it->s1()->parent)
					wxPrintf("WARNING: Two linedefs reference one sidedef");
				else
					it->s1()->parent = &*it;
			}
			if (it->s2()) {
				if (it->s2()->parent)
					wxPrintf("WARNING: Two linedefs reference one sidedef");
				else
					it->s2()->parent = &*it;
			}
		}
		PolygonSplitter splitter;
		for (std::vector<Sector>::iterator it = mapSectors.begin();
				it != mapSectors.end(); it++) {
			splitter.clear();
			splitter.openSector(&*it);
			PolygonGroup2D polyGroup;
			if (splitter.doSplitting(&polyGroup)) {
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
	// if(mapIsCurrentlyLoaded) wxCopyFile(source, ioTemporaryWadFile);
}

void WDEdMapEditor::SaveArchive(wxString target) {
	if (!mapIsCurrentlyLoaded)
		return;

	/*WDEdWADInputStream wis(new wxFileInputStream(ioTemporaryWadFile));
	 WDEdWADOutputStream wos(new wxFileOutputStream(target), wis.GetWadType());*/
}

void WDEdMapEditor::SetTool(WDEdMapEditorTool tool) {
	hoveredElement.elem = nullptr;
	draggingElement.elem = nullptr;
	dragging = WDED_DRAG_NONE;
	currentTool = tool;
}

bool WDEdMapEditor::IsElementHighlighted(const WDEdAnyElement &elem) {
	return (dragging == WDED_DRAG_NONE && (elem.elem == hoveredElement.elem || elem.vertex == hoveredVertex))
			|| elem.elem == draggingElement.elem;
}
