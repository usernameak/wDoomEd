#include "canvas.h"

#include "map_editor.h"
#include "app.h"
#include "properties_dialog.h"

#include <cmath>
#include <climits>
#include <algorithm>

#include <GL/glu.h>

using namespace WDEdMapEditor;

// extern WDEdTexture2D *testPatchTex;

WDEdMainCanvas::WDEdMainCanvas(wxWindow *parent, const int *attribs) :
		wxGLCanvas::wxGLCanvas(parent, wxID_ANY, attribs) {
	ctx = new wxGLContext(this);
	wxGetApp().frame->GetStatusBar()->SetStatusText(
			wxString::Format("Grid: %dx%d", gridSize, gridSize),
			WDED_SB_EL_GRID);
}

void WDEdMainCanvas::Render(wxPaintEvent& WXUNUSED(event)) {
	if (scale == 0.0f)
		scale = 1.0f;
	SetCurrent(*ctx);
	wxPaintDC(this);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, (GLint) GetSize().x, (GLint) GetSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, (GLint) GetSize().x, 0, (GLint) GetSize().y);
	glTranslatef((GLint) GetSize().x / 2, (GLint) GetSize().y / 2, 0.0f);

	glMatrixMode(GL_MODELVIEW);

	if (WDEdMapEditor::mapIsCurrentlyLoaded) {
		glLoadIdentity();
		glTranslatef(offsetX, -offsetY, 0.0f);
		glScalef(scale, scale, scale);

		RenderSectors();
		RenderGrid();
		RenderLines();

		if (WDEdMapEditor::currentTool == WDED_ME_TOOL_VERTS || (WDEdMapEditor::currentTool == WDED_ME_TOOL_LINES && wxGetKeyState(WXK_SHIFT))) {
			glLoadIdentity();
			glTranslatef(offsetX, -offsetY, 0.0f);
			RenderVertices();
		}
	}

	/*if(testPatchTex) {
	 glEnable(GL_TEXTURE_2D);
	 glEnable(GL_ALPHA_TEST);
	 glAlphaFunc(GL_GREATER, 0.5f);
	 testPatchTex->Bind(ctx);
	 glColor3f(1.0f, 1.0f, 1.0f);
	 glLoadIdentity();
	 glBegin(GL_QUADS);
	 glTexCoord2i(0, 0); glVertex2i(0, 0);
	 glTexCoord2i(1, 0); glVertex2i(64, 0);
	 glTexCoord2i(1, 1); glVertex2i(64, 128);
	 glTexCoord2i(0, 1); glVertex2i(0, 128);
	 glEnd();
	 glDisable(GL_TEXTURE_2D);
	 glDisable(GL_ALPHA_TEST);
	 }*/

	glFlush();
	SwapBuffers();
}

