/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */


#include "stdafx.h"

#include "common/config-manager.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"
#include "scumm/smush/smush_player.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

namespace Scumm {

#define OPCODE(x)	{ &ScummEngine_v7he::x, #x }

void ScummEngine_v7he::setupOpcodes() {
	static const OpcodeEntryV7he opcodes[256] = {
		/* 00 */
		OPCODE(o6_pushByte),
		OPCODE(o6_pushWord),
		OPCODE(o6_pushByteVar),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayRead),
		OPCODE(o6_wordArrayRead),
		/* 08 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayIndexedRead),
		OPCODE(o6_wordArrayIndexedRead),
		/* 0C */
		OPCODE(o6_dup),
		OPCODE(o6_not),
		OPCODE(o6_eq),
		OPCODE(o6_neq),
		/* 10 */
		OPCODE(o6_gt),
		OPCODE(o6_lt),
		OPCODE(o6_le),
		OPCODE(o6_ge),
		/* 14 */
		OPCODE(o6_add),
		OPCODE(o6_sub),
		OPCODE(o6_mul),
		OPCODE(o6_div),
		/* 18 */
		OPCODE(o6_land),
		OPCODE(o6_lor),
		OPCODE(o6_pop),
		OPCODE(o6_invalid),
		/* 1C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 20 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 24 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 28 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 2C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 30 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 34 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 38 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 3C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 40 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_writeByteVar),
		OPCODE(o6_writeWordVar),
		/* 44 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayWrite),
		OPCODE(o6_wordArrayWrite),
		/* 48 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayIndexedWrite),
		OPCODE(o6_wordArrayIndexedWrite),
		/* 4C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteVarInc),
		OPCODE(o6_wordVarInc),
		/* 50 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayInc),
		OPCODE(o6_wordArrayInc),
		/* 54 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteVarDec),
		OPCODE(o6_wordVarDec),
		/* 58 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayDec),
		OPCODE(o6_wordArrayDec),
		/* 5C */
		OPCODE(o6_if),
		OPCODE(o6_ifNot),
		OPCODE(o6_startScript),
		OPCODE(o6_startScriptQuick),
		/* 60 */
		OPCODE(o6_startObject),
		OPCODE(o6_drawObject),
		OPCODE(o6_drawObjectAt),
		OPCODE(o6_invalid),
		/* 64 */
		OPCODE(o6_invalid),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_endCutscene),
		/* 68 */
		OPCODE(o6_cutscene),
		OPCODE(o6_stopMusic),
		OPCODE(o6_freezeUnfreeze),
		OPCODE(o7_cursorCommand),
		/* 6C */
		OPCODE(o6_breakHere),
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_setClass),
		OPCODE(o6_getState),
		/* 70 */
		OPCODE(o6_setState),
		OPCODE(o6_setOwner),
		OPCODE(o6_getOwner),
		OPCODE(o6_jump),
		/* 74 */
		OPCODE(o7_startSound),
		OPCODE(o6_stopSound),
		OPCODE(o6_startMusic),
		OPCODE(o6_stopObjectScript),
		/* 78 */
		OPCODE(o6_panCameraTo),
		OPCODE(o6_actorFollowCamera),
		OPCODE(o6_setCameraAt),
		OPCODE(o6_loadRoom),
		/* 7C */
		OPCODE(o6_stopScript),
		OPCODE(o6_walkActorToObj),
		OPCODE(o6_walkActorTo),
		OPCODE(o6_putActorAtXY),
		/* 80 */
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_faceActor),
		OPCODE(o6_animateActor),
		OPCODE(o6_doSentence),
		/* 84 */
		OPCODE(o7_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o7_getActorRoom),
		OPCODE(o6_getObjectX),
		OPCODE(o6_getObjectY),
		OPCODE(o6_getObjectOldDir),
		/* 90 */
		OPCODE(o6_getActorWalkBox),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_findInventory),
		OPCODE(o6_getInventoryCount),
		/* 94 */
		OPCODE(o6_getVerbFromXY),
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o6_setObjectName),
		/* 98 */
		OPCODE(o6_isSoundRunning),
		OPCODE(o6_setBoxFlags),
		OPCODE(o6_invalid),
		OPCODE(o7_resourceRoutines),
		/* 9C */
		OPCODE(o6_roomOps),
		OPCODE(o6_actorOps),
		OPCODE(o6_verbOps),
		OPCODE(o6_getActorFromXY),
		/* A0 */
		OPCODE(o6_findObject),
		OPCODE(o6_pseudoRoom),
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getVerbEntrypoint),
		/* A4 */
		OPCODE(o6_arrayOps),
		OPCODE(o6_saveRestoreVerbs),
		OPCODE(o6_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o6_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o6_invalid),
		OPCODE(o6_isAnyOf),
		OPCODE(o7_quitPauseRestart),
		OPCODE(o6_isActorInBox),
		/* B0 */
		OPCODE(o6_delay),
		OPCODE(o6_delaySeconds),
		OPCODE(o6_delayMinutes),
		OPCODE(o6_stopSentence),
		/* B4 */
		OPCODE(o6_printLine),
		OPCODE(o6_printCursor),
		OPCODE(o6_printDebug),
		OPCODE(o6_printSystem),
		/* B8 */
		OPCODE(o6_printActor),
		OPCODE(o6_printEgo),
		OPCODE(o6_talkActor),
		OPCODE(o6_talkEgo),
		/* BC */
		OPCODE(o6_dimArray),
		OPCODE(o6_dummy),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		/* C0 */
		OPCODE(o6_dim2dimArray),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C4 */
		OPCODE(o6_abs),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distObjectPt),
		OPCODE(o6_distPtPt),
		/* C8 */
		OPCODE(o6_kernelGetFunctions),
		OPCODE(o7_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* D0 */
		OPCODE(o6_getDateTime),
		OPCODE(o6_stopTalking),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_invalid),
		/* D4 */
		OPCODE(o6_shuffle),
		OPCODE(o6_jumpToScript),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		/* D8 */
		OPCODE(o6_isRoomScriptRunning),
		OPCODE(o6_closeFile),
		OPCODE(o6_openFile),
		OPCODE(o6_readFile),
		/* DC */
		OPCODE(o6_writeFile),
		OPCODE(o6_findAllObjects),
		OPCODE(o6_deleteFile),
		OPCODE(o6_rename),
		/* E0 */
		OPCODE(o6_soundOps),
		OPCODE(o6_getPixel),
		OPCODE(o6_localizeArray),
		OPCODE(o6_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o6_seekFilePos),
		OPCODE(o6_redimArray),
		OPCODE(o6_readFilePos),
		/* EC */
		OPCODE(o6_invalid),
		OPCODE(o7_unknownED),
		OPCODE(o7_stringLen),
		OPCODE(o7_unknownEF),
		/* F0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o7_readINI),
		/* F4 */
		OPCODE(o7_writeINI),
		OPCODE(o7_unknownF5),
		OPCODE(o7_unknownF6),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o6_invalid),
		OPCODE(o7_setFilePath),
		OPCODE(o7_unknownFA),
		OPCODE(o7_polygonOps),
		/* FC */
		OPCODE(o7_polygonHit),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodesV7he = opcodes;
}

