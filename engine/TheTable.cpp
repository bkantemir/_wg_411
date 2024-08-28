#include "TheTable.h"
#include "ModelBuilder.h"
#include "Texture.h"
#include "TheApp.h"
#include "geom/Line3D.h"
#include "ButtonTableDrag.h"
#include "utils.h"
#include "Shadows.h"

extern TheApp theApp;
extern std::string filesRoot;


TheTable::~TheTable() {
	cleanUpTable();
}



void TheTable::cleanUpTable() {
	for (int i = tableParts.size() - 1; i >= 0; i--)
		delete tableParts.at(i);
	tableParts.clear();

	for (int i = table_drawJobs.size() - 1; i >= 0; i--)
		delete table_drawJobs.at(i);
	table_drawJobs.clear();

	for (int i = table_buffersIds.size() - 1; i >= 0; i--) {
		unsigned int id = table_buffersIds.at(i);
		glDeleteBuffers(1, &id);
	}
	table_buffersIds.clear();
}
int TheTable::initTable(float tileSizeXZ, float tileSizeUp, float tileSizeDown, int tilesNx, int tilesNz) {
	tileSize = tileSizeXZ;
	tableTiles[0] = tilesNx;
	tableTiles[1] = tilesNz;
	groundLevel = tileSizeDown;

	suggestedStageSize = tileSize * fmin(5, tilesNx)*1.41;

	//define worldBox 
	worldBox.bbMin[1] = 0;
	worldBox.bbMax[1] = tileSizeUp + tileSizeDown;
	worldBox.bbMax[0] = tileSizeXZ * 0.5 * tilesNx;
	worldBox.bbMax[2] = tileSizeXZ * 0.5 * tilesNz;
	worldBox.bbMin[0] = -worldBox.bbMax[0];
	worldBox.bbMin[2] = -worldBox.bbMax[2];
	Gabarites::adjustMidRad(&worldBox);

	//table parts (tiles)
	ModelBuilder* pMB = new ModelBuilder();
	Material mt;
	strcpy_s(mt.shaderType32, 32, "phong");
	strcpy_s(mt.materialName32, 32, "table");
	mt.dropsShadow = 0;
	mt.uSpecularIntencity = 0;
	//mt.uColor.setRGBA(255, 0, 0);
	//mt.uTex0 = Texture::loadTexture(filesRoot+"/dt/mt/wood01.jpg");
	VirtualShape vs;
	v3set(vs.whl, tileSize, groundLevel, tileSize);
	float y0 = groundLevel / 2;
	//table top
	pMB->lockGroup(pMB);
	for (int zN = 0; zN < tableTiles[1]; zN++)
		for (int xN = 0; xN < tableTiles[0]; xN++) {
			SceneSubj* pSS = new SceneSubj();
			tableParts.push_back(pSS);
			pSS->pDrawJobs = &table_drawJobs;
			pSS->pSubjsSet = &tableParts;
			pSS->nInSubjsSet = tableParts.size() - 1;
			pSS->buildModelMatrix();
			pSS->gabaritesWorld.chordType = -1;
			pSS->gabaritesOnScreen.chordType = -1;
			pMB->useSubjN(pMB, pSS->nInSubjsSet);
			/*
			if ((zN + xN) % 2 == 0)
				mt.uColor.setRGBA(255, 0, 0);
			else
				mt.uColor.setRGBA(0, 0, 255);
			//mt.uColor.setRGBA(0.5f, 0.5f, 0.5f, 1.0f);
			*/
			if ((zN + xN) % 2 == 0)
				mt.uColor.setRGBA(0.4f, 0.4f, 0.4f);
			else
				mt.uColor.setRGBA(0.3f, 0.3f, 0.3f);

			pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);

			float z0 = tileSize * zN - worldBox.bbRad[2] + tileSize / 2;
			float x0 = tileSize * xN - worldBox.bbRad[0] + tileSize / 2;

			pMB->lockGroup(pMB);
			pMB->buildBoxFace(pMB, "top", &vs);
			pMB->moveGroupDg(pMB, 0, 0, 0, x0, y0, z0);
			pMB->releaseGroup(pMB);
		}
	/*
	TexCoords tc;
	tc.set_GL(&tc, 0,0,10,10,"");
	pMB->groupApplyTexture(pMB, "top", &tc);//, TexCoords * pTC = NULL, TexCoords * pTC2nm = NULL);
	*/
	pMB->releaseGroup(pMB);
	pMB->buildDrawJobs(pMB, &tableParts, &table_drawJobs, &table_buffersIds);
	delete pMB;
	return 1;
}
void TheTable::placeAt(float* pos, float x, float y, float z) {
	pos[0] = x * tileSize - worldBox.bbRad[0];
	pos[1] = y;
	pos[2] = z * tileSize - worldBox.bbRad[2];
}
int TheTable::onLeftButtonDown() {
	TouchScreen::capturedCode.assign(ButtonTableDrag::defaultMode64);
    getCursorAncorPointTable(TouchScreen::ancorPoint, TouchScreen::lastCursorPos, this);
	return 1;
};

