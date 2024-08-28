#include "RRUI.h"
#include "TheApp.h"
#include "ButtonTableDrag.h"

extern TheApp theApp;


void RRUI::initPlayScreen() {
	UISubj::clear();
	ButtonTableDrag::addTableButtons();
}
