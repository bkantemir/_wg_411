#include "Shadows.h"
#include "TheApp.h"
#include "utils.h"

float Shadows::shadowLight = 0.5f;
int Shadows::depthMapTexN = -1;
Texture* Shadows::pDepthMap = NULL;
Camera Shadows::shadowCamera;
float Shadows::sizeUnitPixelsSize = 1;
float Shadows::uConstZ= 0.0005f;

extern TheApp theApp;

float Shadows::uDepthBias[16]; //z-value shifts depeding on normal
std::vector<SceneSubj*> Shadows::shadowsQueue;



int Shadows::init() {
	if (v3equals(theApp.dirToMainLight, 0)) {
		mylog("ERROR in Shadows::init(): theApp.dirToMainLight not set.\n");
		return -1;
	}

	depthMapTexN = Texture::textures.size();
	Texture* pTex = new Texture();
	Texture::textures.push_back(pTex);
	pTex->size[0] = 2048;
	pTex->size[1] = pTex->size[0];
	pTex->source.assign("depthMap");

	glGenTextures(1, (GLuint*)&pTex->GLid);
	glBindTexture(GL_TEXTURE_2D, pTex->GLid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, pTex->size[0], pTex->size[1]);
	checkGLerrors("Depth tex");

	glBindTexture(GL_TEXTURE_2D, 0);
	/////////////////

	Texture::attachRenderBuffer(depthMapTexN, true);
	pDepthMap = Texture::textures.at(depthMapTexN);

	float* dirToLight = theApp.dirToMainLight;
	//set up camera
	Camera* pCam = &shadowCamera;
	pCam->viewRangeDg = 0;
	pCam->focusDistance = 100; //temporary
	pCam->targetDims[0] = pDepthMap->size[0];
	pCam->targetDims[1] = pDepthMap->size[1];
	pCam->targetRads[0] = pCam->targetDims[0]/2;
	pCam->targetRads[1] = pCam->targetDims[1]/2;
	pCam->targetAspectRatio = pCam->targetDims[0] / pCam->targetDims[1];

	float vDir[4];
	for (int i = 0; i < 3; i++) {
		pCam->ownCoords.pos[i] = theApp.dirToMainLight[i] * pCam->focusDistance;
		vDir[i] = -pCam->ownCoords.pos[i];
	}
	//build pCam->ownCoords.eulerDg[]
	pCam->ownCoords.setEulerDg(v3pitchDg(vDir), v3yawDg(vDir), 0);
	//build pCam->ownCoords.rotationMatrix
	//mat4x4_from_quat(pCam->ownCoords.rotationMatrix, pCam->ownCoords.getRotationQuat());
	pCam->buildLookAtMatrix(pCam);

	//calculate uBias (for depth map)
	int totalN = 16;
	float normalStep = 1.0f / (totalN + 1);
	for (int i = 0; i < totalN; i++) {
		float normalZ = normalStep * (i + 1);
		float bias = sqrtf(1.0f - normalZ * normalZ) / normalZ;
		uDepthBias[i] = bias / (float)pDepthMap->size[0] + 0.0005f;// *1.3f + 0.0005f;
	}
	return 1;
}
int Shadows::renderDepthMap() {
	Texture::setRenderToTexture(depthMapTexN);
	//Texture* pT = Texture::textures.at(depthMapTexN);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::vector<SceneSubj*>* pSubjs = &shadowsQueue;
	//RenderScene
	int subjsN = pSubjs->size();
	for (int subjN = 0; subjN < subjsN; subjN++) {
		SceneSubj* pSS = pSubjs->at(subjN);
		pSS->render(&shadowCamera, NULL, NULL, NULL,true);
			checkGLerrors("pGS->renderDepthMap");
	}
	shadowsQueue.clear();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return 1;
}
int Shadows::addToShadowsQueue(std::vector<std::vector<SceneSubj*>*> pSubjArrays2draw) {
	int arraysN = pSubjArrays2draw.size();
	int added = 0;
	for (int arrayN = 0; arrayN < arraysN; arrayN++) {
		std::vector<SceneSubj*>* pSubjs = pSubjArrays2draw.at(arrayN);
		int subjsN = pSubjs->size();
		for (int sN = 0; sN < subjsN; sN++) {
			SceneSubj* pSS = pSubjs->at(sN);
			if (pSS == NULL)
				continue;
			if (pSS->hidden() > 0)
				continue;
			if (pSS->djTotalN < 1)
				continue;
			if (pSS->dropsShadow == 0)
				continue;
			shadowsQueue.push_back(pSS);
			added++;
		}
	}
	return added;
}

