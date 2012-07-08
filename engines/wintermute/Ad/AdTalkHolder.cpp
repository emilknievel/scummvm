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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Ad/AdTalkHolder.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/wintermute.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdTalkHolder, false)

//////////////////////////////////////////////////////////////////////////
CAdTalkHolder::CAdTalkHolder(CBGame *inGame): CAdObject(inGame) {
	_sprite = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdTalkHolder::~CAdTalkHolder() {
	delete _sprite;
	_sprite = NULL;

	for (int i = 0; i < _talkSprites.GetSize(); i++)
		delete _talkSprites[i];
	_talkSprites.RemoveAll();

	for (int i = 0; i < _talkSpritesEx.GetSize(); i++)
		delete _talkSpritesEx[i];
	_talkSpritesEx.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////
CBSprite *CAdTalkHolder::getTalkStance(const char *stance) {
	CBSprite *ret = NULL;


	// forced stance?
	if (_forcedTalkAnimName && !_forcedTalkAnimUsed) {
		_forcedTalkAnimUsed = true;
		delete _animSprite;
		_animSprite = new CBSprite(Game, this);
		if (_animSprite) {
			HRESULT res = _animSprite->loadFile(_forcedTalkAnimName);
			if (FAILED(res)) {
				Game->LOG(res, "CAdTalkHolder::GetTalkStance: error loading talk sprite (object:\"%s\" sprite:\"%s\")", _name, _forcedTalkAnimName);
				delete _animSprite;
				_animSprite = NULL;
			} else return _animSprite;
		}
	}


	if (stance != NULL) {
		// search special talk stances
		for (int i = 0; i < _talkSpritesEx.GetSize(); i++) {
			if (scumm_stricmp(_talkSpritesEx[i]->_name, stance) == 0) {
				ret = _talkSpritesEx[i];
				break;
			}
		}
		if (ret == NULL) {
			// serach generic talk stances
			for (int i = 0; i < _talkSprites.GetSize(); i++) {
				if (scumm_stricmp(_talkSprites[i]->_name, stance) == 0) {
					ret = _talkSprites[i];
					break;
				}
			}
		}
	}

	// not a valid stance? get a random one
	if (ret == NULL) {
		if (_talkSprites.GetSize() < 1) ret = _sprite;
		else {
			// TODO: remember last
			int rnd = g_wintermute->randInt(0, _talkSprites.GetSize() - 1);
			ret = _talkSprites[rnd];
		}
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSprite
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSprite") == 0) {
		stack->correctParams(1);

		CScValue *val = stack->pop();

		bool setCurrent = false;
		if (_currentSprite && _currentSprite == _sprite) setCurrent = true;

		delete _sprite;
		_sprite = NULL;

		if (val->isNULL()) {
			_sprite = NULL;
			if (setCurrent) _currentSprite = NULL;
			stack->pushBool(true);
		} else {
			const char *filename = val->getString();
			CBSprite *spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->loadFile(filename))) {
				script->runtimeError("SetSprite method failed for file '%s'", filename);
				stack->pushBool(false);
			} else {
				_sprite = spr;
				if (setCurrent) _currentSprite = _sprite;
				stack->pushBool(true);
			}
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSprite") == 0) {
		stack->correctParams(0);

		if (!_sprite || !_sprite->_filename) stack->pushNULL();
		else stack->pushString(_sprite->_filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSpriteObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSpriteObject") == 0) {
		stack->correctParams(0);

		if (!_sprite) stack->pushNULL();
		else stack->pushNative(_sprite, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddTalkSprite") == 0) {
		stack->correctParams(2);

		const char *filename = stack->pop()->getString();
		bool Ex = stack->pop()->getBool();

		CBSprite *spr = new CBSprite(Game, this);
		if (!spr || FAILED(spr->loadFile(filename))) {
			stack->pushBool(false);
			script->runtimeError("AddTalkSprite method failed for file '%s'", filename);
		} else {
			if (Ex) _talkSpritesEx.Add(spr);
			else _talkSprites.Add(spr);
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveTalkSprite") == 0) {
		stack->correctParams(2);

		const char *filename = stack->pop()->getString();
		bool ex = stack->pop()->getBool();

		bool setCurrent = false;
		bool setTemp2 = false;

		if (ex) {
			for (int i = 0; i < _talkSpritesEx.GetSize(); i++) {
				if (scumm_stricmp(_talkSpritesEx[i]->_filename, filename) == 0) {
					if (_currentSprite == _talkSpritesEx[i])
						setCurrent = true;
					if (_tempSprite2 == _talkSpritesEx[i])
						setTemp2 = true;
					delete _talkSpritesEx[i];
					_talkSpritesEx.RemoveAt(i);
					break;
				}
			}
		} else {
			for (int i = 0; i < _talkSprites.GetSize(); i++) {
				if (scumm_stricmp(_talkSprites[i]->_filename, filename) == 0) {
					if (_currentSprite == _talkSprites[i]) 
						setCurrent = true;
					if (_tempSprite2 == _talkSprites[i])
						setTemp2 = true;
					delete _talkSprites[i];
					_talkSprites.RemoveAt(i);
					break;
				}
			}

		}

		stack->pushBool(true);
		if (setCurrent)
			_currentSprite = _sprite;
		if (setTemp2)
			_tempSprite2 = _sprite;

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetTalkSprite") == 0) {
		stack->correctParams(2);

		const char *filename = stack->pop()->getString();
		bool ex = stack->pop()->getBool();
		bool setCurrent = false;
		bool setTemp2 = false;

		CBSprite *spr = new CBSprite(Game, this);
		if (!spr || FAILED(spr->loadFile(filename))) {
			stack->pushBool(false);
			script->runtimeError("SetTalkSprite method failed for file '%s'", filename);
		} else {

			// delete current
			if (ex) {
				for (int i = 0; i < _talkSpritesEx.GetSize(); i++) {
					if (_talkSpritesEx[i] == _currentSprite)
						setCurrent = true;
					if (_talkSpritesEx[i] == _tempSprite2)
						setTemp2 = true;
					delete _talkSpritesEx[i];
				}
				_talkSpritesEx.RemoveAll();
			} else {
				for (int i = 0; i < _talkSprites.GetSize(); i++) {
					if (_talkSprites[i] == _currentSprite) setCurrent = true;
					if (_talkSprites[i] == _tempSprite2) setTemp2 = true;
					delete _talkSprites[i];
				}
				_talkSprites.RemoveAll();
			}

			// set new
			if (ex)
				_talkSpritesEx.Add(spr);
			else 
				_talkSprites.Add(spr);
			stack->pushBool(true);

			if (setCurrent)
				_currentSprite = spr;
			if (setTemp2)
				_tempSprite2 = spr;
		}
		return S_OK;
	}

	else return CAdObject::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdTalkHolder::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("talk-holder");
		return _scValue;
	}

	else return CAdObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::scSetProperty(const char *name, CScValue *value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////
	if(strcmp(name, "Item")==0){
	    SetItem(value->getString());
	    return S_OK;
	}

	else*/ return CAdObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdTalkHolder::scToString() {
	return "[talk-holder object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::saveAsText(CBDynBuffer *buffer, int indent) {
	for (int i = 0; i < _talkSprites.GetSize(); i++) {
		if (_talkSprites[i]->_filename)
			buffer->putTextIndent(indent + 2, "TALK=\"%s\"\n", _talkSprites[i]->_filename);
	}

	for (int i = 0; i < _talkSpritesEx.GetSize(); i++) {
		if (_talkSpritesEx[i]->_filename)
			buffer->putTextIndent(indent + 2, "TALK_SPECIAL=\"%s\"\n", _talkSpritesEx[i]->_filename);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::persist(CBPersistMgr *persistMgr) {
	CAdObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_sprite));
	_talkSprites.persist(persistMgr);
	_talkSpritesEx.persist(persistMgr);

	return S_OK;
}

} // end of namespace WinterMute
