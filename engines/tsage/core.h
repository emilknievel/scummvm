/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/core.h $
 * $Id: core.h 227 2011-02-11 22:13:54Z dreammaster $
 *
 */

#ifndef TSAGE_CORE_H
#define TSAGE_CORE_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/error.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "tsage/events.h"
#include "tsage/graphics.h"
#include "tsage/resources.h"
#include "tsage/saveload.h"
#include "tsage/sound.h"

namespace tSage {

#define MAX_FLAGS 256

class EventHandler;
class SceneObject;
class SceneObjectList;
class ObjectMover;
class Action;
class Serialiser;

class InvObject: public SavedObject {
public:
	int _sceneNumber;
	int _displayResNum;
	int _rlbNum;
	int _cursorNum;
	Rect _bounds;
	CursorType _cursorId;
	Common::String _description;
	int _iconResNum;
public:
	InvObject(int sceneNumber, int rlbNum, int cursorNum, CursorType cursorId, const Common::String description);

	bool inInventory() const { return _sceneNumber == 1; }
	void setCursor();

	virtual Common::String getClassName() { return "InvObject"; }
	virtual void synchronise(Serialiser &s) {
		s.syncAsUint16LE(_sceneNumber);
	}
};

class InvObjectList: public SavedObject {
public:
	InvObject _stunner;
	InvObject _scanner;
	InvObject _stasisBox;
	InvObject _infoDisk;
	InvObject _stasisNegator;
	InvObject _keyDevice;
	InvObject _medkit;
	InvObject _ladder;
	InvObject _rope;
	InvObject _key;
	InvObject _translator;
	InvObject _ale;
	InvObject _paper;
	InvObject _waldos;
	InvObject _stasisBox2;
	InvObject _ring;
	InvObject _cloak;
	InvObject _tunic;
	InvObject _candle;
	InvObject _straw;
	InvObject _scimitar;
	InvObject _sword;
	InvObject _helmet;
	InvObject _items;
	InvObject _concentrator;
	InvObject _nullifier;
	InvObject _peg;
	InvObject _vial;
	InvObject _jacket;
	InvObject _tunic2;
	InvObject _bone;
	InvObject _jar;
	InvObject _emptyJar;

	List<InvObject *> _itemList;
	InvObject *_selectedItem;
public:
	InvObjectList();

	virtual Common::String getClassName() { return "InvObjectList"; }
	virtual void synchronise(Serialiser &s);
};

/*--------------------------------------------------------------------------*/

/**
 * Basic reference counter class
 */
class RefCounter: public Serialisable {
private:
	int _ctr;
public:
	RefCounter() { clear(); }
	virtual ~RefCounter() {}

	RefCounter(int v) { _ctr = v; }

	void clear() { _ctr = 0; }
	void setCtr(int v) { _ctr = v; }
	int decCtr() {
		if (_ctr > 0) --_ctr;
		return _ctr;
	}
	int incCtr() { return ++_ctr; }
	int getCtr() const { return _ctr; }

	virtual void synchronise(Serialiser &s) { s.syncAsSint16LE(_ctr); }
};

class EventHandler: public SavedObject {
public:
	Action *_action;

	EventHandler(): SavedObject() { _action = NULL; }
	virtual ~EventHandler() { destroy(); }

	virtual void synchronise(Serialiser &s) { SYNC_POINTER(_action); }
	virtual Common::String getClassName() { return "EventHandler"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL) {}
	virtual void remove() {}
	virtual void signal() {}
	virtual void process(Event &event) {}
	virtual void dispatch();
	virtual void setAction(Action *action) { setAction(action, NULL); }
	virtual void setAction(Action *action, EventHandler *fmt, ...);
	virtual void destroy() {};
};

class Action: public EventHandler {
public:
	EventHandler *_owner;
	int _actionIndex;
	int _delayFrames;
	uint32 _startFrame;
	int _field16;
	EventHandler *_fmt;

	Action();

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "Action"; }
	virtual void remove();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void attached(EventHandler *newOwner, EventHandler *fmt, va_list va);
	