void Shadows::resetShadowsFor(Camera* pCam4, Gabarites* pWorldBox) {
	Camera* pCam = &shadowCamera;
	//reset angle
	float vDir[4];
	for (int i = 0; i < 3; i++) {
		pCam->ownCoords.pos[i] = theApp.dirToMainLight[i] * pCam->focusDistance;
		vDir[i] = -pCam->ownCoords.pos[i];
	}
	pCam->ownCoords.setEulerDg(v3pitchDg(vDir), v3yawDg(vDir), 0);

	pCam->focusDistance = pWorldBox->boxRad * 2;
	//reset lookAtPoint according to visibleBox
	v3copy(pCam->lookAtPoint, pCam4->visibleBox.bbMid);
	//reset camera position
	Camera::setCameraPosition(pCam);
	Camera::buildLookAtMatrix(pCam);
	mat4x4 mInverted;
	mat4x4_invert(mInverted, pCam->lookAtMatrix);

	pCam->visibleBox.clear();
	pCam->visibleRays.clear();
	pCam->nearClip = 1000000;
	pCam->farClip = -1000000;
	Gabarites gb;
	for (int rayN = pCam4->visibleRays.size() - 1; rayN >= 0; rayN--) {
		float p0[4];
		float p1[4];
		Line3D* pSrcRay = pCam4->visibleRays.at(rayN);
		for (int pointN = 0; pointN < 2; pointN++) {
			if (pointN == 0)
				v3copy(p0, pSrcRay->p0);
			else
				v3copy(p0, pSrcRay->p1);
			for (int i = 0; i < 3; i++)
				p1[i] = p0[i] + theApp.dirToMainLight[i] * 1000;
			Line3D shadowRay;
			Line3D::initLine3D(&shadowRay, p0, p1);
			if (Line3D::clipLineByBox(&shadowRay, pWorldBox, false) < 2)
				continue;
			for (int pointN2 = 0; pointN2 < 2; pointN2++) {
				float p2[4];
				if (pointN2 == 0)
					v3copy(p2, shadowRay.p0);
				else
					v3copy(p2, shadowRay.p1);
				float localCoords[4];
				mat4x4_mul_vec4plus(localCoords, pCam->lookAtMatrix, p2, 1, false);
				gb.adjustMinMaxByPoint(&gb, localCoords);
			}
		}
	}
	pCam->farClip = -gb.bbMin[2];
	pCam->nearClip = -gb.bbMax[2];
	if (pCam->nearClip < 1)
		pCam->nearClip = 1;
	//re-center lookAtPoint
	gb.adjustMidRad(&gb);
	mat4x4_mul_vec4plus(pCam->lookAtPoint, mInverted, gb.bbMid, 1, false);
	Camera::setCameraPosition(pCam);
	float halfStageSize = fmax(gb.bbRad[0], gb.bbRad[1]);
	pCam->stageSize[0] = halfStageSize*2;
	pCam->stageSize[1] = pCam->stageSize[0];

	Camera::buildLookAtMatrix(pCam);
	mat4x4 mProjection;
	mat4x4_ortho(mProjection, -halfStageSize, halfStageSize, -halfStageSize, halfStageSize, pCam->nearClip, pCam->farClip);
	mat4x4_mul(pCam->mViewProjection, mProjection, pCam->lookAtMatrix);
}
