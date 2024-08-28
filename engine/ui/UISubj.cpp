#include "UISubj.h"
#include "TheApp.h"
#include "Shader.h"
#include "Texture.h"

int UISubj::djNtex = -1;
int UISubj::djNclr = -1;
int UISubj::djNdepthmap = -1;
int UISubj::djNline = -1;
int UISubj::djNframe = -1;
int UISubj::djNround = -1;
mat4x4 UISubj::mOrthoViewProjection;

UISubj* UISubj::pSelectedUISubj =NULL;

float UISubj::screenSize[2];
float UISubj::screenAspectRatio;

extern TheApp theApp;
extern float degrees2radians;

UISubj::UISubj(std::string name0, std::vector<UISubj*>* pSubjs, std::vector<DrawJob*>* pDJs) {
    if (pSubjs == NULL)
        pSubjs = &theApp.UISubjs;
    if (pDJs == NULL)
        pDJs = &theApp.drawJobs;
    UISubj* pS = this;
    strcpy_s(pS->className, 32, "UISubj");
    if (name0.empty()) {
        int a = 0;
    }
    strcpy_s(pS->name64, 64, name0.c_str());

    pS->pDrawJobs = pDJs;
    pS->pSubjs = pSubjs;
    //place at first empty slot
    int slotN = findUIslot(pSubjs);
    pSubjs->at(slotN) = pS;
    pS->pSubjs = pSubjs;
    pS->nInSubjs = slotN;
}
UISubj::~UISubj() {
    //UISubj* pS = this;
    //pS->pSubjs->erase(pS->pSubjs->begin() + pS->nInSubjs);
}