	void attach(EventHandler *newOwner, EventHandler *fmt, ...) {
		va_list va;
		va_start(va, fmt);
		attached(newOwner, fmt, va);
		va_end(va);
	}
	int getActionIndex() const { return _actionIndex; }
	void setActionIndex(int index) { _actionIndex = index; }
	void setDelay(int numFrames);
};

class ObjectMover: public EventHandler {
public:
	Common::Point _destPosition;
	Common::Point _moveDelta;
	Common::Point _moveSign;
	int _minorDiff;
	int _majorDiff;
	int _field1A;
	Action *_action;
	SceneObject *_sceneObject;
public:
	ObjectMover() { _action = NULL; _sceneObject = NULL; }
	virtual ~ObjectMover();

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "ObjectMover"; }
	virtual void remove();
	virtual void dispatch();
	virtual void startMove(SceneObject *sceneObj, va_list va) {}
	virtual void setup(const Common::Point &destPos);
	virtual bool dontMove() const;
	virtual void endMove();
};

class ObjectMover2: public ObjectMover {
public:
	SceneObject *_destObject;
	int _minArea;
	int _maxArea;
public:
	ObjectMover2();
	virtual ~ObjectMover2() {}

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "ObjectMover2"; }
	virtual void dispatch();
	virtual void startMove(SceneObject *sceneObj, va_list va);
	virtual void endMove();
};

class ObjectMover3: public ObjectMover2 {
public:
	virtual Common::String getClassName() { return "ObjectMover3"; }
	virtual void dispatch();
	virtual void startMove(SceneObject *sceneObj, va_list va);
	virtual void endMove();
};

class NpcMover: public ObjectMover {
public:
	virtual Common::String getClassName() { return "NpcMover"; }
	virtual void startMove(SceneObject *sceneObj, va_list va);
};

#define MAX_ROUTE_SIZE 20
#define ROUTE_END_VAL -20000

class RouteEnds {
public:
	Common::Point moveSrc;
	Common::Point moveDest;
};

class PlayerMover: public NpcMover {
private:
	void setDest(const Common::Point &destPos);
	void pathfind(Common::Point *routeList, Common::Point srcPos, Common::Point destPos, RouteEnds routeEnds);
	int regionIndexOf(const Common::Point &pt);
	int regionIndexOf(int xp, int yp) { return regionIndexOf(Common::Point(xp, yp)); }
	int findClosestRegion(Common::Point &pt, List<int> &indexList);
	int checkMover(Common::Point &srcPos, const Common::Point &destPos);
	void checkMovement2(const Common::Point &pt1, const Common::Point &pt2, int numSteps, Common::Point &ptOut);
	int proc1(int *routeList, int srcRegion, int destRegion, int &v);

	static Common::Point *findLinePoint(RouteEnds *routeEnds, Common::Point *objPos, int length, Common::Point *outPos);
	static int findDistance(const Common::Point &pt1, const Common::Point &pt2);
	static bool sub_F8E5(const Common::Point &pt1, const Common::Point &pt2, const Common::Point &pt3,
		const Common::Point &pt4, Common::Point *ptOut = NULL);
public:
	Common::Point _finalDest;
	Common::Point _routeList[MAX_ROUTE_SIZE];
	int _routeIndex;

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "PlayerMover"; }
	virtual void startMove(SceneObject *sceneObj, va_list va);
	virtual void endMove();
};

/*--------------------------------------------------------------------------*/

class ScenePalette;

class PaletteModifier: public SavedObject {
public:
	ScenePalette *_scenePalette;
	Action *_action;
public:
	PaletteModifier();

	virtual void synchronise(Serialiser &s) {
		SYNC_POINTER(_scenePalette);
		SYNC_POINTER(_action);
	}
	virtual void signal() = 0;
	virtual void remove() = 0;
};

class PaletteRotation: public PaletteModifier {
public:
	bool _disabled;
	int _delayFrames;
	int _delayCtr;
	uint32 _frameNumber;
	int _currIndex;
	int _start;
	int _end;
	int _rotationMode;
	int _duration;
	uint32 _palette[256];
public:
	PaletteRotation();

