#pragma once
#include "SceneSubj.h"

class Interior : public SceneSubj
{
public:
	float lightIntencity = 0.5;
	MyColor lightColor;
	float darkIntencity = 0;
public:
	Interior() { lightColor.setRGBA(1.0f, 1.0f, 0.7f, 1.0f); };
	Interior(Interior* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(Interior)); };
	virtual Interior* clone() {
		if (strcmp(this->className, "Interior") != 0)
			return NULL;
		return new Interior(this);
	};
	virtual int moveSubj() { return moveInterior(this); };
	static int moveInterior(Interior* pGS);
	virtual int onLoad(std::string tagStr) { return onLoadInterior(this, tagStr); };
	static int onLoadInterior(Interior* pSS, std::string tagStr);

};
