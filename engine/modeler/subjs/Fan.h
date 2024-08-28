#pragma once
#include "SceneSubj.h"

class Fan : public SceneSubj
{
public:
	float spinAngle=0;
public:
	Fan() {};
	Fan(Fan* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(Fan)); };
	virtual Fan* clone() {
		if (strcmp(this->className, "Fan") != 0)
			return NULL;
		return new Fan(this);
	};
	virtual int moveSubj() { return moveFan(this); };
	static int moveFan(Fan* pGS);
	virtual int onLoad(std::string tagStr) { return onLoadFan(this, tagStr); };
	static int onLoadFan(Fan* pSS, std::string tagStr);

};
