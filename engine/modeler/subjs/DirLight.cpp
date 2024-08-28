#include "DirLight.h"
#include "rr/RollingStock.h"
#include "utils.h"
#include "XMLparser.h"
#include "TheApp.h"

extern TheApp theApp;

int DirLight::onLoadDirLight(DirLight* pSS, std::string tagStr) {
	if (XMLparser::varExists("onPower", tagStr))
		pSS->dir = 0;
	if (XMLparser::varExists("onMove", tagStr)) {
		pSS->onMove = 1;
		pSS->dir = 0;
	}
	if (XMLparser::varExists("forward", tagStr))
		pSS->dir = 1;
	if (XMLparser::varExists("reverse", tagStr))
		pSS->dir = -1;
	pSS->blinkHalfCycle = XMLparser::getIntValue("blink", tagStr);
	pSS->flash = XMLparser::getIntBoolValue("flash", tagStr);
	return 1;
}

int DirLight::moveDirLight(DirLight* pSS) {
	pSS->isOn = shouldBeOn(pSS);
	int shouldBe = pSS->isOn;
	if (pSS->isOn > 0 && pSS->blinkHalfCycle > 0) {
		if (pSS->flash == 0) {
			int halfCycleN = theApp.frameN / pSS->blinkHalfCycle;
			shouldBe = halfCycleN % 2;
			if (shouldBe == 0)
				shouldBe = -1;
			if (pSS->ownCoords.pos[0] < 0)
				shouldBe = -shouldBe;
		}
		else{//flash
			int cycleFrameN= theApp.frameN % (pSS->blinkHalfCycle*2);
			if (pSS->ownCoords.pos[0] < 0)
				cycleFrameN -= pSS->blinkHalfCycle;
			if(cycleFrameN==0)
				shouldBe = 1;
			else
				shouldBe = -1;
		}
	}
	int childShouldBe = 1;
	if (shouldBe < 0)
		childShouldBe = 0;
	if (pSS->activeChildN != childShouldBe)
		pSS->activateChildN(childShouldBe);
	return 1;
}
int DirLight::shouldBeOn(DirLight* pSS) {
	SceneSubj* pRoot = pSS->pSubjsSet->at(pSS->rootN);
	if (strstr(pRoot->className, "RollingStock") != pRoot->className)
		return -1;
	RollingStock* pRS = (RollingStock*)pRoot;
	if (pRS->powerOn < 1)
		return -1;
	if (!pRS->powered) {
		//car, not loco
		Coupler* pCp = pRS->pCouplerFront;
		if(pSS->alignedWithRoot<0)
			pCp = pRS->pCouplerBack;
		if (pCp != NULL) {
			if (pCp->connected > 0)
				if (pCp->couplersInvolved == 2)
					return -1;
		}
	}
	RollingStock* pTrainRoot = (RollingStock*)pSS->pSubjsSet->at(pRS->trainRootN);
	if (pSS->dir != 0) {
		int absDir = pTrainRoot->desirableZdir * pTrainRoot->alignedWithTrainHead * pRS->alignedWithTrainHead * pSS->alignedWithRoot;
		if (absDir!=0 && absDir != pSS->dir)
			return -1; //wrong direction
	}

	if (pSS->onMove) {
		if (pTrainRoot->ownZspeed == 0)
			return -1; //stays still
	}
	if (pTrainRoot->desirableZdir == 0)
		return pSS->isOn;
	else
		return 1;
}