	virtual Common::String getClassName() { return "PaletteRotation"; }
	virtual void synchronise(Serialiser &s);
	virtual void signal();
	virtual void remove();

	void setDisabled(bool v) { _disabled = v; }
	void set(ScenePalette *palette, int start, int end, int rotationMode, int duration, Action *action);
	void setPalette(ScenePalette *palette, bool disabled);
	bool decDuration();
	void setDelay(int amount);
};

enum FadeMode {FADEMODE_NONE = 0, FADEMODE_GRADUAL = 1, FADEMODE_IMMEDIATE = 2};

class ScenePalette: public SavedObject {
public:
	uint32 _palette[256];
	GfxColours _colours;
	List<PaletteModifier *> _listeners;
	int _field412;

	uint8 _redColour;
	uint8 _greenColour;
	uint8 _blueColour;
	uint8 _aquaColour;
	uint8 _purpleColour;
	uint8 _limeColour;
public:
	ScenePalette();
	ScenePalette(int paletteNum);

	bool loadPalette(int paletteNum);
	void refresh();
	void setPalette(int index, int count);
	uint8 indexOf(uint r, uint g, uint b, int threshold = 0xffff);
	void getPalette(int start = 0, int count = 256);
	void signalListeners();
	void clearListeners();
	void fade(const byte *adjustData, bool fullAdjust, int percent);
	PaletteRotation *addRotation(int start, int end, int rotationMode, int duration = 0, Action *action = NULL);

	static void changeBackground(const Rect &bounds, FadeMode fadeMode);

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "ScenePalette"; }
};

// DisplayParamType constant set. This must not be an enum
const int SET_WIDTH = 0;
const int SET_X = 1;
const int SET_Y = 2;
const int SET_FONT = 3;
const int SET_BG_COLOUR = 4;
const int SET_FG_COLOUR = 5;
const int SET_KEEP_ONSCREEN = 6;
const int SET_EXT_BGCOLOUR = 7;
const int SET_EXT_FGCOLOUR = 8;
const int SET_POS_MODE = 9;
const int SET_TEXT_MODE = 10;
const int LIST_END = -999;

class SceneItem: public EventHandler {
public:
	Rect _bounds;
	Common::String _msg;
	int _fieldE, _field10;
	Common::Point _position;
	int _yDiff;
	int _sceneRegionId;
public:
	SceneItem(): EventHandler() { _msg = "Feature"; _action = NULL; }

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "SceneItem"; }
	virtual void remove();
	virtual void destroy() {}
	virtual void startMover(CursorType action) { doAction(action); }
	virtual void doAction(int action);

	bool contains(const Common::Point &pt);
	void setBounds(const Rect &newBounds) { _bounds = newBounds; }
	static void display(int resNum, int lineNum, ...);
	static void display2(int resNum, int lineNum) {
		display(resNum, lineNum, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
	}
};

class SceneHotspot: public SceneItem {
public:
	SceneHotspot(): SceneItem() {}

	virtual Common::String getClassName() { return "SceneHotspot"; }
	virtual void doAction(int action);
};

enum AnimateMode {ANIM_MODE_NONE = 0, ANIM_MODE_1 = 1, ANIM_MODE_2 = 2, ANIM_MODE_3 = 3,
		ANIM_MODE_4 = 4, ANIM_MODE_5 = 5, ANIM_MODE_6 = 6, ANIM_MODE_7 = 7, ANIM_MODE_8 = 8};

class SceneObject;

class Visage {
private:
	byte *_data;
public:
	int _resNum;
	int _rlbNum;
public:
	Visage();
	~Visage();

	void setVisage(int resNum, int rlbNum = 9999);
	GfxSurface getFrame(int frameNum);
	int getFrameCount() const;
};

class SceneObjectWrapper: public EventHandler {
private:
	Visage _visageImages;
public:
	SceneObject *_sceneObject;
public:
	SceneObjectWrapper() { _sceneObject = NULL; }
	virtual ~SceneObjectWrapper() {}

