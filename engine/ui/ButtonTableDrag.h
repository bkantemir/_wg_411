#pragma once
#include "UISubj.h"
#include <vector>

class ButtonTableDrag : public UISubj
{
public:
	static int drag_table_xy;
	static int drag_table_xz;
	static int drag_table_spin;
	static int drag_table_sun;

	static float defaultAlpha;
	static uint32_t updatedInFrameN;
	static bool dissolve;
	static char defaultMode64[64];

public:
	ButtonTableDrag();
	ButtonTableDrag(std::string name0, std::vector<UISubj*>* pSubjs = NULL, std::vector<DrawJob*>* pDJs = NULL)
		: UISubj(name0, pSubjs, pDJs) {	strcpy_s(className, 32, "ButtonTableDrag");	};
	static int addTableButtons();
	static int addTableButton(std::string uiName, float x, float y, float w, float h, std::string alignTo, std::string src);
	virtual int render() { return renderButtonTable(this); };
	static int renderButtonTable(UISubj* pUI);
	virtual bool isDraggable() { return true; };
	virtual bool isResponsive() { return true; };
	virtual int onLeftButtonDown() { return onLeftButtonDownTableDrag(this); };
	static int onLeftButtonDownTableDrag(ButtonTableDrag* pBt);

};
