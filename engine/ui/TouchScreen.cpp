#include "TouchScreen.h"
#include "utils.h"
#include "TheApp.h"
#include "input.h"
#include "ScreenLine.h"
#include "Shadows.h"

extern TheApp theApp;


ScreenSubj* TouchScreen::pSelected = NULL;

std::string TouchScreen::capturedCode = "NONE";
uint32_t TouchScreen::captureFrameN;
float TouchScreen::captureCursorPos[2];
float TouchScreen::ancorPoint[4] = { 0,0,0,0 };
float TouchScreen::ancorPointOnScreen[4] = { 0,0,0,0 };
uint16_t TouchScreen::clickMillisMax=500;

int TouchScreen::cursorStatus; //-1 - off screen, 0 - hover, 1 - on, 2 - dragging
float TouchScreen::lastCursorPos[3] = { 0,0,0 };

std::vector<TouchScreen*> TouchScreen::touchScreenEvents;

LineXY TouchScreen::ax0[3];
LineXY TouchScreen::ax1[3];
LineXY TouchScreen::ax2[3];

uint64_t TouchScreen::lastClickTime = 0;
uint64_t TouchScreen::leftButtonDownTime = 0;
float TouchScreen::leftButtonDownCursorPos[2];
float TouchScreen::lastClickPos[2];

void TouchScreen::cleanUp() {
	for (int eN = touchScreenEvents.size() - 1; eN >= 0; eN--)
		delete touchScreenEvents.at(eN);
	touchScreenEvents.clear();
}
void TouchScreen::addEvent(InputEvent eventCode00, float x, float y) {
	if (eventCode00 == MOVE_CURSOR) {
		lastCursorPos[0] = x;
		lastCursorPos[1] = y;
	}
	TouchScreen* pE = new TouchScreen;
	pE->eventCode = eventCode00;
	//v2copy(pE->pos, lastCursorPos);
	pE->pos[0] = x;
	pE->pos[1] = y;
	touchScreenEvents.insert(touchScreenEvents.begin(), pE);
}
void TouchScreen::getInput() {

	while (touchScreenEvents.size() > 0) {
		TouchScreen* pE = touchScreenEvents.back();
		touchScreenEvents.pop_back();
		switch (pE->eventCode) {
		case MOVE_CURSOR:
			continue;
		case SCREEN_IN:
			if (cursorStatus <0) {
				cursorStatus = 0;
			}
			continue;
		case SCREEN_OUT:
			cursorStatus = -1;
			capturedCode.assign("NONE");
			if (pSelected != NULL) {
				pSelected->onFocusOut();
				
				pSelected = NULL;
			}
			continue;
		case LEFT_BUTTON_DOWN:
			leftButtonDown();
			continue;
		case LEFT_BUTTON_UP:
			leftButtonUp();
			continue;
		case RIGHT_BUTTON_DOWN:
			continue;
		case RIGHT_BUTTON_UP:
			continue;
		case SCROLL:
			processScroll(pE->pos[0], pE->pos[1]);
			continue;
		default:
			continue;
		}
	}
	if (cursorStatus < 0) //if cursor out of screen
		return;

	if (cursorStatus == 2) {
		//dragging
		setCursor("fist");
		pSelected->onDrag();
	}
	else{
		pointerOnSubj();
		if (cursorStatus == 1) {
			//left button down non-draggable
			if(pSelected==NULL)
				setCursor("arrow");
			else {
				setCursor("fingerdown");
				//pSelected->onDrag();
			}
		}
		else if (cursorStatus == 0) {
			//hover;
			if (pSelected == NULL)
				setCursor("hand");
			else if (pSelected->isDraggable())
				setCursor("palm");
			else if (pSelected->isClickable())
				setCursor("finger");
			else
				setCursor("arrow");
		}
	}
}



int TouchScreen::pointerOnSubj() {
	ScreenSubj* pCandidate = UISubj::pointerOnUI();
	if(pCandidate==NULL)
		pCandidate=SceneSubj::pointerOnSceneSubj(&theApp.sceneSubjs);


	if (pSelected != pCandidate) {
		if (pSelected != NULL) {
			pSelected->onFocusOut();
			capturedCode.assign("NONE");
		}
		pSelected = pCandidate;
		if (pSelected != NULL) {
			pSelected->onFocus();
			capturedCode.assign(pSelected->className);
		}
	}
	return (pSelected!=NULL);
}


void TouchScreen::leftButtonDown() {
	if (cursorStatus == 3)
		return; //multifinger (android)

	v2copy(leftButtonDownCursorPos, lastCursorPos);
	leftButtonDownTime = getSystemMillis();
	captureFrameN = theApp.frameN;
	v2copy(captureCursorPos, lastCursorPos);
	pointerOnSubj();
	if (pSelected == NULL) {
		pSelected = &theApp.gameTable;
		cursorStatus = 2; //table draggable
	}
	else {//have subj
		if (pSelected->isDraggable()) {
			cursorStatus = 2; //draggable subj
		}
		else if (pSelected->isClickable()) {
			cursorStatus = 1; //Clickable subj
		}
	}

	pSelected->onLeftButtonDown();

}
void TouchScreen::leftButtonUp() {
	capturedCode.assign("NONE");
	cursorStatus = 0;
	theApp.bPause = false;

	if (pSelected == NULL)
		return;

	int clickStatus = 0; //0-just button up, 1-click,2-double-click
	//check if click
	uint64_t currentMillis = getSystemMillis();

	//mylog("%d up\n", (int)currentMillis);

	uint16_t dMillis = currentMillis - leftButtonDownTime;
	if (dMillis < clickMillisMax) {
		//possible click
		float cursorShift = v3lengthFromToXY(leftButtonDownCursorPos, lastCursorPos);
		if (cursorShift < 5) {
			//is a click
			clickStatus = 1;
			mylog("%d click\n", theApp.frameN);
			//check if double-click
			dMillis = currentMillis - lastClickTime;
			if (dMillis < 2000) {
				//possible double-click
				cursorShift = v3lengthFromToXY(lastClickPos, lastCursorPos);
				if (cursorShift < 5) {
					//is double-click
					clickStatus = 2;
					mylog("%d double-click\n", theApp.frameN);
				}
			}
			lastClickTime = currentMillis;
			v2copy(lastClickPos, lastCursorPos);
		}
	}
	switch (clickStatus) {
	case 2://double-click
		pSelected->onDoubleClick();
		break;
	case 1://just click
		pSelected->onClick();
		break;
	default://just button up
		pSelected->onLeftButtonUp();
		break;
	}
	pSelected = NULL;
	SceneSubj::pSelectedSceneSubj = NULL;
	UISubj::pSelectedUISubj = NULL;
}
int TouchScreen::processScroll(float x, float y) {
	//ignore x, y<0-zoom out, >0-zoom in
	Camera* pCam = &theApp.mainCamera;
	float k = 1.0 + y * 0.05;
	for (int i = 0; i < 2; i++)
		pCam->stageSize[i] *= k;
	pCam->reset(pCam, &theApp.gameTable.worldBox,true);
	Shadows::resetShadowsFor(&theApp.mainCamera, &theApp.gameTable.worldBox);

	//mylog("scroll %d \n", (int)y);
	return 1;
}