int UISubj::init() {
    std::vector<DrawJob*>* pDrawJobs = &theApp.drawJobs;
    std::vector<unsigned int>* pBuffersIds = &theApp.buffersIds;

    AttribRef aPos;
    AttribRef aTuv;

    //djTexture
    float dt4x5[4][5] =
    {
        { -0.5f,  0.5f, 0.f, 0.f, 0.f }, //top-left
        { -0.5f, -0.5f, 0.f, 0.f, 1.f }, //bottom-left
        {  0.5f,  0.5f, 0.f, 1.f, 0.f }, //top-right
        {  0.5f, -0.5f, 0.f, 1.f, 1.f }  //bottom-right
    };
    djNtex = pDrawJobs->size();
    DrawJob* pDJ = new DrawJob(pDrawJobs);
    pDJ->pointsN = 4;
    int vertex_buffer = DrawJob::newBufferId(pBuffersIds);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dt4x5), dt4x5, GL_STATIC_DRAW);
    //VBO is ready
    pDJ->setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 5);
    pDJ->setAttribRef(&pDJ->aTuv, vertex_buffer, sizeof(float) * 3, sizeof(float) * 5);

    Material* pMT = &pDJ->mt;
    pMT->primitiveType = GL_TRIANGLE_STRIP;
    pMT->uTex0 = 22;
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    Shader::assignShader(pMT, "flat");
    //pMT->shaderN = Shader::progN_flat_uTex;

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);

    //djClr
    float dt4x3[4][3] =
    {
        { -0.5f,  0.5f, 0.f, }, //top-left
        { -0.5f, -0.5f, 0.f, }, //bottom-left
        {  0.5f,  0.5f, 0.f, }, //top-right
        {  0.5f, -0.5f, 0.f, }  //bottom-right
    };
    djNclr = pDrawJobs->size();
    pDJ = new DrawJob(pDrawJobs);
    pDJ->pointsN = 4;
    //unsigned int vertex_buffer;
    vertex_buffer = DrawJob::newBufferId(pBuffersIds);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dt4x3), dt4x3, GL_STATIC_DRAW);
    //VBO is ready
    DrawJob::setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 3);

    pMT = &pDJ->mt;
    pMT->primitiveType = GL_TRIANGLE_STRIP;
    pMT->uColor.setRGBA(255, 255, 0, 255);
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    Shader::assignShader(pMT, "flat");
    //pMT->shaderN = Shader::progN_flat_uClr;

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);


    //djDepthmap

    Shader::buildShaderObjectFromFiles(NULL, "z-buffer", "/dt/shaders/flat_utex_v.txt", "/dt/shaders/flat_depthmap_f.txt");

    float vertsDM[4][5] =
    {   //image is upside down
        { -0.5f,  0.5f, 0.f, 0.f, 1.f }, //top-left
        { -0.5f, -0.5f, 0.f, 0.f, 0.f }, //bottom-left
        {  0.5f,  0.5f, 0.f, 1.f, 1.f }, //top-right
        {  0.5f, -0.5f, 0.f, 1.f, 0.f }  //bottom-right
    };
    djNdepthmap = pDrawJobs->size();
    pDJ = new DrawJob(pDrawJobs);
    pDJ->pointsN = 4;
    vertex_buffer = DrawJob::newBufferId(pBuffersIds);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertsDM), vertsDM, GL_STATIC_DRAW);
    //VBO is ready
    pDJ->setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 5);
    pDJ->setAttribRef(&pDJ->aTuv, vertex_buffer, sizeof(float) * 3, sizeof(float) * 5);

    pMT = &pDJ->mt;
    pMT->primitiveType = GL_TRIANGLE_STRIP;
    pMT->uTex0 = 22;
    pMT->takesShadow = 0;
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    Shader::assignShader(pMT, "z-buffer");
    //pMT->shaderN = Shader::progN_flat_depthmap;

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);

    //2-point line
    float dt2x3[2][3] =
    {
        { 0.f,  0.f, 0.f }, //top-left
        { 100.f, 100.f, 0.f }
    };
    djNline = pDrawJobs->size();
    pDJ = new DrawJob(pDrawJobs);
    pDJ->pointsN = 2;
    //unsigned int vertex_buffer;
    vertex_buffer = DrawJob::newBufferId(pBuffersIds);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dt2x3), dt2x3, GL_DYNAMIC_DRAW);
    //VBO is ready
    DrawJob::setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 3);

    pMT = &pDJ->mt;
    pMT->primitiveType = GL_LINES;
    pMT->uColor.setRGBA(0, 255, 0, 255);
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    Shader::assignShader(pMT, "flat");
    //pMT->shaderN = Shader::progN_flat_uClr;

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);



    //djFrame
    float dt4x3frame[4][3] =
    {
        { -0.5f,  0.5f, 0.f, }, //top-left
        { -0.5f, -0.5f, 0.f, }, //bottom-left
        {  0.5f, -0.5f, 0.f, },  //bottom-right
        {  0.5f,  0.5f, 0.f, } //top-right
    };
    djNframe = pDrawJobs->size();
    pDJ = new DrawJob(pDrawJobs);
    pDJ->pointsN = 4;
    //unsigned int vertex_buffer;
    vertex_buffer = DrawJob::newBufferId(pBuffersIds);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dt4x3frame), dt4x3frame, GL_STATIC_DRAW);
    //VBO is ready
    DrawJob::setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 3);

    pMT = &pDJ->mt;
    pMT->primitiveType = GL_LINE_LOOP;
    pMT->uColor.setRGBA(255, 255, 0, 255);
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    Shader::assignShader(pMT, "flat");
    //pMT->shaderN = Shader::progN_flat_uClr;

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);

    //djRound
    const int radSectN = 18;
    float roundPoints[radSectN][5];
    float angStep = 360.0 / radSectN;
    for (int pointN = 0; pointN < radSectN; pointN++) {
        float ang = angStep * pointN;
        float* pPoint = roundPoints[pointN];
        pPoint[0] = cos(ang * degrees2radians)*0.5;
        pPoint[1] = sin(ang * degrees2radians) * 0.5;
        pPoint[2] = 0;
        //tuv
        pPoint[3] = pPoint[0]+0.5;
        pPoint[4] = -pPoint[1] + 0.5;
    }
    djNround = pDrawJobs->size();
    pDJ = new DrawJob(pDrawJobs);
    pDJ->pointsN = radSectN;
    //unsigned int vertex_buffer;
    vertex_buffer = DrawJob::newBufferId(pBuffersIds);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roundPoints), roundPoints, GL_STATIC_DRAW);
    //VBO is ready
    pDJ->setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 5);
    pDJ->setAttribRef(&pDJ->aTuv, vertex_buffer, sizeof(float) * 3, sizeof(float) * 5);

    pMT = &pDJ->mt;
    pMT->primitiveType = GL_TRIANGLE_FAN;
    pMT->uTex0 = 22;
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    Shader::assignShader(pMT, "flat");

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);

    return 1;
}

