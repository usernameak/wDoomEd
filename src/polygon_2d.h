
#ifndef __POLYGON_2D_H__
#define __POLYGON_2D_H__

#include <vector>
#include <wx/glcanvas.h>

#include "texture.h"

#include "math_util.h"

namespace WDEdMapEditor {
	struct Sector;
}

struct Polygon2D {
	unsigned int n_vertices;
	WDEdMathUtil::Point *vertices;
};

struct PolygonGroup2D {
	std::vector<Polygon2D> subPolys;
	void addSubPoly() {subPolys.emplace_back();}
	Polygon2D *getSubPoly(int i) {return &subPolys[i];}
	int nSubPolys() {return subPolys.size();}
	void removeSubPoly(int i) {subPolys.erase(subPolys.begin() + i);}
	int countVertices();
	inline void reset() {
		if(triangles) {
			delete[] triangles;
			triangles = nullptr;
		}
		subPolys.clear();
		needsUpdate = true;
	}
	void triangulate();
	void setTexture(WDEdTexture2D *);
	void setupVBO(GLuint);
	~PolygonGroup2D();
    int nTriangles = 0;
private:
	WDEdTexture2D *tex = nullptr;
    float *triangles = nullptr;
    bool needsUpdate = true;
};

class PolygonSplitter
{
friend class Polygon2D;
private:
	// Structs
	struct edge_t
	{
		int		v1, v2;
		bool	ok;
		bool	done;
		bool	inpoly;
		int		sister;
	};
	struct vertex_t
	{
		double 			x, y;
		std::vector<int>		edges_in;
		std::vector<int>		edges_out;
		bool			ok;
		double			distance;
		vertex_t(double x=0, double y=0) { this->x = x; this->y = y; ok = true; }
		operator WDEdMathUtil::Point() const {return WDEdMathUtil::Point((float)x, (float)y);}
	};
	struct poly_outline_t
	{
		std::vector<int>	edges;
		WDEdMathUtil::BoundingBox		bbox;
		bool		clockwise;
		bool		convex;
	};

	// Splitter data
	std::vector<vertex_t>		vertices;
	std::vector<edge_t>			edges;
	std::vector<int>				concave_edges;
	std::vector<poly_outline_t>	polygon_outlines;
	int						split_edges_start;
	bool					verbose;
	double					last_angle;

public:
	PolygonSplitter();
	~PolygonSplitter();

	void	clear();
	void	setVerbose(bool v) { verbose = v; }

	int		addVertex(double x, double y);
	int		addEdge(double x1, double y1, double x2, double y2);
	int		addEdge(int v1, int v2);

	int		findNextEdge(int edge, bool ignore_valid = true, bool only_convex = true, bool ignore_inpoly = false);
	void	flipEdge(int edge);

	void	detectConcavity();
	bool	detectUnclosed();

	bool	tracePolyOutline(int edge_start);
	bool	testTracePolyOutline(int edge_start);

	bool	splitFromEdge(int splitter_edge);
	bool	buildSubPoly(int edge_start, Polygon2D* poly);
	bool	doSplitting(PolygonGroup2D* poly);

	void	openSector(WDEdMapEditor::Sector* sector);
};

#endif//__POLYGON_2D_H__