	void setSceneObject(SceneObject *so);

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "SceneObjectWrapper"; }
	virtual void dispatch();
};

enum ObjectFlags {OBJFLAG_FIXED_PRIORITY = 1, OBJFLAG_NO_UPDATES = 2, OBJFLAG_ZOOMED = 4, 
	OBJFLAG_8 = 8, OBJFLAG_100 = 0x100, OBJFLAG_200 = 0x200, OBJFLAG_REMOVE = 0x400, OBJFLAG_800 = 0x800,
	OBJFLAG_1000 = 0x1000, OBJFLAG_PANE_0 = 0x4000, OBJFLAG_PANE_1 = 0x8000, 
	OBJFLAG_PANES = OBJFLAG_PANE_0 | OBJFLAG_PANE_1
};

class SceneObject: public SceneHotspot {
private:
	Visage _visageImages;

	int getNewFrame();
	void animEnded();
	int changeFrame();
	bool isNoMover() const { return !_mover || (_field6E > 0); }
public:
	uint32 _updateStartFrame;
	uint32 _walkStartFrame;
	Common::Point _field2E;
	int _percent;
	int _priority;
	int _angle;
	uint32 _flags;
	int _xs, _xe;
	Rect _paneRects[2];
	int _visage;
	SceneObjectWrapper *_objectWrapper;
	int _strip;
	AnimateMode  _animateMode;
	int _frame;
	int _endFrame;
	int _field68;
	int _frameChange;
	int _numFrames;
	int _field6E;
	EventHandler *_mover;
	Common::Point _moveDiff;
	int _field7A;
	Action *_endAction;
	uint32 _regionBitList;
public:
	SceneObject();
	virtual ~SceneObject();

	void setPosition(const Common::Point &p, int yDiff = 0);
	void setStrip(int frameNum);
	void setStrip2(int frameNum);
	void setZoom(int percent);
	void changeZoom(int percent);
	void setFrame(int frameNum);
	void setFrame2(int frameNum);
	void setPriority(int priority);
	void setPriority2(int priority);
	void setVisage(int visage);
	void setObjectWrapper(SceneObjectWrapper *objWrapper);
	void addMover(ObjectMover *mover, ...);
	void getHorizBounds();
	int checkRegion(const Common::Point &pt);
	void animate(AnimateMode animMode, ...);
	SceneObject *clone() const;
	void checkAngle(const SceneObject *obj);
	void flag100();
	void unflag100();
	int getSpliceArea(const SceneObject *obj);
	int getFrameCount();

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "SceneObject"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void process(Event &event) { event.handled = true; }
	virtual void dispatch();
	virtual void calcAngle(const Common::Point &pt);
	virtual void removeObject();
	virtual GfxSurface getFrame();
	virtual void reposition();
	virtual void draw();
	virtual void proc19() {}
	virtual void updateScreen();
};

class SceneText: public SceneObject {
public:
	int _fontNumber;
	int _width;
	TextAlign _textMode;
	int _colour1;
	int _colour2;
	int _colour3;
	GfxSurface _textSurface;
public:
	SceneText();
	~SceneText();

	void setup(const Common::String &msg);

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "SceneText"; }
	virtual GfxSurface getFrame() { return _textSurface; }
};

class Player: public SceneObject {
public:
	bool _canWalk;
	bool _uiEnabled;
	int _field8C;
public:
	Player(): SceneObject() {}

	virtual Common::String getClassName() { return "Player"; }
	virtual void synchronise(Serialiser &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void process(Event &event);

	void disableControl();
	void enableControl();
};

/*--------------------------------------------------------------------------*/

class LineSliceSet {
public:
	Common::Array<LineSlice> items;

	void load(int size, const byte *srcP) {
		for (int i = 0; i < size; ++i, srcP += 4)
			items.push_back(LineSlice(READ_LE_UINT16(srcP), READ_LE_UINT16(srcP + 2)));
	}
	void load2(int size, ...) {
		va_list va;
		va_start(va, size);

		while (size-- > 0) {
			int xs = va_arg(va, int);
			int xe = va_arg(va, int);
			items.push_back(LineSlice(xs, xe));
		}			
	}