int TheTable::getCursorAncorPointTable(float* ancorPoint, float* cursorPos, TheTable* pTable) {
	v3setAll(ancorPoint, 0);
    float* targetRads = theApp.mainCamera.targetRads;
    //cursor position in GL range
    float cursorGLpos[4];
    cursorGLpos[0] = (TouchScreen::lastCursorPos[0] - targetRads[0]) / targetRads[0];
    cursorGLpos[1] = -(TouchScreen::lastCursorPos[1] - targetRads[1]) / targetRads[1];
    cursorGLpos[2] = 1;
    //cursor position in world coords
    mat4x4 mCamInverted;
    mat4x4_invert(mCamInverted, theApp.mainCamera.mViewProjection);
    float cursor3Dpos[4];
    mat4x4_mul_vec4plus(cursor3Dpos, mCamInverted, cursorGLpos, 1, true);
    Line3D* pL3D = new Line3D(theApp.mainCamera.ownCoords.pos, cursor3Dpos);
    float y0 = pTable->groundLevel;
    if (Line3D::crossPlane(ancorPoint, pL3D, 1, y0) == 0) {
        mylog("ERROR in TheTable::getCursorAncorPointTable - no crossing\n");
        return -1;
    }
    return 1;
}
int TheTable::onDrag() {
    theApp.bPause = true;
	if (v2match(TouchScreen::captureCursorPos, TouchScreen::lastCursorPos))
		return 0;
	Camera* pCam = &theApp.mainCamera;

	if (TouchScreen::capturedCode.compare("drag_table_sun") == 0) {//Light-SPIN
		float d[2];
		for (int i = 0; i < 2; i++)
			d[i] = TouchScreen::lastCursorPos[i] - TouchScreen::captureCursorPos[i];
		//d[0] *= -1;
		float* lightDir=theApp.dirToMainLight;
		float yaw0 = v3yawDg(lightDir);
		float pitch0 = v3pitchDg(lightDir);

		float yaw = angleDgNorm180(yaw0 + d[0] / UISubj::screenSize[0] * 180);
		//dragging front(closest) table rib
		float pitchD = d[1] / UISubj::screenSize[1] * 20;
		float pitch = pitch0 + pitchD;
		pitch = minimax(pitch, -80, -50);

		Coords lt;
		lt.setEulerDg(pitch, yaw, 0);
		float v1[4] = { 0,0,1,0 };
		mat4x4_mul_vec4plus(lightDir, lt.rotationMatrix, v1, 0);

		Shadows::resetShadowsFor(&theApp.mainCamera, &theApp.gameTable.worldBox);

	}
	else if (TouchScreen::capturedCode.compare("drag_table_spin") == 0) {//TABLE-SPIN
		float d[2];
		for (int i = 0; i < 2; i++)
			d[i] = TouchScreen::lastCursorPos[i] - TouchScreen::captureCursorPos[i];
		d[0] *= -1;
		float yaw = angleDgNorm180(pCam->ownCoords.getEulerDg(1) + d[0] / UISubj::screenSize[0] * 180);
		//dragging front(closest) table rib
		float pitchD = d[1] / UISubj::screenSize[1] * 45;
		float pitch = pCam->ownCoords.getEulerDg(0) + pitchD;
		pitch = minimax(pitch, 10, 70);

		pCam->ownCoords.setEulerDg(pitch, yaw, 0);
		Camera::reset(pCam, &worldBox,true);

	}
	else if (TouchScreen::capturedCode.compare("drag_table_xz") == 0) {
		float newAncor[4];
		getCursorAncorPointTable(newAncor, TouchScreen::lastCursorPos, this);
		float shift[4];
		for (int i = 0; i < 3; i++)
			shift[i] = TouchScreen::ancorPoint[i] - newAncor[i];

		//mylog("%d: %d x %d shift %d x %d\n", (int)theApp.frameN, (int)newAncor[0], (int)newAncor[2], (int)shift[0], (int)shift[2]);

		//adjust cam lookAtPoint
		pCam->lookAtPoint[0] += shift[0];
		pCam->lookAtPoint[2] += shift[2];

		Camera::reset(pCam, &worldBox,true);
		getCursorAncorPointTable(TouchScreen::ancorPoint, TouchScreen::lastCursorPos, this);
	}
	/*
	else if (TouchScreen::capturedCode.compare("drag_table_xy") == 0) {
		float shift[4] = { 0,0,0,0 };
		for (int i = 0; i < 2; i++)
			shift[i] = TouchScreen::lastCursorPos[i] - TouchScreen::captureCursorPos[i];
		
		float vAdjust[4];
		mat4x4_mul_vec4plus(vAdjust, pCam->ownCoords.rotationMatrix, shift, 1, true);
		for (int i = 0; i < 3; i++)
			pCam->lookAtPoint[i] += vAdjust[i];
		pCam->setCameraPosition(pCam);
		Line3D ln;
		Line3D::initLine3D(&ln, pCam->ownCoords.pos, pCam->lookAtPoint);
		Line3D::crossPlane(pCam->lookAtPoint, &ln, 1, theApp.gameTable.groundLevel);

		Camera::reset(pCam, &worldBox);
	}
	*/
	else {
		int a = 0;
	}
	v2copy(TouchScreen::captureCursorPos, TouchScreen::lastCursorPos);
	return 1;
}
int TheTable::onFocusOut() {
	ButtonTableDrag::dissolve = true;
	TouchScreen::capturedCode.assign("NONE"); 
	return 1; 
}

