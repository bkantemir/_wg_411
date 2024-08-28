#pragma once
#include "UISubj.h"

class ScreenLine : UISubj
{
public:
	float point[2][3] =
	{
		{ 0.0f,  0.0f, 0.f },
		{ 100.f, 100.f, 0.f }
	};
public:
	ScreenLine(std::vector<UISubj*>* pSubjs = NULL, std::vector<DrawJob*>* pDJs = NULL) 
		: UISubj("simply ScreenLine",pSubjs, pDJs) { strcpy_s(className, 32, "ScreenLine"); };
	virtual int render() { return drawScreenLine(this); };
	static int drawScreenLine(ScreenLine* pUI);
	static int addLine2queue(float* p0, float* p1, unsigned int RGBA32, float lineWidth, bool deleteOnDraw);
};
