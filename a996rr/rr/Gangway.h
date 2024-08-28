#pragma once
#include "SceneSubj.h"

class Gangway : public SceneSubj
{
public:
	Gangway* pCounterGangway = NULL;
	int level = 1;

	static float yLevel1;
	static float yLevel2;

public:
	Gangway() {};
	Gangway(Gangway* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(Gangway)); };
	virtual Gangway* clone() {
		if (strcmp(this->className, "gangway") != 0)
			return NULL;
		return new Gangway(this);
	};
	virtual int render(Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap) {
		return renderGangway(this, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);
	};
	static int renderGangway(Gangway* pCp, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap);

	virtual int moveSubj() { return checkGangway(this); };
	static int checkGangway(Gangway* pGS);
	virtual int onLoad(std::string tagStr) { return onLoadGangway(this, tagStr); };
	static int onLoadGangway(Gangway* pGw, std::string tagStr);
};


