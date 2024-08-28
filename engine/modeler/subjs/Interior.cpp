#include "Interior.h"
#include "XMLparser.h"
#include "rr/RollingStock.h"
#include "TheApp.h"

extern TheApp theApp;

int Interior::moveInterior(Interior* pSS) {
	
	SceneSubj* pRoot = pSS->pSubjsSet->at(pSS->rootN);
	if (strstr(pRoot->className, "RollingStock") != pRoot->className)
		return -1;
	RollingStock* pRS = (RollingStock*)pRoot;
	
	bool lightOn = pRS->powerOn;//&& theApp.isDark;

	if (!lightOn) {//light off
		if(pSS->darkIntencity==0)
			pSS->uHighLightLevel = 0;
		else {
			pSS->uHighLightLevel = pSS->darkIntencity;
			pSS->uHighLightColor.setRGBA(0,0,0);
		}
	}
	else{//light on
		if (pSS->lightIntencity == 0)
			pSS->uHighLightLevel = 0;
		else {
			pSS->uHighLightLevel = pSS->lightIntencity;
			memcpy(&pSS->uHighLightColor, &pSS->lightColor, sizeof(MyColor));
		}
	}
	return 1;
}
int Interior::onLoadInterior(Interior* pSS, std::string tagStr) {
	pSS->darkIntencity = XMLparser::getFloatValue("darkIntencity", tagStr);
	pSS->lightIntencity = XMLparser::getFloatValue("lightIntencity", tagStr);
	if (XMLparser::varExists("lightColor", tagStr)) {
		unsigned int uintColor = 0;
		XMLparser::setUintColorValue(&uintColor, "lightColor", tagStr);
		pSS->lightColor.setUint32(uintColor);
	}
	return 1;
}