void WDEdMainCanvas::RenderSectors() {
	for (std::vector<WDEdMapEditor::Sector>::iterator it =
			WDEdMapEditor::mapSectors.begin();
			it != WDEdMapEditor::mapSectors.end(); ++it) {
		glColor4f(it->lightLevel / 256.0f, it->lightLevel / 256.0f,
				it->lightLevel / 256.0f, 1.0);

		WDEdTexture2D *tex = it->floorTex();
		if (tex) {
			glEnable(GL_TEXTURE_2D);
			tex->Bind(ctx);
		}

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < it->nTriangles; i++) {
			if (tex) {
				glTexCoord2f(it->triangles[i].x / tex->imageWidth,
						it->triangles[i].y / tex->imageHeight);
			}
			glVertex2f(it->triangles[i].x, it->triangles[i].y);
		}
		glEnd();

		if (tex) {
			glDisable(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void WDEdMainCanvas::RenderGrid() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 0.2);

	glBegin(GL_LINES);
	int step = gridSize;
	float initialX = -GetSize().x * 0.5 / scale - offsetX / scale;
	initialX = initialX - fmod(initialX, step) - step;
	float finalX = GetSize().x * 0.5 / scale - offsetX / scale;
	float initialY = -GetSize().y * 0.5 / scale + offsetY / scale;
	initialY = initialY - fmod(initialY, step) - step;
	float finalY = GetSize().y * 0.5 / scale + offsetY / scale;
	for (float x = initialX; x < finalX; x += step) {
		if (x == pointedX) {
			glColor4f(1.0, 1.0, 1.0, 0.4);
		}
		glVertex2f(x, initialY);
		glVertex2f(x, finalY);
		if (x == pointedX) {
			glColor4f(1.0, 1.0, 1.0, 0.2);
		}
	}
	for (float y = initialY; y < finalY; y += step) {
		if (y == pointedY) {
			glColor4f(1.0, 1.0, 1.0, 0.4);
		}
		glVertex2f(initialX, y);
		glVertex2f(finalX, y);
		if (y == pointedY) {
			glColor4f(1.0, 1.0, 1.0, 0.2);
		}
	}
	glEnd();

	glDisable(GL_BLEND);
}

void WDEdMainCanvas::RenderLines() {
	glColor4f(1.0, 1.0, 1.0, 1.0);

	glBegin(GL_LINES);
	for (std::vector<WDEdMapEditor::LineDef>::iterator it =
			WDEdMapEditor::mapLinedefs.begin();
			it != WDEdMapEditor::mapLinedefs.end(); ++it) {
		if (IsElementHighlighted(&*it)) {
			glColor4f(1.0, 0.6, 0.0, 1.0);
		} else if (!(it->flags & WDED_LINEFLAG_IMPASSABLE)) {
			glColor4f(0.6, 0.6, 0.6, 1.0);
		}
		int x1 = WDEdMapEditor::mapVertexes[it->beginVertex].x;
		int y1 = WDEdMapEditor::mapVertexes[it->beginVertex].y;
		int x2 = WDEdMapEditor::mapVertexes[it->endVertex].x;
		int y2 = WDEdMapEditor::mapVertexes[it->endVertex].y;

		glVertex2i(WDEdMapEditor::mapVertexes[it->beginVertex].x,
				WDEdMapEditor::mapVertexes[it->beginVertex].y);
		glVertex2i(WDEdMapEditor::mapVertexes[it->endVertex].x,
				WDEdMapEditor::mapVertexes[it->endVertex].y);
		float dx = x2 - x1;
		float dy = y2 - y1;
		int cx = (x1 + x2) / 2;
		int cy = (y1 + y2) / 2;
		float len = sqrt(dx * dx + dy * dy);
		if(len == 0) {
			dx = dy = 0;
		} else {
			dx /= len;
			dy /= len;
		}
		dx *= 5;
		dy *= 5;
		glVertex2i(cx, cy);
		glVertex2f(cx + dy, cy - dx);
		if (IsElementHighlighted(&*it)
				|| !(it->flags & WDED_LINEFLAG_IMPASSABLE)) {
			glColor4f(1.0, 1.0, 1.0, 1.0);
		}
	}
	glEnd();
}

void WDEdMainCanvas::RenderVertices() {
	glBegin(GL_QUADS);
	for (std::vector<WDEdMapEditor::Vertex>::reverse_iterator it =
			WDEdMapEditor::mapVertexes.rbegin();
			it != WDEdMapEditor::mapVertexes.rend(); ++it) {
		if (IsElementHighlighted(&*it)) {
			glColor4f(1.0, 0.6, 0.0, 1.0);
		}
		glVertex2i(it->x * scale - 2, it->y * scale - 2);
		glVertex2i(it->x * scale - 2, it->y * scale + 2);
		glVertex2i(it->x * scale + 2, it->y * scale + 2);
		glVertex2i(it->x * scale + 2, it->y * scale - 2);
		if (IsElementHighlighted(&*it)) {
			glColor4f(1.0, 1.0, 1.0, 1.0);
		}
	}
	glEnd();
}

wxPoint WDEdMainCanvas::convertCoordsScreenToWorld(wxPoint &pos) {
	wxPoint ret(pos);
	ret.x -= GetSize().x / 2;
	ret.y -= GetSize().y / 2;
	ret.x -= offsetX;
	ret.y -= offsetY;
	ret.x /= scale;
	ret.y /= scale;
	ret.y = -ret.y;
	return ret;
}

void WDEdMainCanvas::StartDragging(wxMouseEvent& event) {
	SetFocus();
	if (event.Button(wxMOUSE_BTN_MIDDLE)) {
		CaptureMouse();
		wxSetCursor(wxCURSOR_SIZING);
		dragging = WDED_DRAG_MOVESCREEN;
	} else if (event.Button(wxMOUSE_BTN_LEFT)) {
		CaptureMouse();
		dragging = WDED_DRAG_MOVEELEM;

		if (wxGetKeyState(WXK_SHIFT)) {
			switch (currentTool) {
			case WDED_ME_TOOL_VERTS:
				mapVertexes.push_back(Vertex(pointedX, pointedY));
				hoveredElement = &mapVertexes.back();
				break;
			case WDED_ME_TOOL_LINES:
				LineDef line;
				std::vector<Vertex>::iterator v1;
				if(!hoveredVertex ||
						(v1 = std::find_if(mapVertexes.begin(), mapVertexes.end(),
							[](const Vertex &vc){
								return &vc == hoveredVertex;
							})) == mapVertexes.end()) {
					mapVertexes.push_back(Vertex(pointedX, pointedY));
					line.beginVertex = mapVertexes.size() - 1;
				} else {
					line.beginVertex = v1 - mapVertexes.begin();
				}
				mapVertexes.push_back(Vertex(pointedX, pointedY));
				line.endVertex = mapVertexes.size() - 1;
				line.flags = 1;
				line.linetype = 0;
				line.arg0 = 0;
				line.frontSide = mapSidedefs.size();
				line.backSide = 0xFFFF;
				mapLinedefs.push_back(line);
				mapSidedefs.push_back(SideDef(&mapLinedefs.back()));
				dragging = WDED_DRAG_DRAWLINE;
				draggingElement.line = &mapLinedefs.back();
				goto getout;
				break;
			}
		}
		draggingElement = hoveredElement;
	}
	getout:;

	Refresh(); Update();

	wxPoint mouseOnScreen = wxGetMousePosition();
	mousePrevX = mouseOnScreen.x;
	mousePrevY = mouseOnScreen.y;

	event.Skip();
}

void WDEdMainCanvas::EndDragging(wxMouseEvent& event) {
	ReleaseMouse();
	wxSetCursor(wxCURSOR_ARROW);
	if(dragging == WDED_DRAG_DRAWLINE) {
		if(hoveredVertex) {
			DeleteVertexAndShiftRefs(draggingElement.line->endVertex);
			draggingElement.line->endVertex = std::find_if(mapVertexes.begin(), mapVertexes.end(),
					[](const Vertex &vc){
						return &vc == hoveredVertex;
					}) - mapVertexes.begin();
		}
	}
	dragging = WDED_DRAG_NONE;
	draggingElement.elem = nullptr;
}

void WDEdMainCanvas::Drag(wxMouseEvent& event) {
	if (dragging == WDED_DRAG_MOVESCREEN) {
		wxPoint mouseOnScreen = wxGetMousePosition();
		offsetX += mouseOnScreen.x - mousePrevX;
		offsetY += mouseOnScreen.y - mousePrevY;
		mousePrevX = mouseOnScreen.x;
		mousePrevY = mouseOnScreen.y;
		Refresh();
		Update();
	} else if (dragging == WDED_DRAG_MOVEELEM && draggingElement.elem) {
		wxPoint pos = event.GetPosition();
		wxPoint wpos = convertCoordsScreenToWorld(pos);
		pointedX = round((double) wpos.x / gridSize) * gridSize;
		pointedY = round((double) wpos.y / gridSize) * gridSize;
		switch (currentTool) {
		case WDED_ME_TOOL_VERTS:
			draggingElement.vertex->x = pointedX;
			draggingElement.vertex->y = pointedY;
			break;
		case WDED_ME_TOOL_LINES:

			break;
		}
	} else if (dragging == WDED_DRAG_DRAWLINE) {
		wxPoint pos = event.GetPosition();
		wxPoint wpos = convertCoordsScreenToWorld(pos);
		pointedX = round((double) wpos.x / gridSize) * gridSize;
		pointedY = round((double) wpos.y / gridSize) * gridSize;
		mapVertexes[draggingElement.line->endVertex].x = pointedX;
		mapVertexes[draggingElement.line->endVertex].y = pointedY;
		// TODO: closing drawn lines
	}
	event.Skip();
}

static double distanceFromPointToLine(double x1, double y1, double x2,
		double y2, double x3, double y3) {
	float px = x2 - x1;
	float py = y2 - y1;
	float temp = (px * px) + (py * py);
	float u = ((x3 - x1) * px + (y3 - y1) * py) / (temp);
	if (u > 1) {
		u = 1;
	} else if (u < 0) {
		u = 0;
	}
	float x = x1 + u * px;
	float y = y1 + u * py;

	float dx = x - x3;
	float dy = y - y3;
	double dist = sqrt(dx * dx + dy * dy);
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
	wxPoint wpos = convertCoordsScreenToWorld(pos);

	pointedX = round((double) wpos.x / gridSize) * gridSize;
	pointedY = round((double) wpos.y / gridSize) * gridSize;

	wxGetApp().frame->GetStatusBar()->SetStatusText(
			wxString::Format("(%d, %d)", pointedX, pointedY),
			WDED_SB_EL_COORDS);

	if (currentTool == WDED_ME_TOOL_LINES) {
		LineDef *leastLine = nullptr;
		double leastDist = INFINITY;
		for (std::vector<WDEdMapEditor::LineDef>::iterator it =
				WDEdMapEditor::mapLinedefs.begin();
				it != WDEdMapEditor::mapLinedefs.end(); ++it) {
			Vertex beginVert = mapVertexes[it->beginVertex];
			Vertex endVert = mapVertexes[it->endVertex];
			double dist = distanceFromPointToLine(beginVert.x, beginVert.y,
					endVert.x, endVert.y, wpos.x, wpos.y);

			if (dist < leastDist) {
				leastDist = dist;
				leastLine = &*it;
			}
		}
		if (leastDist > 12 / scale)
			leastLine = nullptr;
		if (hoveredElement.line != leastLine) {
			hoveredElement.line = leastLine;
		}
	}
	{
		Vertex *leastVert = nullptr;
		double leastDist = INFINITY;
		for (std::vector<WDEdMapEditor::Vertex>::iterator it =
				WDEdMapEditor::mapVertexes.begin();
				it != WDEdMapEditor::mapVertexes.end(); ++it) {
			double dist = distance(it->x, it->y, wpos.x, wpos.y);
			if (dist < leastDist && !(dragging == WDED_DRAG_DRAWLINE && it - mapVertexes.begin() == draggingElement.line->beginVertex)) {
				leastDist = dist;
				leastVert = &*it;
			}
		}
		if (leastDist > 12 / scale)
			leastVert = nullptr;
		if (currentTool == WDED_ME_TOOL_VERTS && hoveredElement.vertex != leastVert) {
			hoveredElement.vertex = leastVert;
		}
		if(hoveredVertex != leastVert) {
			hoveredVertex = leastVert;
		}
	}
	Refresh();
	Update();
}

void WDEdMainCanvas::Scale(float factor) {
	scale *= factor;
	Refresh();
	Update();
}

void WDEdMainCanvas::KeyDown(wxKeyEvent &event) {
	switch (event.GetKeyCode()) {
	case WXK_NUMPAD_ADD:
		Scale(2.0f);
		break;
	case WXK_NUMPAD_SUBTRACT:
		Scale(0.5f);
		break;
	case 91: // [
		gridSize /= 2;
		if (gridSize == 0) {
			gridSize = 1;
		}
		wxGetApp().frame->GetStatusBar()->SetStatusText(
				wxString::Format("Grid: %dx%d", gridSize, gridSize),
				WDED_SB_EL_GRID);
		Refresh();
		Update();
		break;
	case 93: // ]
		gridSize *= 2;
		wxGetApp().frame->GetStatusBar()->SetStatusText(
				wxString::Format("Grid: %dx%d", gridSize, gridSize),
				WDED_SB_EL_GRID);
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

void WDEdMainCanvas::OpenPropertiesMenu(wxMouseEvent& event) {
	if (hoveredElement.elem) {
		WDEdPropertiesDialog dlg(wxGetApp().frame);
		switch (currentTool) {
		case WDED_ME_TOOL_VERTS:
			dlg.AddGroup("Position");
			dlg.AddNumberField<int16_t>("X coordinate",
					&hoveredElement.vertex->x);
			dlg.AddNumberField<int16_t>("Y coordinate",
					&hoveredElement.vertex->y);
			break;
		case WDED_ME_TOOL_LINES:
			dlg.AddBitCheckboxGroup<uint16_t>("Flags",
					&hoveredElement.line->flags,
					{ { "Impassable", 0x0001 },
					  { "Block monsters", 0x0002 },
					  { "Two sided", 0x0004 },
					  { "Upper unpegged", 0x0008 },
					  { "Lower unpegged", 0x0010 },
					  { "Secret", 0x0020 },
					  { "Block sound", 0x0040 },
					  { "Not on automap", 0x0080 },
					  { "Always on automap", 0x0100 },
					});
			dlg.AddGroup("Special");
			dlg.AddNumberField<uint16_t>("Line special",
					&hoveredElement.line->linetype);
			dlg.AddNumberField<uint16_t>("Special tag",
					&hoveredElement.line->arg0);
			break;
		}
		dlg.Finish();
		if (dlg.ShowModal() == wxID_OK) {
			Refresh();
			Update();
		}
	}
}

BEGIN_EVENT_TABLE(WDEdMainCanvas, wxGLCanvas) EVT_PAINT (WDEdMainCanvas::Render)
EVT_LEFT_DOWN (WDEdMainCanvas::StartDragging)
EVT_MIDDLE_DOWN (WDEdMainCanvas::StartDragging)
EVT_LEFT_DOWN (WDEdMainCanvas::MouseLeftDown)
EVT_MOTION (WDEdMainCanvas::Drag)
EVT_MOTION (WDEdMainCanvas::MouseMove)
EVT_MIDDLE_UP (WDEdMainCanvas::EndDragging)
EVT_LEFT_UP (WDEdMainCanvas::EndDragging)
EVT_RIGHT_UP (WDEdMainCanvas::OpenPropertiesMenu)
EVT_KEY_DOWN (WDEdMainCanvas::KeyDown)
END_EVENT_TABLE()