void UISubj::buildModelMatrix(UISubj* pUI) {
    if (pUI->transformMatrixIsReady)
        return;
    memcpy(&pUI->absCoords, &pUI->ownCoords, sizeof(Coords2D));
    if (strstr(pUI->countFrom, "left") != NULL)
        pUI->absCoords.pos[0] = pUI->ownCoords.pos[0] - screenSize[0] / 2 + pUI->scale[0] / 2;
    else if (strstr(pUI->countFrom, "right") != NULL)
        pUI->absCoords.pos[0] = -(pUI->ownCoords.pos[0] - screenSize[0] / 2 + pUI->scale[0] / 2);

    if (strstr(pUI->countFrom, "top") != NULL)
        pUI->absCoords.pos[1] = pUI->ownCoords.pos[1] - screenSize[1] / 2 + pUI->scale[1] / 2;
    else if (strstr(pUI->countFrom, "bottom") != NULL)
        pUI->absCoords.pos[1] = -(pUI->ownCoords.pos[1] - screenSize[1] / 2 + pUI->scale[1] / 2);

    //screen y - from bottom up
    pUI->absCoords.pos[1] = -pUI->absCoords.pos[1];

    //count parent here
    //...
    mat4x4 m, mr;
    mat4x4_translate(m, pUI->absCoords.pos[0], pUI->absCoords.pos[1], 0);
    mat4x4_rotate_Z(mr, m, pUI->absCoords.aZdg * degrees2radians);
    mat4x4_scale_aniso(m, mr, pUI->scale[0], pUI->scale[1], 1);

    mat4x4_mul(pUI->transformMatrix, mOrthoViewProjection, m);
    pUI->transformMatrixIsReady = true;
}
int UISubj::renderStandard(UISubj* pUI) {
    if (pUI->mt0.uAlphaFactor == 0)
        return 0;
    std::vector<DrawJob*>* pDrawJobs = &theApp.drawJobs;

    buildModelMatrix(pUI);


    //render subject
    for (int i = 0; i < pUI->djTotalN; i++) {
        DrawJob* pDJ = pDrawJobs->at(pUI->djStartN + i);
        Material* pMt = &pDJ->mt;
        if (i == 0)
            pMt = &pUI->mt0;

        executeDJbasic(pDJ, (float*)pUI->transformMatrix, pMt);
    }
    if (pUI->deleteOnDraw) {
        pUI->pSubjs->at(pUI->nInSubjs) = NULL;
        delete pUI;
    }
    return 1;
}
int UISubj::executeDJbasic(DrawJob* pDJ, float* uMVP, Material* pMt) {

    if (pMt == NULL)
        pMt = &(pDJ->mt);

    int shaderN = pMt->shaderN;
    if (shaderN < 0)
        return 0;

    glLineWidth(pMt->lineWidth);

    glBindVertexArray(pDJ->glVAOid);

    Shader* pShader = Shader::shaders.at(shaderN);
    glUseProgram(pShader->GLid);

    //input uniforms
    glUniformMatrix4fv(pShader->l_uMVP, 1, GL_FALSE, (const GLfloat*)uMVP);

    //attach textures
    if (pShader->l_uTex0 >= 0) {
        int textureId = Texture::getGLid(pMt->uTex0);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 0.    
        glUniform1i(pShader->l_uTex0, 0);
    }
    if (pShader->l_uTex1mask >= 0) {
        int textureId = Texture::getGLid(pMt->uTex1mask);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE1); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 1.    
        glUniform1i(pShader->l_uTex1mask, 1);
    }

    //material uniforms
    if (pShader->l_uColor >= 0)
        glUniform4fv(pShader->l_uColor, 1, pMt->uColor.forGL());
    if (pShader->l_uTex1alphaChannelN >= 0)
        glUniform1i(pShader->l_uTex1alphaChannelN, pMt->uTex1alphaChannelN);
    if (pShader->l_uTex1alphaNegative >= 0)
        glUniform1i(pShader->l_uTex1alphaNegative, pMt->uTex1alphaNegative);
    if (pShader->l_uAlphaFactor >= 0)
        glUniform1f(pShader->l_uAlphaFactor, pMt->uAlphaFactor);
    if (pShader->l_uAlphaBlending >= 0)
        glUniform1i(pShader->l_uAlphaBlending, pMt->uAlphaBlending);

    if (pShader->l_uHighLightLevel >= 0)
        glUniform1f(pShader->l_uHighLightLevel, 0);// uFogLevel);

    //adjust render settings
    if (pMt->zBuffer > 0) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }
    else
        glDisable(GL_DEPTH_TEST);

    if (pMt->zBufferUpdate > 0)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);

    if (pShader->l_uAlphaBlending >= 0 && pMt->uAlphaBlending > 0 || pMt->uAlphaFactor < 1) {
        glDepthMask(GL_FALSE); //don't update z-buffer
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);

    //execute
    if (pDJ->glEBOid > 0)
        glDrawElements(pMt->primitiveType, pDJ->pointsN, GL_UNSIGNED_SHORT, 0);
    else
        glDrawArrays(pMt->primitiveType, 0, pDJ->pointsN);

    glBindVertexArray(0);


    //checkGLerrors("DrawJob::executeDrawJob end");

    return 1;
}

