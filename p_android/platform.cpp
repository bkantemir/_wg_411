#include <android/log.h>
#include "stdio.h"
#include "TheApp.h"
#include <EGL/egl.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <sys/stat.h>	//mkdir for Android
#include "ScreenLine.h"

extern struct android_app* pAndroidApp;
extern EGLDisplay androidDisplay;
extern EGLSurface androidSurface;

extern TheApp theApp;
 
void mylog(const char* _Format, ...) {
    char outStr[4096];
    va_list _ArgList;
    va_start(_ArgList, _Format);
    vsprintf(outStr, _Format, _ArgList);
    va_end(_ArgList);
    int outStrLen = strlen(outStr);
    int printLimit = 1000;
    if(outStrLen < printLimit)
        __android_log_print(ANDROID_LOG_INFO, "mylog", outStr, NULL);
    else{ //text is too big, split
        int printed = 0;
        std::string outString = std::string(outStr);
        while(printed <= outStrLen){
            std::string part2print = outString.substr(printed,printLimit);
            __android_log_print(ANDROID_LOG_INFO, "mylog", part2print.c_str(), NULL);
            __android_log_print(ANDROID_LOG_INFO, "mylog", "---\n", NULL);
            printed += printLimit;
        }
    }
};
 
void mySwapBuffers() {
    eglSwapBuffers(androidDisplay, androidSurface);
}

int fingersOnScreen=0;
float fingersCoords[2][2];
float fingersDistance;
float multifingerAncorPoint[4];
int lastEventFrameN;

