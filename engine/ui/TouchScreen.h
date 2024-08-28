#pragma once
#include <vector>
#include "ScreenSubj.h"
#include "Gabarites.h"

typedef enum {
    MOVE_CURSOR,
    SCREEN_IN,
    SCREEN_OUT,
    LEFT_BUTTON_DOWN,
    LEFT_BUTTON_UP,
    RIGHT_BUTTON_DOWN,
    RIGHT_BUTTON_UP,
    SCROLL //treat as zoom?
} InputEvent;

class TouchScreen
{
public:
	int eventCode = 0;
    float pos[2];

    static ScreenSubj* pSelected;

    static float lastCursorPos[3];
	static int cursorStatus; //-1 - off screen, 0 - hover, 1 - on, 2-dragging, 3 - multifinger (android)

	static std::vector<TouchScreen*> touchScreenEvents;
    static std::string capturedCode;
    static uint32_t captureFrameN;
    static float captureCursorPos[2];
    static float ancorPoint[4];
    static float ancorPointOnScreen[4];
    
    static LineXY ax0[3];
    static LineXY ax1[3];
    static LineXY ax2[3];
    
    static uint64_t leftButtonDownTime;
    static float leftButtonDownCursorPos[2];
    static uint64_t lastClickTime;
    static float lastClickPos[2];
    static uint16_t clickMillisMax;

public:
	static void cleanUp();
    static void addEvent(InputEvent event, float x=0, float y=0);
    static void getInput();
    static void leftButtonDown();
    static void leftButtonUp();
    static int pointerOnSubj();
    static int processScroll(float x, float y);
}; 

