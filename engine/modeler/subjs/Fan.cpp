#include "Fan.h"
#include "XMLparser.h"

int Fan::moveFan(Fan* pSS) {
	/*
	SceneSubj* pRoot = pSS->pSubjsSet->at(pSS->rootN);
	if (strstr(pRoot->className, "RollingStock") != pRoot->className)
		return -1;
	RollingStock* pRS = (RollingStock*)pRoot;
	*/
	float rotationSpeedDg = 5;
	pSS->spinAngle = angleDgNorm180(pSS->spinAngle + rotationSpeedDg);
	pSS->ownCoords.setRoll(pSS->spinAngle);
	return 1;
}
int Fan::onLoadFan(Fan* pSS, std::string tagStr) {
	/*
	pSS->lightIntencity = XMLparser::getFloatValue("lightIntencity", tagStr);
	if (XMLparser::varExists("lightColor", tagStr)) {
		unsigned int uintColor = 0;
		XMLparser::setUintColorValue(&uintColor, "lightColor", tagStr);
		pSS->lightColor.setUint32(uintColor);
	}
	*/
	return 1;
}