void amotionEventActionDown(){
    fingersOnScreen = 1;
	TouchScreen::addEvent(SCREEN_IN);
	TouchScreen::pointerOnSubj();
    TouchScreen::addEvent(LEFT_BUTTON_DOWN);
}
void amotionEventActionUp(){
	fingersOnScreen = 0;
    TouchScreen::addEvent(LEFT_BUTTON_UP);
	TouchScreen::addEvent(SCREEN_OUT);
}
void myPollEvents() {
    // Process all pending events before running game logic.
    int events;
    android_poll_source *pSource;
    if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0)
        if (pSource)
            pSource->process(pAndroidApp, pSource);
    //if no display - wait for it
    while (androidDisplay == EGL_NO_DISPLAY)
        if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0)
            if (pSource)
                pSource->process(pAndroidApp, pSource);

    // handle all queued inputs
    int eventsCount = pAndroidApp->motionEventsCount;
	if(eventsCount>0)
		lastEventFrameN=(int)theApp.frameN;
    //mylog("%d eventsCount=%d\n",(int)theApp.frameN,eventsCount);

    for (auto i = 0; i < eventsCount; i++) {

        // cache the current event
        auto &motionEvent = pAndroidApp->motionEvents[i];

        // cache the current action
        auto action = motionEvent.action;
        // Only consider touchscreen events, like touches
        //auto actionMasked = action & AINPUT_SOURCE_TOUCHSCREEN;
        auto actionMasked = action & AMOTION_EVENT_ACTION_MASK;

        // count fingers first
        switch (actionMasked) {
            case AMOTION_EVENT_ACTION_DOWN:

                mylog("%d Finger down\n",(int)theApp.frameN);

                fingersOnScreen=1;
                break;
            case AMOTION_EVENT_ACTION_UP:
                (int)theApp.frameN;

                mylog("%d Finger up\n",(int)theApp.frameN);

                fingersOnScreen=0;
                break;
           case AMOTION_EVENT_ACTION_POINTER_DOWN:

                //mylog("Pointer down\n");

                fingersOnScreen++;
                break;
           case AMOTION_EVENT_ACTION_POINTER_UP:

                //mylog("Pointer up\n");

                fingersOnScreen--;
                break;

            default:
                ;//aout << "Pointer Move";
        }

        // get pointers positions
		//if(fingersOnScreen>0){
			auto &pointer = motionEvent.pointers[0];
			fingersCoords[0][0] = GameActivityPointerAxes_getX(&pointer);
			fingersCoords[0][1] = GameActivityPointerAxes_getY(&pointer);
			//v2copy(TouvhScreen::lastCursorPos,fingersCoords[0]);
			if(!v2match(TouchScreen::lastCursorPos,fingersCoords[0]))
				TouchScreen::addEvent(MOVE_CURSOR, fingersCoords[0][0], fingersCoords[0][1]);
			if(fingersOnScreen>1){
				auto &pointer = motionEvent.pointers[1];
				fingersCoords[1][0] = GameActivityPointerAxes_getX(&pointer);
				fingersCoords[1][1] = GameActivityPointerAxes_getY(&pointer);
			}        
		//}
        //mylog("%d: [%d/%d] [%d/%d]\n", (int)theApp.frameN,(int)x,(int)y,(int)x2,(int)y2);

        // determine the kind of event it is
        switch (actionMasked) {
            case AMOTION_EVENT_ACTION_DOWN:
				amotionEventActionDown();
                break;
           case AMOTION_EVENT_ACTION_UP:
				amotionEventActionUp();

                mylog("%d Finger up\n",(int)theApp.frameN);

                break;
           case AMOTION_EVENT_ACTION_POINTER_DOWN:
				//remember initial distance
				fingersDistance = v3lengthFromToXY(fingersCoords[0],fingersCoords[1]);
				//remember initial ancorPoint
				float multifingerCenter[2];
				for(int i=0;i<2;i++)
					multifingerCenter[i]=(fingersCoords[0][i]+fingersCoords[1][i])/2;
				TheTable::getCursorAncorPointTable(multifingerAncorPoint, multifingerCenter, &theApp.gameTable);

				TouchScreen::cursorStatus=3;
				theApp.bPause = true;
				TouchScreen::capturedCode.assign("MultiFinger");
				if(TouchScreen::pSelected!=NULL){
					TouchScreen::pSelected->onFocusOut();
					TouchScreen::pSelected=NULL;
				}
				
               mylog("%d POINTER down\n",(int)theApp.frameN);

                break;
           case AMOTION_EVENT_ACTION_POINTER_UP:

                mylog("%d POINTER up\n",(int)theApp.frameN);

                break;

            default:
                ;//aout << "Pointer Move";
        }
    }

    if(eventsCount==0 && fingersOnScreen>0){
        //missed last finger up
         if( (int)theApp.frameN-lastEventFrameN>15) {
			amotionEventActionUp();


			mylog("%d missed finger up\n",(int)theApp.frameN);
		 }
    }
    else if(eventsCount>0 && fingersOnScreen<1){
        //missed first finger down?
        if( (int)theApp.frameN-lastEventFrameN>0) {
			amotionEventActionDown();

            mylog("%d missed finger down\n", (int) theApp.frameN);
        }
    }

	if(TouchScreen::cursorStatus==3 && fingersOnScreen>1){ //multitouch
		float dist = v3lengthFromToXY(fingersCoords[0],fingersCoords[1]);
		if(fingersDistance != dist){
			//recenter
			float newCenter[2];
			for(int i=0;i<2;i++)
				newCenter[i]=(fingersCoords[0][i]+fingersCoords[1][i])/2;
			float newAncor[4];
			TheTable::getCursorAncorPointTable(newAncor, newCenter, &theApp.gameTable);
			float shift[4];
			for (int i = 0; i < 3; i++)
				shift[i] = multifingerAncorPoint[i] - newAncor[i];

			//mylog("%d: %d x %d shift %d x %d\n", (int)theApp.frameN, (int)newAncor[0], (int)newAncor[2], (int)shift[0], (int)shift[2]);

			//adjust cam lookAtPoint
            Camera* pCam=&theApp.mainCamera;
			pCam->lookAtPoint[0] += shift[0];
			pCam->lookAtPoint[2] += shift[2];

			Camera::reset(pCam, &theApp.gameTable.worldBox,true);
			TheTable::getCursorAncorPointTable(multifingerAncorPoint, TouchScreen::lastCursorPos, &theApp.gameTable);
			///////////////////////////////
			//scale
			float d=fingersDistance-dist;
			TouchScreen::addEvent(SCROLL, 0, d/20);
			fingersDistance = dist;
		}
		//ribbon
		ScreenLine::addLine2queue(fingersCoords[0],fingersCoords[1], MyColor::getUint32(0.0f,1.0f,0.0f,0.5f), 8, true);
	}
	
    android_app_clear_motion_events(pAndroidApp);

    // handle key inputs
    for (auto i = 0; i < pAndroidApp->keyUpEventsCount; i++) {
        // cache the current event
        auto &keyEvent = pAndroidApp->keyUpEvents[i];
        if (keyEvent.keyCode == AKEYCODE_BACK) {
            // actions on back key
            theApp.bExitApp = true;
        }
    }
    android_app_clear_key_up_events(pAndroidApp);
	
	
    //updateRenderArea
    EGLint width,height;
    eglQuerySurface(androidDisplay, androidSurface, EGL_WIDTH, &width);
    eglQuerySurface(androidDisplay, androidSurface, EGL_HEIGHT, &height);
    //screenSize[0] = 0;
    //screenSize[1] = 0;
        //mylog(">>>>>>>>>>>>>>>APP_CMD_INIT_WINDOW %d x %d\n",width,height);
    theApp.onScreenResize(width,height);

}
void strcpy_s(char* dst, int maxSize, const char* src) {
	strcpy(dst, src);
	//fill tail by zeros
	int strLen = strlen(dst);
	if (strLen < maxSize)
		for (int i = strLen; i < maxSize; i++)
			dst[i] = 0;
}
int fopen_s(FILE** pFile, const char* filePath, const char* mode) {
	*pFile = fopen(filePath, mode);
	if (*pFile == NULL) {
		mylog("ERROR: can't open file %s\n", filePath);
		return -1;
	}
	return 1;
}
int myMkDir(const char* outPath) {
	struct stat info;
	if (stat(outPath, &info) == 0)
		return 0; //exists already
	int status = mkdir(outPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status == 0)
		return 1; //Successfully created
	mylog("ERROR creating, status=%d, errno: %s.\n", status, std::strerror(errno));
	return -1;
}

void sprintf_s(char *buffer, size_t sizeOfBuffer,const char* _Format, ...) {
    va_list _ArgList;
    va_start(_ArgList, _Format);
    vsprintf(buffer, _Format, _ArgList);
    va_end(_ArgList);
};
