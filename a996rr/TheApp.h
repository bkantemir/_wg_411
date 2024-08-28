#pragma once
#include <vector>
#include "SceneSubj.h"
#include "UISubj.h"
#include "Camera.h"
#include "rr/RailMap45.h"
#include "TouchScreen.h"

class TheApp
{
public:
	uint32_t frameN = 0;
	//synchronization
	uint64_t lastFrameMillis = 0;
	int targetFPS = 30;
	int millisPerFrame = 1000 / targetFPS;

	bool bPause=false;
	bool bExitApp = false;
	Camera mainCamera;
	static Camera collisionCamera;
	mat4x4 collisionMViewInversed;
	float dirToMainLight[4] = { 0,0,0,0 };
	bool isDark = false;

	RailMap45 gameTable;

	TouchScreen touchScreenHadler;

	std::vector<DrawJob*> drawJobs;
	std::vector<unsigned int> buffersIds;

	std::vector<UISubj*> UISubjs;

	std::vector<SceneSubj*> sceneSubjs;
	std::vector<SceneSubj*> staticSubjs;

	std::vector<std::vector<SceneSubj*>*> pSubjArrays2draw;
	std::vector<std::vector<SceneSubj*>*> pSubjArrays2process;
	std::vector<std::vector<SceneSubj*>*> pSubjArraysPairs4collisionDetection;
public:
	int run();
	int getReady();
	int drawFrame();
	int cleanUp();
	int onScreenResize(int width, int height);
	static SceneSubj* newSceneSubj(std::string subjClass, std::string sourceFile="",
		std::vector<SceneSubj*>* pSubjsSet0 = NULL, std::vector<DrawJob*>* pDrawJobs0 = NULL);
	static void mylogObjSize(std::string objName, int objSize);
	static void showHierarchy(SceneSubj* pSS);
	int checkGameStatus();
};
