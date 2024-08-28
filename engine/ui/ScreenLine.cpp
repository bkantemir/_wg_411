#include "ScreenLine.h"
#include "TheApp.h"

extern TheApp theApp;


int ScreenLine::drawScreenLine(ScreenLine* pUI) {
    //transfer data
    DrawJob* pDJ = pUI->pDrawJobs->at(pUI->djStartN);

    //mylog_v3("\np0 ", pUI->point[1]);

    unsigned int vertex_buffer = pDJ->aPos.glVBOid;
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pUI->point), pUI->point, GL_DYNAMIC_DRAW);

    memcpy(pUI->transformMatrix, mOrthoViewProjection, sizeof(mat4x4));

    //render subject
    Material* pMt = &pUI->mt0;

    executeDJbasic(pDJ, (float*)pUI->transformMatrix, pMt);

    if (pUI->deleteOnDraw) {
        pUI->pSubjs->at(pUI->nInSubjs) = NULL;
        delete pUI;
    }
    return 1;
}
int ScreenLine::addLine2queue(float* p0, float* p1, unsigned int RGBA32, float lineWidth, bool deleteOnDraw) {
    ScreenLine* pS = new ScreenLine(NULL, NULL);
    //data
    pS->djStartN = djNline;
    pS->djTotalN = 1;
    v2copy(pS->point[0], p0);
    v2copy(pS->point[1], p1);

    {//coords correction
        for (int pN = 0; pN < 2; pN++) {
            float* pP = pS->point[pN];
            pP[0] = pP[0] - screenSize[0] / 2;
            pP[1] = -pP[1] + screenSize[1] / 2;
        }
    }
    //other parameters
    Material* pMt = &pS->mt0;
    memcpy(pMt, &pS->pDrawJobs->at(pS->djStartN)->mt, sizeof(Material));
    pMt->uColor.setUint32(RGBA32);
    if (pMt->uColor.forGL()[3]<1.f)
        pMt->uAlphaBlending = 1;
    pMt->lineWidth = lineWidth;
    pS->deleteOnDraw = deleteOnDraw;
    return 1;
}
