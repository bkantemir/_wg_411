#include "Gangway.h"
#include "Coupler.h"
#include "RollingStock.h"
#include "TheApp.h"

extern TheApp theApp;
extern float degrees2radians;

float Gangway::yLevel1 = 2;
float Gangway::yLevel2 = 8;



int Gangway::onLoadGangway(Gangway* pGw, std::string tagStr) {
	//tag instructions
	if (tagStr.length() == 0)
		return 0;

	if (XMLparser::varExists("level2", tagStr))
		pGw->level = 2;

	if (pGw->level == 2)
		pGw->ownCoords.pos[1] = yLevel2;
	else
		pGw->ownCoords.pos[1] = yLevel1;
	return 1;
}

int Gangway::checkGangway(Gangway* pGw) {
	SceneSubj* pRoot = pGw->pSubjsSet->at(pGw->rootN);
	if (strstr(pRoot->className, "RollingStock") != pRoot->className)
		return 0;
	RollingStock* pWagon = (RollingStock*)pRoot;
	Coupler* pCp = pWagon->pCouplerFront;
	if(pGw->alignedWithRoot<0)
		pCp = pWagon->pCouplerBack;
	pGw->pCounterGangway = NULL;
	if (pCp == NULL)
		return 0;
	if (pCp->connected < 1)
		return 0;
	if (pCp->couplersInvolved < 2)
		return 0;
	//connected wagon
	Coupler* pCp2 = pCp->pCounterCoupler;
	RollingStock* pWagon2 = (RollingStock*)pGw->pSubjsSet->at(pCp2->rootN);
	//corresponding gangway
	Gangway* pGw2= pWagon2->pGangwayFront;
	if(pCp2->alignedWithRoot<0)
		pGw2 = pWagon2->pGangwayBack;
	if (pGw2 == NULL)
		return 0;
	if (pGw->level != pGw2->level)
		return 0;
	pGw->pCounterGangway = pGw2;
	pGw2->pCounterGangway = pGw;
	return 1;
}


int Gangway::renderGangway(Gangway* pGw, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap) {
	if (pGw->pCounterGangway==NULL) {
		//render non-connected
		renderStandard(pGw, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);
		return 1;
	}
	//if here - have connected gangways pair
	if (pGw->nInSubjsSet < pGw->pCounterGangway->nInSubjsSet)
		return 0;//will render later, with counterGangway
	//here - render couplers pair
	Gangway* pGw2 = pGw->pCounterGangway;
	float distBetween = v3lengthFromToXZ(pGw->absCoords.pos, pGw2->absCoords.pos);
	float pos[4];
	for (int i = 0; i < 3; i++)
		pos[i] = pGw->absCoords.pos[i] + (pGw2->absCoords.pos[i] - pGw->absCoords.pos[i]) * 0.5;

	float yaw = v3yawDgFromTo(pGw->absCoords.pos, pGw2->absCoords.pos);
	mat4x4 posModelMatrix;
	mat4x4_translate(posModelMatrix, pos[0], pos[1], pos[2]);
	mat4x4 absModelMatrix;
	mat4x4_rotate_Y(absModelMatrix, posModelMatrix, yaw * degrees2radians);


	SceneSubj* pModel = Rail::railModels.at(Rail::gangwaysCoupleModelN);
	m16copy((float*)pModel->absModelMatrixUnscaled, (float*)absModelMatrix);
	mat4x4_scale_aniso(pModel->absModelMatrix, pModel->absModelMatrixUnscaled, 1, 1, distBetween);
	renderStandard(pModel, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);
	for (int mN = 1; mN < pModel->totalElements; mN++) {
		SceneSubj* pSS = pModel->pSubjsSet->at(pModel->nInSubjsSet+mN);
		mat4x4_translate(pSS->ownModelMatrixUnscaled, pSS->ownCoords.pos[0], pSS->ownCoords.pos[1], pSS->ownCoords.pos[2] * distBetween);
		mat4x4_mul(pSS->ownModelMatrixUnscaled, pSS->ownModelMatrixUnscaled, pSS->ownCoords.rotationMatrix);
		mat4x4_mul(pSS->absModelMatrixUnscaled, pModel->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled);
		memcpy(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, sizeof(mat4x4));
		Coords::getPositionFromMatrix(pSS->absCoords.pos, pSS->absModelMatrixUnscaled);
		renderStandard(pSS, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);
	}
	return 1;
}


