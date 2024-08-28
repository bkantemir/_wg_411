#pragma once
#include "Gabarites.h"
#include "geom/MyPolygon.h"

//(x-x0)/kx=(y-y0)/ky=(z-z0)/kz
class Line3D
{
public:
	float p0[4] = { 0,0,0,0 };
	float p1[4] = { 0,0,0,0 };
	float k_slope[3] = { 0,0,0 };
	bool flatAxis[3] = { 0,0,0 };
	float axisConst[3] = { 0,0,0 };
public:
	Line3D(void) {};
	Line3D(float* p00, float* p01) { initLine3D(this, p00, p01); };
	static void initLine3D(Line3D* pLn, float* p00, float* p01);
	void calculateLine3D() { calculateLine3D(this); };
	static void calculateLine3D(Line3D* pLn);
	static int crossPlane(float* vOut, Line3D* pL3D, int planeN,float planeLevel);
	static bool isPointBetween(float* p3d, float* v1, float* v2);
	static bool isPointInSegment(float* p3d, Line3D* pLn);
	static int clipLineByBox(Line3D* pLn, Gabarites* pViewBox, bool withinSegment);
};
