#pragma once
#include "DrawJob.h"
#include "SceneSubj.h"
#include "TouchScreen.h"

class TheTable : public SceneSubj
{
public:
	int tableTiles[2] = { 1,1 };
	float tileSize = 100;
	float groundLevel = 0;
	Gabarites worldBox;
	float suggestedStageSize = 100;

	std::vector<DrawJob*> table_drawJobs;
	std::vector<unsigned int> table_buffersIds;
	std::vector<SceneSubj*> tableParts;

public:
	virtual ~TheTable();
	int initTable(float tileSizeXZ, float tileSizeUp, float tileSizeDown, int tilesNx, int tilesNz);
	void cleanUpTable();
	void placeAt(float* pos, float x, float y,float z);

	virtual bool isDraggable() { return true; };
	virtual int onDrag();
	virtual int onFocusOut();
	virtual int onFocus() { return 1; };
	virtual int onLeftButtonUp() { return onFocusOut(); };
	virtual int onLeftButtonDown();
	static int getCursorAncorPointTable(float* ancorPoint, float* cursorPos, TheTable* pTable);
	virtual int onClick() { return onLeftButtonUp(); };
	virtual int onDoubleClick() { return onLeftButtonUp(); };
};


