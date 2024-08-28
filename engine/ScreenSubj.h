#pragma once
#include "linmath.h"

class ScreenSubj
{
public:
    char className[32] = "";
    char name64[64] = ""; //64 because of GLTF node names

public:
    virtual bool isClickable() { return false; };
    virtual bool isDraggable() { return false; };
    virtual bool isResponsive() { return (isClickable() || isDraggable()); };

    virtual int onDrag() { return 1; };
    virtual int onClick() { onFocusOut(); return 1; };
    virtual int onDoubleClick() { return 1; };
    virtual int onLongClick() { return 1; };
    virtual int onFocusOut() { return 1; };
    virtual int onFocus() { return 1; };
    virtual int onLeftButtonUp() { onFocusOut(); return 1; };
    virtual int onLeftButtonDown() { return 1; };
    virtual ScreenSubj* getResponsiveSubj(){return NULL;};
};
