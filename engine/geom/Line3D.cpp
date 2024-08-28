#include "Line3D.h"
#include "utils.h"

void Line3D::initLine3D(Line3D* pLn, float* p00, float* p01) {
	v3copy(pLn->p0, p00);
	v3copy(pLn->p1, p01);
	calculateLine3D(pLn);
}
void Line3D::calculateLine3D(Line3D* pLn) {
	//(x-x0)/kx=(y-y0)/ky=(z-z0)/kz
	int firstNonFlatAxisN = -1;
	for (int i = 0; i < 3; i++) {
		float d = pLn->p1[i] - pLn->p0[i];
		if (d == 0) {
			pLn->flatAxis[i] = true;
			pLn->axisConst[i] = pLn->p0[i];
		}
		else {
			pLn->flatAxis[i] = false;
			if (firstNonFlatAxisN < 0)
				firstNonFlatAxisN = i;
			if (firstNonFlatAxisN == i)
				pLn->k_slope[i] = 1;
			else
				pLn->k_slope[i] = (pLn->p1[i] - pLn->p0[i]) * pLn->k_slope[firstNonFlatAxisN] / (pLn->p1[firstNonFlatAxisN] - pLn->p0[firstNonFlatAxisN]);
				//kx  = (x - x0)* ky/(y - y0)
		}
	}
}
int Line3D::crossPlane(float* vOut, Line3D* pL3D, int planeN, float planeLevel) {
	if (pL3D->flatAxis[planeN]) {
		//line is parallel to plane
		if (pL3D->axisConst[planeN] == planeLevel) {
			//line is matching plane
			v3copy(vOut, pL3D->p0);
			return 100; //multiple intersections
		}
		else {
			//line hovers plane
			return 0; //no intersections
		}
	}
	//1 intersection
	vOut[planeN] = planeLevel;
	float C = (planeLevel - pL3D->p0[planeN]) / pL3D->k_slope[planeN];
	for (int i = 0; i < 3; i++) {
		if (i == planeN)
			continue;
		//(x-x0)/kx=C -> x=C*kx+x0
		vOut[i] = C * pL3D->k_slope[i] + pL3D->p0[i];
	}
	//mylog_v3("vOut", vOut);
	return 1;
}
bool Line3D::isPointBetween(float* p3d, float* v1, float* v2) {
	for (int i = 0; i < 3; i++)
		if (((p3d[i] - v1[i]) * (p3d[i] - v2[i])) > 0)
			return false;
	return true;

}
bool Line3D::isPointInSegment(float* p3d, Line3D* pLn) {
	return isPointBetween(p3d, pLn->p0, pLn->p1);
}
int Line3D::clipLineByBox(Line3D* pLn, Gabarites* pViewBox,bool withinSegment) {
	//returns N of fit points. If 0 - segment is out of box 
	for (int planeN = 0; planeN < 3; planeN++) {
		float p0[4];
		if (crossPlane(p0, pLn, planeN, pViewBox->bbMin[planeN]) < 1)
			continue; //line is out of plane
		float p1[4];
		if (crossPlane(p1, pLn, planeN, pViewBox->bbMax[planeN]) < 1)
			continue; //line is out of plane
		Line3D ln2;
		initLine3D(&ln2, p0, p1);

		if (!withinSegment) {
			memcpy(pLn, &ln2, sizeof(Line3D));
			withinSegment = true;
			continue;
		}

		//clipping
		bool clipped = false;
		std::vector<float*> confirmedPoints;
		if (isPointInSegment(pLn->p0, &ln2))
			confirmedPoints.push_back(pLn->p0);
		else
			clipped = true;
		if (isPointInSegment(pLn->p1, &ln2))
			confirmedPoints.push_back(pLn->p1);
		else
			clipped = true;
		if (clipped) {
			if (isPointInSegment(ln2.p0, pLn))
				confirmedPoints.push_back(ln2.p0);
			if (isPointInSegment(ln2.p1, pLn))
				confirmedPoints.push_back(ln2.p1);
			if (confirmedPoints.size() == 0) {
				return 0;
			}
			if (confirmedPoints.size() == 1) {
				//update line
				v3copy(pLn->p0, confirmedPoints.at(0));
				v3copy(pLn->p1, confirmedPoints.at(0));
				calculateLine3D(pLn);
				return 1;
			}
			if (confirmedPoints.size() > 2) {
				//check for redundant points
				for (int i = 1; i < confirmedPoints.size(); i++) {
					if (!v3match(confirmedPoints.at(0), confirmedPoints.at(i))) {
						if (i != 1)
							v3copy(confirmedPoints.at(1), confirmedPoints.at(i));
						break;
					}
				}
			}
			//update line
			v3copy(pLn->p0, confirmedPoints.at(0));
			v3copy(pLn->p1, confirmedPoints.at(1));
			calculateLine3D(pLn);
		}
	}
	return 2;
}