	void add(LineSlice &slice) { items.push_back(slice); }
	void add(int xs, int xe) { items.push_back(LineSlice(xs, xe)); }
	static LineSliceSet mergeSlices(const LineSliceSet &set1, LineSliceSet &set2);
};

class Region {
public:
	int _regionSize;
	int _regionId;
	Rect _bounds;
	Common::Array<LineSliceSet> _ySlices;
public:
	Region() { _regionSize = 0; _regionId = 0; }
	Region(int resNum, int rlbNum, ResourceType ctlType = RES_CONTROL);

	bool contains(const Common::Point &pt);
	bool empty() const;
	void clear();
	void setRect(const Rect &r);
	void setRect(int xs, int ys, int xe, int ye);
	const LineSliceSet &getLineSlices(int yp);
	LineSliceSet sectPoints(int yp, const LineSliceSet &sliceSet);
	void draw();
	void uniteLine(int yp, LineSliceSet &sliceSet);


	static LineSliceSet mergeSlices(const LineSliceSet &set1, const LineSliceSet &set2);
};

class SceneRegions: public List<Region> {
public:
	void load(int sceneNum);

	int indexOf(const Common::Point &pt);
};

class SceneObjectList: public SavedObject {
private:
	void checkIntersection(Common::Array<SceneObject *> &ObjList, uint ObjIndex, int PaneNum);
	void sortList(Common::Array<SceneObject *> &ObjList);

	List<SceneObject *> _objList;
public:
	SceneObjectList() {}

	virtual Common::String getClassName() { return "SceneObjectList"; }
	virtual void synchronise(Serialiser &s);

	void draw();
	void activate();
	static void deactivate();

	typedef void (*EventHandlerFn)(EventHandler *fn);
	void recurse(EventHandlerFn Fn) {
		// Loop through each object
		for (List<SceneObject *>::iterator i = _objList.begin(); i != _objList.end(); ) {
			SceneObject *o = *i;
			++i;
			Fn(o);
		}
	}
	List<SceneObject *>::iterator begin() { return _objList.begin(); }
	List<SceneObject *>::iterator end() { return _objList.end(); }
	bool contains(SceneObject *sceneObj) { return _objList.contains(sceneObj); }
	void push_back(SceneObject *sceneObj) { _objList.push_back(sceneObj); }
	void push_front(SceneObject *sceneObj) { _objList.push_front(sceneObj); }
	void remove(SceneObject *sceneObj) { _objList.remove(sceneObj); }
};

class ScenePriorities: public List<Region> {
public:
	int _resNum;
	int _field14;
	int _field16;
	Region _defaultPriorityRegion;
public:
	void load(int resNum);

	Region *find(int priority);
};

/*--------------------------------------------------------------------------*/

class GameSoundHandler {
public:
	void proc1() {
		warning("TODO: GameSoundHandler::proc1");
	}
	void proc5(int v) {
		warning("TODO: GameSoundHandler::proc5");
	}
	void proc11(int v1, int v2, int v3, int v4) {
		warning("TODO: GameSoundHandler::proc11");
	}
	int proc12() {
		// TODO
		return -1;
	}
	void proc2(int v) {
		// TODO
	}
	int proc3() {
		return 0;
	}
	void setVolume(int volume) {
		warning("TODO GameSoundHandler::setVolume");
	}
	void startSound(int soundNum) {
		warning("TODO GameSoundHandler::startSound");
	}
};

class SoundHandler: public EventHandler {
public:
	GameSoundHandler _sound;
	Action *_action;
	int _field280;
public:
	SoundHandler();
	~SoundHandler();

