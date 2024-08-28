#pragma once
#include "Switcher.h"

class DirLight : public Switcher
{
public:
	int isOn = -1;
	int dir = 1; //if-1 - reverse, 0-doesn't matter
	int onMove = 0;
	int blinkHalfCycle = 0;
	int flash = 0;
public:
	DirLight() {};
	DirLight(DirLight* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(DirLight)); };
	virtual DirLight* clone() {
		if (strcmp(this->className, "DirLight") != 0)
			return NULL;
		return new DirLight(this);
	};
	virtual int moveSubj() { return moveDirLight(this); };
	static int moveDirLight(DirLight* pGS);
	static int shouldBeOn(DirLight* pGS);
	virtual int onLoad(std::string tagStr) { return onLoadDirLight(this, tagStr); };
	static int onLoadDirLight(DirLight* pSS, std::string tagStr);

};
