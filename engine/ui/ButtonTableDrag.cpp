
#include "ButtonTableDrag.h"
#include "Texture.h"
#include "TheApp.h"

extern TheApp theApp;
extern float displayDPI;

int ButtonTableDrag::drag_table_xy = -1;
int ButtonTableDrag::drag_table_xz = -1;
int ButtonTableDrag::drag_table_spin = -1;
int ButtonTableDrag::drag_table_sun=-1;
float ButtonTableDrag::defaultAlpha=0;
uint32_t ButtonTableDrag::updatedInFrameN=0;
bool ButtonTableDrag::dissolve=false;
char ButtonTableDrag::defaultMode64[64] = "drag_table_xz";

int ButtonTableDrag::addTableButtons() {
	float r = 0.5*displayDPI;
	float d = (int)(r * 1.2);
	float x = d * 2;
	float y = -r * 2;
	defaultAlpha = 0;
	//init table drag group
	//drag_table_xy = addTableButton("drag_table_xy", x, y += d, r, r, "", "/dt/img/buttons/drag_table_xy.bmp");
	drag_table_xz = addTableButton("drag_table_xz", x, y += d, r, r, "", "/dt/img/buttons/drag_table_xz.bmp");
	drag_table_spin = addTableButton("drag_table_spin", x, y += d, r, r, "", "/dt/img/buttons/drag_table_spin.bmp");
	drag_table_sun = addTableButton("drag_table_sun", x, y += d, r, r, "", "/dt/img/buttons/drag_table_sun.bmp");
	return 1;
}
int ButtonTableDrag::addTableButton(std::string uiName, float x, float y, float w, float h, std::string alignTo, std::string src) {

	UISubj* pUI = new ButtonTableDrag(uiName, NULL, NULL);
	//strcpy_s(pUI->name, 32, uiName.c_str());

	std::vector<DrawJob*>* pDrawJobs = pUI->pDrawJobs;

	setCoords(pUI, x, y, w, h, alignTo);

	pUI->djStartN = djNround;
	memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));

	int uTex0 = Texture::loadTexture(src);

	pUI->mt0.uTex0 = uTex0;

	pUI->mt0.uAlphaFactor = 0.33;


	return pUI->nInSubjs;
}
int ButtonTableDrag::renderButtonTable(UISubj* pUI) {
	if (updatedInFrameN != theApp.frameN) {
		updatedInFrameN = theApp.frameN;
		if (TouchScreen::pSelected != &theApp.gameTable)
			TouchScreen::capturedCode.assign("NONE");
		if (TouchScreen::capturedCode.find("drag_table_") == 0) {
			//dragging
			dissolve = false;
			defaultAlpha=0.33;
		}
		else { //not dragging
			if (defaultAlpha > 0) {
				dissolve = true;
			}
		}
		if (dissolve) {
			defaultAlpha -= 0.002;
			if (defaultAlpha <= 0) {
				defaultAlpha = 0;
				dissolve = false;
			}
		}
	}
	if(TouchScreen::capturedCode.compare(pUI->name64)==0)
		pUI->mt0.uAlphaFactor = 1;
	else if(pSelectedUISubj==pUI)//hover
		pUI->mt0.uAlphaFactor = 0.66;
	else
		pUI->mt0.uAlphaFactor = defaultAlpha;
	return renderStandard(pUI);
}
int ButtonTableDrag::onLeftButtonDownTableDrag(ButtonTableDrag* pBt) {
	strcpy_s(defaultMode64, 64, pBt->name64);
	TouchScreen::pSelected = &theApp.gameTable;
	TouchScreen::capturedCode.assign(defaultMode64);
	TouchScreen::pSelected->onLeftButtonDown();

	SceneSubj::pSelectedSceneSubj = &theApp.gameTable;
	SceneSubj::pSelectedSceneSubj00 = &theApp.gameTable;

	return 1;
}