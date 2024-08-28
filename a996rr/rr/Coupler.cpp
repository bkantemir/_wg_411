#include "Coupler.h"
#include "RollingStock.h"
#include "TheApp.h"
//#include "Shadows.h"

extern TheApp theApp;
extern float degrees2radians;


float Coupler::couplingDistances[3] = { 5,6,8 };
float Coupler::couplersPairLength = 8.4;

int Coupler::onLoadCoupler(Coupler* pCp, std::string tagStr) {
	//tag instructions
	if (tagStr.length() == 0)
		return 0;

	if (XMLparser::varExists("full", tagStr))
		pCp->hook = 1;
	return 1;
}

int Coupler::checkCoupler(Coupler* pCoupler) {
	SceneSubj* pRoot = pCoupler->pSubjsSet->at(pCoupler->rootN);
	if (strstr(pRoot->className, "RollingStock") != pRoot->className)
		return 0;
	RollingStock* pWagon = (RollingStock*)pRoot;
	memcpy(&pCoupler->railCoordOld, &pCoupler->railCoord, sizeof(RailCoord));
	theApp.gameTable.shiftRailCoord(&pCoupler->railCoord, &pWagon->railCoord, &theApp.gameTable, pCoupler->zOffsetFromRoot);
	pCoupler->railCoord.alignedWithRail *= pCoupler->alignedWithRoot;

	if (pCoupler->connected == 1)
		return 0; //hard connected
	if (pCoupler->connected == 2) {
		//conditionally connected
		return 0;
	}
	//if here - not coupled, check couplers around,
	float far = theApp.gameTable.tileSize;
	Coupler* p2skip = NULL;
	//check coupler from prev frame first
	if (pCoupler->pCounterCoupler != NULL) {
		Coupler* pCp2 = pCoupler->pCounterCoupler;
		if (pCp2->pCounterCoupler != pCoupler) {
			//coupler intercepted
			pCoupler->pCounterCoupler = NULL;
		}
		else {
			//mylog("%d: checking coupler %d vs %d\n", (int)theApp.frameN, pCoupler->nInSubjsSet, pCoupler->pCounterCoupler->nInSubjsSet);
			float d = theApp.gameTable.offset2railCoord(&pCoupler->railCoord, &pCoupler->pCounterCoupler->railCoord, &theApp.gameTable, far);
			if (d < far) {//still actual
				//update distance
				pCoupler->distanceBefore = pCoupler->distance;
				pCoupler->distance = d;
			}
			else { //too far
				//release corresponding coupler too
				p2skip = pCp2;
				if (pCp2->pCounterCoupler == pCoupler) {
					pCp2->pCounterCoupler = NULL;
					pCp2->connected = -1;
				}
				pCoupler->pCounterCoupler = NULL;
				pCoupler->connected = -1;
			}
		}
	}
	if (pCoupler->pCounterCoupler == NULL) {
		//check around for candidates
		Coupler* pClosest = NULL;
		float bestDistance = 1000000;
		for (int sN = 0; sN < pCoupler->nInSubjsSet; sN++) {
			SceneSubj* pSS = pCoupler->pSubjsSet->at(sN);
			if (pSS == NULL)
				continue;
			if (strcmp(pSS->className, "coupler") != 0)
				continue;
			Coupler* pCp2 = (Coupler*)pSS;

			if (pCp2 == p2skip) {
				continue;
			}
			if (pCp2->connected > 0) {
				continue;
			}
			if (pCp2->rootN == pCoupler->rootN) {
				continue;
			}

			float d = abs(pCoupler->absCoords.pos[0] - pCp2->absCoords.pos[0]);
			if (d >= far) {
				continue; //too far
			}
			d = abs(pCoupler->absCoords.pos[2] - pCp2->absCoords.pos[2]);
			if (d >= far) {
				continue; //too far
			}
			d = theApp.gameTable.offset2railCoord(&pCoupler->railCoord, &pCp2->railCoord, &theApp.gameTable, far);

			if (d >= far) {
				continue; //wrong direction
			}
			if (bestDistance > d) {
				bestDistance = d;
				pClosest = pCp2;
			}
		}
		if (pClosest != NULL) {	//have new candidate
			pCoupler->connected = 0;//coupler ahead
			pCoupler->pCounterCoupler = pClosest;
			pCoupler->distance = bestDistance;
			pCoupler->distanceBefore = far;
		}
	}
	if (pCoupler->pCounterCoupler == NULL)
		return 0;
	//if here - have a counter coupler
	int couplersInvolved = pCoupler->hook + pCoupler->pCounterCoupler->hook;
	float couplingDistance = couplingDistances[couplersInvolved];
	couplingDistance += pCoupler->lever;
	couplingDistance += pCoupler->pCounterCoupler->lever;
	if (pCoupler->distance <= couplingDistance) {
		//if here - close enough
		RollingStock* pWagon2 = (RollingStock*)pWagon->pSubjsSet->at(pCoupler->pCounterCoupler->rootN);
		if (pWagon->trainRootN == pWagon2->trainRootN)
			return 0;//ignore - own tail
		if (couplingDistance - pCoupler->distance > 3) {
			//TOO close - keep divorce
			RollingStock::divorceTrains(pWagon,pWagon2);
			//mylog("d=%d - divorce\n", (int)pCoupler->distance);
		}
		else {
			//if here - coupling?
			if(pCoupler->distance < pCoupler->distanceBefore)
			{
				pCoupler->connected = 1;
				pCoupler->couplersInvolved = couplersInvolved;
			}
		}
	}
	//duplicate in corresponding coupler
	Coupler* pCp2 = pCoupler->pCounterCoupler;
	if (pCp2->connected == 0 && pCp2->pCounterCoupler != pCoupler && pCp2->distance > pCoupler->distance)
		pCp2->pCounterCoupler = NULL;
	if (pCoupler->connected>0 || pCp2->pCounterCoupler == pCoupler || pCp2->pCounterCoupler == NULL ) {
		pCp2->pCounterCoupler = pCoupler;
		pCp2->distance = pCoupler->distance;
		pCp2->connected = pCoupler->connected;
		pCp2->couplersInvolved = pCoupler->couplersInvolved;
	}
	
	if (pCoupler->connected > 0) {
		//RollingStock::reinspectTrain(pCp2);
		
		std::vector<SceneSubj*>* pSubjs = pCoupler->pSubjsSet;
		RollingStock* pW2 = (RollingStock*)pSubjs->at(pCp2->rootN);
		RollingStock* pRoot2 = (RollingStock*)pSubjs->at(pW2->trainRootN);
		RollingStock* pW1 = (RollingStock*)pSubjs->at(pCoupler->rootN);
		RollingStock* pRoot1 = (RollingStock*)pSubjs->at(pW1->trainRootN);
		int alignedTrains = pW1->alignedWithTrainHead*pRoot1->alignedWithTrainHead* pW2->alignedWithTrainHead * pRoot2->alignedWithTrainHead;
		if((pCoupler==pW1->pCouplerFront)== (pCp2 == pW2->pCouplerFront))
			alignedTrains *= -1;
		if (pRoot1->nInSubjsSet > pRoot2->nInSubjsSet)
			std::swap(pRoot1, pRoot2);
		float l1 = pRoot1->tr.trainLength;
		float l2 = pRoot2->tr.trainLength;
		float k1 = l1 / (l1 + l2);
		float k2 = 1.0f - k1;
		int wagonsN=RollingStock::reinspectTrain(pCp2);
		//new root
		RollingStock* pRoot = (RollingStock*)pSubjs->at(pW1->trainRootN);
		pRoot->ownZspeed = pRoot1->ownZspeed * k1 + pRoot2->ownZspeed * k2 * alignedTrains;
		
	}
		
	return 1;
}
int Coupler::renderCoupler(Coupler* pCp, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap) {
	if (pCp->djTotalN > 0)	//render coupler box
		renderStandard(pCp, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);

	if (pCp->hook == 0)
		return 1;
	//have coupler hook
	if (pCp->connected < 1 || pCp->couplersInvolved < 2) {
		//render simple single coupler
		if (pCp->lever > 0) {
			SceneSubj* pShaft = Rail::railModels.at(Rail::couplerModelN + 1);
			m16copy((float*)pShaft->absModelMatrix, (float*)pCp->absModelMatrix);
			mat4x4_scale_aniso(pShaft->absModelMatrix, pShaft->absModelMatrix,1, 1, pCp->lever);
			m16copy((float*)pShaft->absModelMatrixUnscaled, (float*)pShaft->absModelMatrix);
			renderStandard(pShaft, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);
		}
		SceneSubj* pHook = Rail::railModels.at(Rail::couplerModelN);
		mat4x4_translate(pHook->ownModelMatrixUnscaled, 0, 0, pCp->lever);
		mat4x4_mul(pHook->absModelMatrix, pCp->absModelMatrix, pHook->ownModelMatrixUnscaled);
		m16copy((float*)pHook->absModelMatrixUnscaled, (float*)pHook->absModelMatrix);

		renderStandard(pHook, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);
		return 1;
	}
	//if here - have connected couplers pair
	if (pCp->nInSubjsSet < pCp->pCounterCoupler->nInSubjsSet)
		return 0;//will render later, with counterCoupler
	//here - render couplers pair
	Coupler* pCp2 = pCp->pCounterCoupler;
	float distBetweenCouplers = v3lengthFromToXZ(pCp->absCoords.pos, pCp2->absCoords.pos);
	float progress = pCp->lever / (pCp->lever + pCp2->lever + couplersPairLength);
	float pos[4];
	for (int i = 0; i < 3; i++)
		pos[i] = pCp->absCoords.pos[i] + (pCp2->absCoords.pos[i]- pCp->absCoords.pos[i]) * progress;

	float yaw = v3yawDgFromTo(pCp->absCoords.pos, pCp2->absCoords.pos);
	mat4x4 posModelMatrix;
	mat4x4_translate(posModelMatrix, pos[0], pos[1], pos[2]);
	mat4x4 absModelMatrix;
	mat4x4_rotate_Y(absModelMatrix, posModelMatrix, yaw * degrees2radians);

	SceneSubj* pPair = Rail::railModels.at(Rail::couplersCoupleModelN);
	m16copy((float*)pPair->absModelMatrix, (float*)absModelMatrix);
	m16copy((float*)pPair->absModelMatrixUnscaled, (float*)absModelMatrix);

	renderStandard(pPair, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);

	//shaft
	mat4x4_translate(posModelMatrix, pCp->absCoords.pos[0], pCp->absCoords.pos[1], pCp->absCoords.pos[2]);
	mat4x4_rotate_Y(absModelMatrix, posModelMatrix, yaw * degrees2radians);
	mat4x4_scale_aniso(absModelMatrix, absModelMatrix, 1, 1, distBetweenCouplers);
	SceneSubj* pShaft = Rail::railModels.at(Rail::couplerShaftModelN);
	m16copy((float*)pShaft->absModelMatrix, (float*)absModelMatrix);
	m16copy((float*)pShaft->absModelMatrixUnscaled, (float*)pShaft->absModelMatrix);
	renderStandard(pShaft, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);

	return 1;
}

