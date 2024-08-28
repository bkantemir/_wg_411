#include "Gabarites.h"
#include "utils.h"

void Gabarites::adjustMidRad(Gabarites* pGB) {
	for (int i = 0; i < 3; i++) {
		pGB->bbRad[i] = (pGB->bbMax[i] - pGB->bbMin[i]) * 0.5f;
		pGB->bbMid[i] = (pGB->bbMax[i] + pGB->bbMin[i]) * 0.5f;
	}
	pGB->boxRad = v3lengthFromTo(pGB->bbMin, pGB->bbMax) / 2;
}
void Gabarites::adjustMinMaxByPoint(Gabarites* pGB, float* newPoint) {
	for (int i = 0; i < 3; i++) {
		if (pGB->bbMin[i] > newPoint[i])
			pGB->bbMin[i] = newPoint[i];
		if (pGB->bbMax[i] < newPoint[i])
			pGB->bbMax[i] = newPoint[i];
	}
}
void Gabarites::toLog(std::string title, Gabarites* pGB) {
	mylog("%s:\n",title.c_str());
	mylog("bbMin: %s\n", v3toStr(pGB->bbMin));
	mylog("bbMax: %s\n", v3toStr(pGB->bbMax));
	mylog("bbMid: %s\n", v3toStr(pGB->bbMid));
	mylog("bbRad: %s\n", v3toStr(pGB->bbRad));
}

void Gabarites::fillBBox(Gabarites* pGBout, Gabarites* pGBin, mat4x4 mMVP, float* targetRads) {
    //calculate mid point position
    if (mat4x4_mul_vec4screen(pGBout->bbMid, mMVP, pGBin->bbMid, targetRads) < 0) {
        //point out of reach
        pGBout->isInViewRange = -1; //off-screen
        return;
    }
    //build size vectors
    for (int axisN = 0; axisN < 3; axisN++) {
        if (pGBin->bbRad[axisN] == 0) {
            v3setAll(pGBout->sizeVector[axisN], 0);
            v3setAll(pGBout->sizeDirXY[axisN], 0);
            pGBout->sizeXY[axisN] = 0;
            continue;
        }
        float vIn[4];
        v3copy(vIn, pGBin->bbMid);
        vIn[axisN] += pGBin->bbRad[axisN];
        float vOut[4];
        if(mat4x4_mul_vec4screen(vOut, mMVP, vIn, targetRads) < 0) {
            //point out of reach
            pGBout->isInViewRange = -1; //off-screen
            return;
        }

        v3fromTo(pGBout->sizeVector[axisN], pGBout->bbMid, vOut);
        v3normXY(pGBout->sizeDirXY[axisN], pGBout->sizeVector[axisN]);
        pGBout->sizeXY[axisN] = v3lengthXY(pGBout->sizeVector[axisN]);
    }
    //calculate visible bounding box
    v3setAll(pGBout->bbMin, 1000000);
    v3setAll(pGBout->bbMax, -1000000);
    for (int z = -1; z <= 1; z += 2)
        for (int y = -1; y <= 1; y += 2)
            for (int x = -1; x <= 1; x += 2)
                for (int i = 0; i < 3; i++) {
                    float xx = pGBout->bbMid[i] + pGBout->sizeVector[2][i] * z + pGBout->sizeVector[1][i] * y + pGBout->sizeVector[0][i] * x;
                    if (pGBout->bbMin[i] > xx)
                        pGBout->bbMin[i] = xx;
                    if (pGBout->bbMax[i] < xx)
                        pGBout->bbMax[i] = xx;
                }
    adjustMidRad(pGBout);

    if (targetRads == NULL)
        pGBout->isInViewRange = 0; //not a GL range
    else{ //dealing with GL range, 
        //check if within view range
        int score = 0;
        for (int i = 0; i < 3; i++) {
            float rMin = -1;
            float rMax = 1;
            if (i < 2) {
                rMin = 0;
                rMax = targetRads[i]*2;
            }
            if (pGBout->bbMin[i] > rMax) {
                score = -1;
                break;
            }
            if (pGBout->bbMax[i] < rMin) {
                score = -1;
                break;
            }
            if (pGBout->bbMin[i] > rMin)
                score++;
            if (pGBout->bbMax[i] < rMax)
                score++;
        }
        if (score < 0)
            pGBout->isInViewRange = -1; //off-screen
        else if (score == 6)
            pGBout->isInViewRange = 1; //on-screen
        else
            pGBout->isInViewRange = 0; //partially
        
        if (pGBout->bbMin[2] < -1)
            score=-1;
    }
}

