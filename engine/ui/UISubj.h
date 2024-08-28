#pragma once
#include <vector>
#include "Coords2D.h"
#include "linmath.h"
#include "DrawJob.h"
#include "ScreenSubj.h"

class UISubj : public ScreenSubj
{
public:
	int d2parent = 0; //shift to parent object

	Coords2D ownCoords;
	char countFrom[32] = ""; //top left...
	Coords2D absCoords;
	Coords2D ownSpeed;
	float scale[3] = { 1,1,1 };
	mat4x4 transformMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	bool transformMatrixIsReady = false;

	int djStartN = 0; //first DJ N in DJs array (DrawJob::drawJobs)
	int djTotalN = 1; //number of DJs
	Material mt0;
	std::vector<DrawJob*>* pDrawJobs = NULL;
	std::vector<UISubj*>* pSubjs = NULL;
	int nInSubjs = -1;

	bool deleteOnDraw = false;

	static int djNtex;
	static int djNclr;
	static int djNdepthmap;
	static int djNline;
	static int djNframe;
	static int djNround;
	static mat4x4 mOrthoViewProjection;

	static float screenSize[2];
	static float screenAspectRatio;
	static UISubj* pSelectedUISubj;
public:
	UISubj() {};
	UISubj(std::string name0, std::vector<UISubj*>* pSubjs = NULL, std::vector<DrawJob*>* pDJs = NULL);
	virtual ~UISubj();
	static int init();
	static int clear();
	static int onScreenResize(int w, int h);
	static int renderAll();
	virtual int render() { return renderStandard(this); };
	static int renderStandard(UISubj* pUI);
	static int addZBufferSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, int uTex0);
	static int addTexSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, int uTex0);
	static int addClrSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, unsigned int rgba);
	static int addFrameSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, unsigned int rgba, float lineW = 1);
	static int addRoundSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, std::string src);
	static int setCoords(UISubj* pUI, float x, float y, float w, float h, std::string countFrom);
	static void buildModelMatrix(UISubj* pUI);
	static int executeDJbasic(DrawJob* pDJ, float* uMVP, Material* pMt);
	static int findUIslot(std::vector<UISubj*>* pSubjs, int needSlotsN=1);
	virtual bool isResponsive() { return false; };
	virtual int onLeftButtonDown();
	virtual int onFocusOut();
	virtual int onFocus();
	static UISubj* pointerOnUI();

};


