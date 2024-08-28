#pragma once
#include "SceneSubj.h"

class Switcher : public SceneSubj
{
public:
	int childsN = 0;
	int d2lastChild = 0;
	int activeChildN=0; //child #, not nInSubjsSet
public:
	virtual int onDeploy(std::string tagStr) { return onDeploySwitcher(this, tagStr); };
	static int onDeploySwitcher(Switcher* pSS, std::string tagStr);
	int activateChildN(int childN) {return activateChildN(this, childN);};
	static int activateChildN(Switcher* pSS, int childN);
};