int Gabarites::fillGabarites(Gabarites* pG, mat4x4 absModelMatrix, Gabarites* pG00, mat4x4 mViewProjection, float* targetRads) {
    //build MVP matrix for given subject
    mat4x4 mMVP;
    mat4x4_mul(mMVP, mViewProjection, absModelMatrix);

    fillBBox(pG, pG00, mMVP, targetRads);
    if (pG->isInViewRange < 0)
        return 0;
    if(pG->chordType<0)
        return 0; //don't need chord

     //CHORDE: select biggest axis
     int biggestAxisN = 0;
     float biggestSize = 0;
     for (int axisN = 0; axisN < 3; axisN++) {
         if (biggestSize >= pG->sizeXY[axisN])
             continue;
         biggestSize = pG->sizeXY[axisN];
         biggestAxisN = axisN;
     }
     float* pDirChord = pG->sizeDirXY[biggestAxisN];
     //perpendicular
     float pDirPerp[2];
     pDirPerp[0] = pDirChord[1];
     pDirPerp[1] = -pDirChord[0];
     float chordH = 0; //half-length
     float chordR = 0; //rad
     for (int i = 0; i < 3; i++) {
         if (i == biggestAxisN) {
             chordH += pG->sizeXY[i];
             continue;
         }
         if (pG->sizeXY[i] == 0)
             continue;
         chordH = chordH + abs(v2dotProductNorm(pDirChord, pG->sizeDirXY[i])) * pG->sizeXY[i];
         chordR = chordR + abs(v2dotProductNorm(pDirPerp, pG->sizeDirXY[i])) * pG->sizeXY[i];
     }
     pG->chordR = chordR;
     chordH -= chordR;
     
     if (chordH < 1.0f) //dot
         pG->chord.initLineXY(&pG->chord, pG->bbMid, pG->bbMid);
     else {
         float dot0[2];
         float dot1[2];
         for (int i = 0; i < 2; i++) {
             float d = pDirChord[i] * chordH;
             dot0[i] = pG->bbMid[i] + d;
             dot1[i] = pG->bbMid[i] - d;
         }
         pG->chord.initLineXY(&pG->chord, dot0, dot1);
     }
     
     return 1;
}
float Gabarites::checkCollision(float* collisionPoint, Gabarites* pGB1, Gabarites* pGB2) {
    //returns penetration depth
    //check bounding box first
    for (int i = 0; i < 3; i++){
        if (pGB1->bbMin[i] >= pGB2->bbMax[i])
            return 0;
        if (pGB1->bbMax[i] <= pGB2->bbMin[i])
            return 0;
    }

    //if here - bounding boxes overlap
    float chordsRR = pGB1->chordR + pGB2->chordR;
    float dist = LineXY::dist_l2l(&pGB1->chord, &pGB2->chord);
    float overlap = chordsRR - dist;
    if (overlap <= 0)//have collision
        return 0;
    //find collision point
    float k1 = pGB2->chordR / chordsRR;
    float k2 = 1.0f - k1;
    for (int i = 0; i < 2; i++)
        collisionPoint[i] = pGB1->chord.closestPoint[i] * k1 + pGB2->chord.closestPoint[i] * k2;
    //z component
    float zOverlapMin = std::max(pGB1->bbMin[2], pGB2->bbMin[2]);
    float zOverlapMax = std::min(pGB1->bbMax[2], pGB2->bbMax[2]);
    collisionPoint[2] = (zOverlapMin + zOverlapMax) / 2;
    return overlap;
}