void ScummEngine_v7he::executeOpcode(byte i) {
	OpcodeProcV7he op = _opcodesV7he[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v7he::getOpcodeDesc(byte i) {
	return _opcodesV7he[i].desc;
}

void ScummEngine_v7he::arrrays_unk2(int dst, int src, int len2, int len) {
	int edi, value;
	int i = 0;

	if (len == -1) {
		len = resStrLen(getStringAddress(src));
		len2 = 0;
	}

	edi = resStrLen(getStringAddress(dst));

	len -= len2;
	len++;

	while (i < len) {
		writeVar(0, src);
		value = readArray(0, 0, len2 + i);
		writeVar(0, dst);
		writeArray(0, 0, edi + i, value);
		i++;
	}

	writeArray(0, 0, edi + i, 0);
}

void ScummEngine_v7he::o7_cursorCommand() {
	int a, i;
	int args[16];
	int subOp = fetchScriptByte();

	switch (subOp) {
	case 0x90:		// SO_CURSOR_ON Turn cursor on
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case 0x91:		// SO_CURSOR_OFF Turn cursor off
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case 0x92:		// SO_USERPUT_ON
		_userPut = 1;
		break;
	case 0x93:		// SO_USERPUT_OFF
		_userPut = 0;
		break;
	case 0x94:		// SO_CURSOR_SOFT_ON Turn soft cursor on
		_cursor.state++;
		if (_cursor.state > 1)
			error("Cursor state greater than 1 in script");
		verbMouseOver(0);
		break;
	case 0x95:		// SO_CURSOR_SOFT_OFF Turn soft cursor off
		_cursor.state--;
		verbMouseOver(0);
		break;
	case 0x96:		// SO_USERPUT_SOFT_ON
		_userPut++;
		break;
	case 0x97:		// SO_USERPUT_SOFT_OFF
		_userPut--;
		break;
	case 0x99: 		// SO_CURSOR_IMAGE Set cursor image
		_Win32ResExtractor->setCursor(pop()); 				/* Difference */
		break;
	case 0x9A:		// SO_CURSOR_HOTSPOT Set cursor hotspot
		a = pop();
		setCursorHotspot(pop(), a);
		break;
	case 0x9C:		// SO_CHARSET_SET
		initCharset(pop());
		break;
	case 0x9D:		// SO_CHARSET_COLOR
		getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
		break;
	case 0xD6:		// SO_CURSOR_TRANSPARENT Set cursor transparent color
		setCursorTransparency(pop());
		break;
	default:
		error("o7_cursorCommand: default case %x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void ScummEngine_v7he::o7_startSound() {
	byte op;
	op = fetchScriptByte();

	switch (op) {
	case 9:
		_heSndLoop |= 4;
		break;
	case 23:
		debug(1,"o7_startSound: case 29 (%d, %d, %d)", pop(), pop(), pop());
		break;
	case 56:
		_heSndLoop |= 2;
		break;
	case 164:
		_heSndLoop |= 2;
		break;
	case 224:
		_heSndSoundFreq = pop();
		break;

	case 230:
		_heSndChannel = pop();
		break;

	case 231:
		_heSndOffset = pop();
		break;

	case 232:
		_heSndSoundId = pop();
		_heSndOffset = 0;
		_heSndSoundFreq = 11025;
		_heSndChannel = VAR(VAR_MUSIC_CHANNEL);
		break;

	case 245:
		_heSndLoop |= 1;
		break;

	case 255:
		// _sound->addSoundToQueue(_heSndSoundId, _heSndOffset, _heSndChannel, _heSndLoop);
		_sound->addSoundToQueue(_heSndSoundId, _heSndOffset);
		debug(2, "o7_startSound stub (%d, %d, %d, %d)", _heSndSoundId, _heSndOffset, _heSndChannel, _heSndLoop);
		_heSndLoop = 0;
		break;

	default:
		error("o7_startSound invalid case %d", op);
	}
}

void ScummEngine_v7he::o7_pickupObject() {
	int obj, room;

	room = pop();
	obj = pop();
	if (room == 0)
		room = getObjectRoom(obj);

	addObjectToInventory(obj, room);
	putOwner(obj, VAR(VAR_EGO));
	if (_heversion <= 70) {
		putClass(obj, kObjectClassUntouchable, 1);
		putState(obj, 1);
		markObjectRectAsDirty(obj);
		clearDrawObjectQueue();
	}
	runInventoryScript(obj);									/* Difference */
}

void ScummEngine_v7he::o7_getActorRoom() {
	int act = pop();

	if (act < _numActors) {
		Actor *a = derefActor(act, "o7_getActorRoom");
		push(a->room);
	} else
		push(getObjectRoom(act));
}

void ScummEngine_v7he::o7_resourceRoutines() {
	int resid, op;
	op = fetchScriptByte();

	switch (op) {
	case 100:		// SO_LOAD_SCRIPT
		resid = pop();
		ensureResourceLoaded(rtScript, resid);
		break;
	case 101:		// SO_LOAD_SOUND
		resid = pop();
		ensureResourceLoaded(rtSound, resid);
		break;
	case 102:		// SO_LOAD_COSTUME
		resid = pop();
		ensureResourceLoaded(rtCostume, resid);
		break;
	case 103:		// SO_LOAD_ROOM
		resid = pop();
		ensureResourceLoaded(rtRoom, resid);
		break;
	case 104:		// SO_NUKE_SCRIPT
		resid = pop();
		setResourceCounter(rtScript, resid, 0x7F);
		break;
	case 105:		// SO_NUKE_SOUND
		resid = pop();
		setResourceCounter(rtSound, resid, 0x7F);
		break;
	case 106:		// SO_NUKE_COSTUME
		resid = pop();
		setResourceCounter(rtCostume, resid, 0x7F);
		break;
	case 107:		// SO_NUKE_ROOM
		resid = pop();
		setResourceCounter(rtRoom, resid, 0x7F);
		break;
	case 108:		// SO_LOCK_SCRIPT
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		lock(rtScript, resid);
		break;
	case 109:		// SO_LOCK_SOUND
		resid = pop();
		lock(rtSound, resid);
		break;
	case 110:		// SO_LOCK_COSTUME
		resid = pop();
		lock(rtCostume, resid);
		break;
	case 111:		// SO_LOCK_ROOM
		resid = pop();
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		lock(rtRoom, resid);
		break;
	case 112:		// SO_UNLOCK_SCRIPT
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		unlock(rtScript, resid);
		break;
	case 113:		// SO_UNLOCK_SOUND
		resid = pop();
		unlock(rtSound, resid);
		break;
	case 114:		// SO_UNLOCK_COSTUME
		resid = pop();
		unlock(rtCostume, resid);
		break;
	case 115:		// SO_UNLOCK_ROOM
		resid = pop();
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		unlock(rtRoom, resid);
		break;
	case 116:
		break;
	case 117:		// SO_LOAD_CHARSET
		resid = pop();
		loadCharset(resid);
		break;
	case 118:		// SO_NUKE_CHARSET
		resid = pop();
		nukeCharset(resid);
		break;
	case 119:		// SO_LOAD_OBJECT
		{
			int room, obj = popRoomAndObj(&room);
			loadFlObject(obj, room);
			break;
		}
	case 120: 					/* queue for load */
	case 121:
	case 122:
	case 123:
	case 203:
		debug(5,"stub queueload (%d) resource %d", op, pop());
		break;
	case 159:
		resid = pop();
		unlock(rtImage, resid);
		break;
	case 192:
		resid = pop();
		nukeResource(rtImage, resid);
		break;
	case 201:
		resid = pop();
		ensureResourceLoaded(rtImage, resid);
		break;
	case 202:
		resid = pop();
		lock(rtImage, resid);
		break;
	case 233:
		resid = pop();
		debug(5,"stub o7_resourceRoutines lock object %d", resid);
		break;
	case 235:
		resid = pop();
		debug(5,"stub o7_resourceRoutines unlock object %d", resid);
		break;
	default:
		debug(1,"o7_resourceRoutines: default case %d", op);
	}
}

void ScummEngine_v7he::o7_quitPauseRestart() {
	byte subOp = fetchScriptByte();
	int par1;

	switch (subOp & 0xff) {
	case 158:		// SO_RESTART
		restart();
		break;
	case 160:
		// FIXME: check
		shutDown();
		break;
	case 250:
		par1 = pop();
		warning("stub: o7_quitPauseRestart subOpcode %d", subOp);
		break;
	case 253:
		par1 = pop();
		warning("stub: o7_quitPauseRestart subOpcode %d", subOp);
	case 244:		// SO_QUIT
		shutDown();
		break;
	case 251:
	case 252:
		warning("stub: o7_quitPauseRestart subOpcode %d", subOp);
		break;
	default:
		warning("o7_quitPauseRestart invalid case %d", subOp);
	}
}

void ScummEngine_v7he::o7_unknownED() {
	int array, pos, len;
	int chr, result = 0;

	len = pop();
	pos = pop();
	array = pop();

	if (len == -1) {
		pos = 0;
		len = resStrLen(getStringAddress(array));
	}

	writeVar(0, array);
	while (pos <= len) {
		chr = readArray(0, 0, pos);
		result += _charset->getCharWidth(chr);
		pos++;
	}

	push(result);
	debug(1,"stub o7_unknownED");
}

void ScummEngine_v7he::o7_kernelSetFunctions() {
	int args[29];
	int num;
	Actor *a;

	num = getStackList(args, ARRAYSIZE(args));

	debug(1, "o7_kernelSetFunctions: case %d (param count %d)", args[0], num);
	switch (args[0]) {
	case 1:
		// Used to restore images when decorating cake in
		// Fatty Bear's Birthday Surprise
		virtScreenLoad(args[1], args[2], args[3], args[4], args[5]);
		break;
	case 20:
		// Clear/stop unknown animation queue
		break;
	case 21:
		_skipDrawObject = 1;
		break;
	case 22:
		_skipDrawObject = 0;
		break;
	case 23:
		_charset->clearCharsetMask();
		_fullRedraw = 1;
		break;
	case 24:
		_skipProcessActors = 1;
		_fullRedraw = 1;
		break;
	case 25:
		_skipProcessActors = 0;
		_fullRedraw = 1;
		break;
	case 30:
		a = derefActor(args[1], "o7_kernelSetFunctions: 30");
		a->clipOverride.bottom = args[2];
		break;
	case 714:
		break;
	default:
		error("o7_kernelSetFunctions: default case %d (param count %d)", args[0], num);
	}
}

void ScummEngine_v7he::o7_stringLen() {
	int id, len;
	byte *addr;

	id = pop();

	addr = getStringAddress(id);
	if (!addr)
		error("o72_stringLen: Reference to zeroed array pointer (%d)", id);

	len = resStrLen(getStringAddress(id));
	push(len);
}

void ScummEngine_v7he::o7_unknownEF() {
	int dst, size;
	int b = pop();
	int a = pop();
	int src = pop();

	size = b - a + 2;

	writeVar(0, 0);
	defineArray(0, kStringArray, 0, size);
	writeArray(0, 0, 0, 0);

	dst = readVar(0);

	arrrays_unk2(dst, src, a, b);

	push(dst);
	debug(1,"stub o7_unknownEF");
}

void ScummEngine_v7he::o7_readINI() {
	int len;
	int type;
	int retval;
	byte option[256];

	// we pretend that we don't have .ini file
	addMessageToStack(_scriptPointer, option, sizeof(option));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	type = pop();
	switch (type) {
	case 1: // number
		if (!strcmp((char *)option, "ReadPagesAutomatically"))
			push(1);
		else if (!strcmp((char *)option, "NoPrinting"))
			push(1);
		else
			push(0);
		break;
	case 2: // string
		writeVar(0, 0);
		defineArray(0, kStringArray, 0, 0);
		retval = readVar(0);
		writeArray(0, 0, 0, 0);
		push(retval); // var ID string
		break;
	default:
		error("o7_readINI: default type %d", type);
	}
}

void ScummEngine_v7he::o7_writeINI() {
	int type, value;
	byte option[256], option2[256];
	int len;
	
	type = pop();
	value = pop();

	addMessageToStack(_scriptPointer, option, sizeof(option));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	switch (type) {
	case 1: // number
		debug(1, "o7_writeINI: %s set to %d", option, value);
		break;
	case 2: // string
		addMessageToStack(_scriptPointer, option2, sizeof(option2));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
		debug(1, "o7_writeINI: %s set to %s", option, option2);
		break;
	default:
		error("o7_writeINI: default type %d", type);
	}
}

void ScummEngine_v7he::o7_unknownF5() {
	int chr, max;
	int array, len, pos, result = 0;
	max = pop();
	pos = pop();
	array = pop();

	len = resStrLen(getStringAddress(array));

	writeVar(0, array);
	while (pos <= len) {
		chr = readArray(0, 0, pos);
		result += _charset->getCharWidth(chr);
		if (result >= max) {
			push(pos);
			return;
		}
		pos++;
	}

	push(len);
	debug(1,"stub o7_unknownF5 (%d)", result);
}

void ScummEngine_v7he::o7_unknownF6() {
	int len, edi, pos, value, id;
	value = pop();
	edi = pop();
	pos = pop();
	id = pop();

	if (edi >= 0) {
		len = resStrLen(getStringAddress(id));
		if (len < edi)
			edi = len;
	} else {
		edi = 0;
	}

	if (pos < 0)
		pos = 0;

	writeVar(0, id);
	if (edi > pos) {
		while (edi >= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos++;
		}
	} else {
		while (edi <= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos--;
		}
	}

	push(-1);
	debug(1,"stub o7_unknownF6");
}

void ScummEngine_v7he::o7_setFilePath() {
	// File related
	int len;
	byte filename[100];

	addMessageToStack(_scriptPointer, filename, sizeof(filename));

	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	debug(1,"stub o7_setFilePath(%s)", filename);
}

void ScummEngine_v7he::o7_unknownFA() {
	int num = fetchScriptByte();
	int len = resStrLen(_scriptPointer);
	debug(1,"stub o7_unknownFA(%d, \"%s\")", num, _scriptPointer);
	_scriptPointer += len + 1;
}

void ScummEngine_v7he::o7_polygonOps() {
	byte b;
	b = fetchScriptByte();
	int vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y;
	int id;
	int fromId, toId;

	switch (b) {
	case 246:
	case 248:
		vert4y = pop();
		vert4x = pop();
		vert3y = pop();
		vert3x = pop();
		vert2y = pop();
		vert2x = pop();
		vert1y = pop();
		vert1x = pop();
		id = pop();

		polygonStore(id, (b == 248), vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, 
					 vert4x, vert4y);
		break;
	case 247:
		toId = pop();
		fromId = pop();

		polygonErase(fromId, toId);
		break;
	}
}

void ScummEngine::polygonStore(int id, bool flag, int vert1x, int vert1y, int vert2x, 
							int vert2y, int vert3x, int vert3y, int vert4x, int vert4y) {
	int i;

	for (i = 0; i < _WizNumPolygons; i++)
		if (_WizPolygons[i].id == 0)
			break;
	
	if (i == _WizNumPolygons) {
		error("ScummEngine::polygonStore: out of polygon slot, max = %d", 
			  _WizNumPolygons);
	}

	_WizPolygons[i].vert[0].x = vert1x;
	_WizPolygons[i].vert[0].y = vert1y;
	_WizPolygons[i].vert[1].x = vert2x;
	_WizPolygons[i].vert[1].y = vert2y;
	_WizPolygons[i].vert[2].x = vert3x;
	_WizPolygons[i].vert[2].y = vert3y;
	_WizPolygons[i].vert[3].x = vert4x;
	_WizPolygons[i].vert[3].y = vert4y;
	_WizPolygons[i].vert[4].x = vert1x;
	_WizPolygons[i].vert[4].y = vert1y;
	_WizPolygons[i].id = id;
	_WizPolygons[i].numVerts = 5;
	_WizPolygons[i].flag = flag;

	_WizPolygons[i].bound.left = 10000;
	_WizPolygons[i].bound.top = 10000;
	_WizPolygons[i].bound.right = -10000;
	_WizPolygons[i].bound.bottom = -10000;

	for (int j = 0; j < 5; j++) {
		_WizPolygons[i].bound.left = MIN(_WizPolygons[i].bound.left, _WizPolygons[i].vert[j].x);
		_WizPolygons[i].bound.top = MIN(_WizPolygons[i].bound.top, _WizPolygons[i].vert[j].y);
		_WizPolygons[i].bound.right = MAX(_WizPolygons[i].bound.right, _WizPolygons[i].vert[j].x);
		_WizPolygons[i].bound.bottom = MAX(_WizPolygons[i].bound.bottom, _WizPolygons[i].vert[j].y);
	}
}

void ScummEngine_v7he::polygonErase(int fromId, int toId) {
	for (int i = 0; i < _WizNumPolygons; i++) {
		if (_WizPolygons[i].id >= fromId && _WizPolygons[i].id <= toId)
			memset(&_WizPolygons[i], 0, sizeof(WizPolygon));
	}
}

void ScummEngine_v7he::o7_polygonHit() {
	int y = pop();
	int x = pop();

	push(polygonHit(0, x, y));
}

int ScummEngine_v7he::polygonHit(int id, int x, int y) {
	for (int i = 0; i < _WizNumPolygons; i++) {
		if ((!id || _WizPolygons[i].id == id) && _WizPolygons[i].bound.contains(x, y)) {
			if (polygonContains(_WizPolygons[i], x, y)) {
				return _WizPolygons[i].id;
			}
		}
	}

	return 0;
}

bool ScummEngine_v7he::polygonDefined(int id) {
	for (int i = 0; i < _WizNumPolygons; i++)
		if (_WizPolygons[i].id == id)
			return true;

	return false;
}

bool ScummEngine_v7he::polygonContains(WizPolygon &pol, int x, int y) {
	int pi = pol.numVerts - 1;
	bool diry = (y < pol.vert[pi].y);
	bool curdir;
	bool r = false;

	for (int i = 0; i < pol.numVerts; i++) {
		curdir = (y <= pol.vert[i].y);

		if (curdir != diry) {
			if (((pol.vert[pi].y - pol.vert[i].y) * (pol.vert[i].x - x) <=
				 (pol.vert[pi].x - pol.vert[i].x) * (pol.vert[i].y - y)) == diry)
				r = !r;
		}

		pi = i;
		diry = curdir;
	}

	return r;
}

} // End of namespace Scumm