	void startSound(int soundNum, Action *action = NULL, int volume = 127);
	void proc1(Action *action) {
		proc11(0, 5, 10, 1, action);
	}
	void proc2(int v) {
		warning("TODO: SoundHandler::proc2");
	}
	void proc3() {
		warning("TODO: SoundHandler::proc5");
	}
	void proc4() {
		_sound.proc1();
	}
	void proc5(int v) {
		_sound.proc5(v);
	}
	void proc11(int v1, int v2, int v3, int v4, Action *action) {
		if (action)
			_action = action;

		_sound.proc11(v1, v2, v3, v4);
	}
	void setVolume(int volume) { _sound.setVolume(volume); }

	virtual Common::String getClassName() { return "SoundHandler"; }
	virtual void dispatch();
};

/*--------------------------------------------------------------------------*/

class SceneItemList: public List<SceneItem *> {
public:
	void addItems(SceneItem *first, ...);
};

/*--------------------------------------------------------------------------*/

class RegionSupportRec {
public:
	int _yp;
	int _xp;
	int _xDiff;
	int _yDiff;
	int _xDirection;
	int _halfDiff;
	int _yDiff2;

	void process();
};

#define PROCESS_LIST_SIZE 100

class WalkRegion: public Region {
private:
	static RegionSupportRec _processList[PROCESS_LIST_SIZE];
	void loadProcessList(byte *dataP, int dataSize, int &dataIndex, int &regionHeight);
	int process1(int idx, byte *dataP, int dataSize);
	void process2(int dataIndex, int x1, int y1, int x2, int y2);
	void process3(int yp, int dataCount, int &idx1, int &idx2);
	void process4(int yp, int idx1, int idx2, int &count);
	void process5(int idx1, int idx2);
	void loadRecords(int yp, int size, int processIndex);
	void process6(RegionSupportRec &rec);
public:
	Common::Point _pt;
	int _idxListIndex;
	int _idxList2Index;
public:
	void loadRegion(byte *dataP, int size);
};

class WRField18 {
public:
	Common::Point _pt1, _pt2;
	int _v;
public:
	void load(byte *data);
};

class WalkRegions {
public:
	int _resNum;
	RouteEnds _routeEnds;
	Common::Array<WalkRegion> _regionList;
	Common::Array<WRField18> _field18;
	Common::Array<int> _idxList;
	Common::Array<int> _idxList2;
public:
	WalkRegions() { _resNum = -1; }

	void clear();
	void load(int sceneNum);
	int indexOf(const Common::Point &pt, List<int> *indexList = NULL);
	WalkRegion &operator[](int idx) {
		assert((idx >= 1) && (idx <= (int)_regionList.size()));
		return _regionList[idx - 1];
	}
};

/*--------------------------------------------------------------------------*/

class GameHandler: public EventHandler {
public:
	RefCounter _lockCtr;
	RefCounter _waitCtr;
	int _nextWaitCtr;
	int _field14;
public:
	GameHandler(); 
	virtual ~GameHandler();
	void execute();

	virtual void synchronise(Serialiser &s);
	virtual Common::String getClassName() { return "GameHandler"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL) {}
	virtual void dispatch() {}
};

class SceneHandler: public GameHandler {
public:
	int _saveGameSlot;
	int _loadGameSlot;
	int _delayTicks;
	Common::String _saveName;
public:
	SceneHandler();
	void registerHandler();

	virtual Common::String getClassName() { return "SceneHandler"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void process(Event &event);
	virtual void dispatch();

	static void handleListener(EventHandler *obj);
	static void saveListener(Serialiser &ser);
};

/*--------------------------------------------------------------------------*/

class Game {
private:
	List<GameHandler *> _handlers;

	static bool notLockedFn(GameHandler *g);
	void restart();
	void handleSaveLoad(bool saveFlag, int &saveSlot, Common::String &saveName);
public:
	void addHandler(GameHandler *entry) { _handlers.push_back(entry); }
	void removeHandler(GameHandler *entry) { _handlers.remove(entry); }

	void execute();
	void restartGame();
	void saveGame();
	void restoreGame();
	void quitGame();
	void endGame(int resNum, int lineNum);
};

} // End of namespace tSage

#endif