int UISubj::addZBufferSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, int uTex0) {
    //std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    //std::vector<DrawJob*>* pDrawJobs = &theApp.drawJobs;

    UISubj* pUI = new UISubj(NULL,NULL);
    strcpy_s(pUI->name64,64, uiName.c_str());
    
    std::vector<DrawJob*>* pDrawJobs = pUI->pDrawJobs;

    setCoords(pUI, x, y, w, h, alignTo);

    pUI->djStartN = djNdepthmap;
    memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uTex0 = uTex0;
    return pUI->nInSubjs;
}
int UISubj::addTexSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, int uTex0) {
    //std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    //std::vector<DrawJob*>* pDrawJobs = &theApp.drawJobs;

    UISubj* pUI = new UISubj(NULL,NULL);
    strcpy_s(pUI->name64,64, uiName.c_str());
    
    std::vector<DrawJob*>* pDrawJobs = pUI->pDrawJobs;

    setCoords(pUI, x, y, w, h, alignTo);

    pUI->djStartN = djNtex;
    memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uTex0 = uTex0;
    return pUI->nInSubjs;
}
int UISubj::addClrSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, unsigned int rgba) {
    //std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    //std::vector<DrawJob*>* pDrawJobs = &theApp.drawJobs;

    UISubj* pUI = new UISubj(NULL, NULL);
    strcpy_s(pUI->name64,64, uiName.c_str());

    std::vector<DrawJob*>* pDrawJobs = pUI->pDrawJobs;

    setCoords(pUI, x, y, w, h, alignTo);

    pUI->djStartN = djNclr;
    memcpy(&pUI->mt0, &pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uColor.setUint32(rgba);
    if (pUI->mt0.uColor.forGL()[3] < 1.f)
        pUI->mt0.uAlphaBlending = 1;

    return pUI->nInSubjs;
}
int UISubj::addFrameSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, unsigned int rgba,float lineW) {
    //std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    //std::vector<DrawJob*>* pDrawJobs = &theApp.drawJobs;

    UISubj* pUI = new UISubj(NULL, NULL);
    strcpy_s(pUI->name64,64, uiName.c_str());

    std::vector<DrawJob*>* pDrawJobs = pUI->pDrawJobs;

    setCoords(pUI, x, y, w, h, alignTo);

    pUI->djStartN = djNframe;
    memcpy(&pUI->mt0, &pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uColor.setUint32(rgba);
    if (pUI->mt0.uColor.forGL()[3] < 1.f)
        pUI->mt0.uAlphaBlending = 1;
    pUI->mt0.lineWidth = lineW;

    return pUI->nInSubjs;
}
int UISubj::renderAll() {
    std::vector<UISubj*>* pSubjs = &theApp.UISubjs;
    int subjsN = pSubjs->size();
		//mylog("%d UI subjs\n",subjsN);
    for (int subjN = 0; subjN < subjsN; subjN++) {
        UISubj* pUI = pSubjs->at(subjN);
        if (pUI == NULL)
            continue;

        pUI->render();
    }
    return 1;
}
int UISubj::onScreenResize(int width, int height) {
    std::vector<UISubj*>* pSubjs = &theApp.UISubjs;
    screenSize[0] = (float)width;
    screenSize[1] = (float)height;
    screenAspectRatio = (float)width / (float)height;
    float w = screenSize[0] / 2;
    float h = screenSize[1] / 2;
    mat4x4_ortho(mOrthoViewProjection, -w, w, -h, h, -1, 1);
    //mat4x4_ortho(mOrthoViewProjection, 0, width, 0, height, -1, 1);

    int subjsN = pSubjs->size();
    for (int subjN = 0; subjN < subjsN; subjN++) {
        UISubj* pUI = pSubjs->at(subjN);
        if (pUI == NULL)
            continue;
        pUI->transformMatrixIsReady = false;
    }
    return 1;
}

int UISubj::clear() {
    std::vector<UISubj*>* pSubjs = &theApp.UISubjs;
    for (int subjN = pSubjs->size() - 1; subjN >= 0; subjN--) {
        UISubj* pS= pSubjs->at(subjN);
        if (pS == NULL)
            continue;
        delete pS;
        pSubjs->at(subjN) = NULL;
    }
    pSubjs->clear();
    return 1;
}
int UISubj::setCoords(UISubj* pUI, float x, float y, float w, float h, std::string countFrom) {
    strcpy_s(pUI->countFrom, 32, (char*)countFrom.c_str());
    pUI->ownCoords.pos[0] = x;
    pUI->ownCoords.pos[1] = y;
    pUI->scale[0] = w;
    pUI->scale[1] = h;
    pUI->transformMatrixIsReady = false;
    buildModelMatrix(pUI);
    return 1;
}

int UISubj::findUIslot(std::vector<UISubj*>* pSubjs,int needSlotsN) {
    int subjsN = pSubjs->size();
    for (int sN = 0; sN < subjsN; sN++) {
        UISubj* pS = pSubjs->at(sN);
        if (pS == NULL)
            return sN;
    }
    //if here - no free slots,add 1
    pSubjs->push_back(NULL);
    return subjsN;
}
int UISubj::addRoundSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, std::string src) {

    UISubj* pUI = new UISubj(NULL, NULL);
    strcpy_s(pUI->name64,64, uiName.c_str());

    std::vector<DrawJob*>* pDrawJobs = pUI->pDrawJobs;

    setCoords(pUI, x, y, w, h, alignTo);

    pUI->djStartN = djNround;
    memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));

    int uTex0 = Texture::loadTexture(src);
    //, unsigned int ckey = 0, int glRepeatH = GL_MIRRORED_REPEAT, int glRepeatV = GL_MIRRORED_REPEAT);

    pUI->mt0.uTex0 = uTex0;

    pUI->mt0.uAlphaFactor = 0.33;


    return pUI->nInSubjs;
}
UISubj* UISubj::pointerOnUI() {
    if (TouchScreen::capturedCode.find("drag_table_") == 0)
        return 0;
    float cursorPos[2];
    cursorPos[0] = TouchScreen::lastCursorPos[0] - screenSize[0] / 2;
    cursorPos[1] = -TouchScreen::lastCursorPos[1] + screenSize[1] / 2;
    std::vector<UISubj*>* pSubjs;
    UISubj* pCandidate = NULL;
    pSubjs = &theApp.UISubjs;
    int subjsN = pSubjs->size();
    for (int sN = 0; sN < subjsN; sN++) {
        UISubj* pUI = pSubjs->at(sN);
        if (pUI == NULL)
            continue;
        if (!pUI->isResponsive())
            continue;
        if (pUI->mt0.uAlphaFactor== 0)
            continue;
        //check gabarites
        bool dontFit = false;
        for (int i = 0; i < 2; i++) {
            if (cursorPos[i] < pUI->absCoords.pos[i] - pUI->scale[i]/2)
                dontFit = true;
            else if (cursorPos[i] > pUI->absCoords.pos[i] + pUI->scale[i]/2)
                dontFit = true;
        }
        if (dontFit)
            continue;

        pCandidate = pUI;

    }


    pSelectedUISubj = pCandidate;

    return pSelectedUISubj;
}
int UISubj::onLeftButtonDown() {
    TouchScreen::capturedCode.assign(name64);
    //getCursorAncorPointStandard(TouchScreen::ancorPoint, TouchScreen::lastCursorPos, TouchScreen::pSelected00);
    mt0.uAlphaFactor = 1;
    return 1;
}
int UISubj::onFocus() {
    if (!isResponsive())
        return 0;
    mt0.uAlphaFactor = 0.66;
    return 1;
}
int UISubj::onFocusOut(){
    if (!isResponsive()) 
        return 0; 
    mt0.uAlphaFactor = 0.33; 
    pSelectedUISubj = NULL;
    TouchScreen::pSelected = NULL;
    return 1;
}

