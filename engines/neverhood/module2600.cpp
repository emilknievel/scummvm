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

#include "neverhood/module2600.h"

namespace Neverhood {

static const uint32 kModule2600SoundList[] = {
	0xB288D450,
	0x90804450,
	0x99801500,
	0xB288D455,
	0x93825040,
	0
};

Module2600::Module2600(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 1) {
		createScene(4, 1);
	} else {
		createScene(0, 1);
	}

	_vm->_soundMan->addSoundList(0x40271018, kModule2600SoundList);
	_vm->_soundMan->setSoundListParams(kModule2600SoundList, true, 50, 600, 5, 150);
	_vm->_soundMan->playTwoSounds(0x40271018, 0x41861371, 0x43A2507F, 0);

}

Module2600::~Module2600() {
	_vm->_soundMan->deleteGroup(0x40271018);
}

void Module2600::createScene(int sceneNum, int which) {
	debug("Module2600::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		createNavigationScene(0x004B8608, which);
		break;
	case 1:
		createNavigationScene(0x004B8638, which);
		break;
	case 2:
		createNavigationScene(0x004B86C8, which);
		break;
	case 3:
		if (getGlobalVar(V_CREATURE_ANGRY)) {
			createNavigationScene(0x004B8758, which);
		} else {
			createNavigationScene(0x004B86F8, which);
		}
		break;
	case 4:
		createNavigationScene(0x004B87B8, which);
		break;
	case 6:
		createNavigationScene(0x004B8698, which);
		break;
	case 7:
		_vm->_soundMan->deleteGroup(0x40271018);
		createSmackerScene(0x30090001, true, true, false);
		break;
	case 8:
		_childObject = new Scene2609(_vm, this, which);
		break;
	case 1002:
		if (getGlobalVar(V_FRUIT_COUNTING_INDEX) == 1) {
			createSmackerScene(0x018C0404, true, true, false);
		} else if (getGlobalVar(V_FRUIT_COUNTING_INDEX) == 2) {
			createSmackerScene(0x018C0407, true, true, false);
		} else {
			createSmackerScene(0x818C0405, true, true, false);
		}
		if (getGlobalVar(V_FRUIT_COUNTING_INDEX) >= 2) {
			setGlobalVar(V_FRUIT_COUNTING_INDEX, 0);
		} else {
			incGlobalVar(V_FRUIT_COUNTING_INDEX, +1);
		}
		break;
	case 1003:
		createSmackerScene(0x001C0007, true, true, false);
		break;
	case 1006:
		if (getGlobalVar(V_WATER_RUNNING)) {
			createSmackerScene(0x049A1181, true, true, false);
		} else {
			createSmackerScene(0x04981181, true, true, false);
		}
		break;
	case 1008:
		if (getGlobalVar(V_WATER_RUNNING)) {
			createSmackerScene(0x42B80941, true, true, false);
		} else {
			createSmackerScene(0x42980941, true, true, false);
		}
		break;
	}
	SetUpdateHandler(&Module2600::updateScene);
	_childObject->handleUpdate();
}

void Module2600::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				createScene(1, 3);
			} else {
				leaveModule(0);
			}
			break;
		case 1:
			if (_moduleResult == 0) {
				createScene(6, 0);
			} else if (_moduleResult == 1) {
				createScene(0, 0);
			} else if (_moduleResult == 2) {
				createScene(2, 1);
			} else if (_moduleResult == 3) {
				createScene(3, 0);
			}
			break;
		case 2:
			if (_moduleResult == 0) {
				createScene(1, 0);
			} else if (_moduleResult == 1) {
				createScene(1002, -1);
			}
			break;
		case 3:
			if (_moduleResult == 0) {
				if (getGlobalVar(V_CREATURE_ANGRY)) {
					createScene(4, 0);
				} else {
					createScene(1003, -1);
				}
			} else if (_moduleResult == 2) {
				createScene(1, 1);
			} else if (_moduleResult == 3) {
				if (getGlobalVar(V_CREATURE_ANGRY)) {
					createScene(4, 0);
				} else {
					setGlobalVar(V_CREATURE_ANGRY, 1);
					createScene(7, -1);
				}
			}
			break;
		case 4:
			if (_moduleResult == 0) {
				leaveModule(1);
			} else {
				createScene(3, 1);
			}
			break;
		case 6:
			if (_moduleResult == 0) {
				createScene(1006, -1);
			} else if (_moduleResult == 1) {
				createScene(1, 2);
			}
			break;
		case 7:
			leaveModule(0);
			break;
		case 8:
			createScene(1008, -1);
			break;
		case 1002:
			createScene(2, 1);
			break;
		case 1003:
			createScene(3, 0);
			break;
		case 1006:
			createScene(8, -1);
			break;
		case 1008:
			createScene(6, 0);
			break;
		}
	}
}
			
SsScene2609Button::SsScene2609Button(NeverhoodEngine *vm, Scene *parentScene)
	: StaticSprite(vm, 1400), _parentScene(parentScene), _countdown(0) {
	
	loadSprite(0x825A6923, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	if (!getGlobalVar(V_WATER_RUNNING))
		setVisible(false);
	loadSound(0, 0x10267160);
	loadSound(1, 0x7027FD64);
	loadSound(2, 0x44043000);
	loadSound(3, 0x44045000);
	SetUpdateHandler(&SsScene2609Button::update);
	SetMessageHandler(&SsScene2609Button::handleMessage);
}

void SsScene2609Button::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		if (getGlobalVar(V_WATER_RUNNING)) {
			setGlobalVar(V_WATER_RUNNING, 0);
			sendMessage(_parentScene, 0x2001, 0);
		} else {
			setGlobalVar(V_WATER_RUNNING, 1);
			sendMessage(_parentScene, 0x2002, 0);
		}
	}
}

uint32 SsScene2609Button::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0) {
			sendMessage(_parentScene, 0x2000, 0);
			if (getGlobalVar(V_WATER_RUNNING)) {
				setVisible(false);
				playSound(3);
				playSound(1);
				_countdown = 12;
			} else {
				setVisible(true);
				playSound(2);
				playSound(0);
				_countdown = 96;
			}
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

AsScene2609Water::AsScene2609Water(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1000) {
	
	_x = 240;
	_y = 420;
	setDoDeltaX(1);
	createSurface1(0x9C210C90, 1200);
	setClipRect(260, 260, 400, 368);
	_vm->_soundMan->addSound(0x08526C36, 0xDC2769B0);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2609Water::handleMessage);
	if (getGlobalVar(V_WATER_RUNNING))
		sendMessage(this, 0x2002, 0);
}

AsScene2609Water::~AsScene2609Water() {
	_vm->_soundMan->deleteSoundGroup(0x08526C36);
}

uint32 AsScene2609Water::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		stopAnimation();
		setVisible(false);
		_vm->_soundMan->stopSound(0xDC2769B0);
		break;
	case 0x2002:
		startAnimation(0x9C210C90, 0, -1);
		setVisible(true);
		_vm->_soundMan->playSoundLooping(0xDC2769B0);
		break;
	}
	return messageResult;
}

Scene2609::Scene2609(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _isBusy(false) {
	
	setBackground(0x51409A16);
	setPalette(0x51409A16);
	_asWater = insertSprite<AsScene2609Water>();
	_ssButton = insertSprite<SsScene2609Button>(this);
	_vm->_collisionMan->addSprite(_ssButton);
	insertMouse435(0x09A1251C, 20, 620);
	insertStaticSprite(0x02138002, 1200);
	insertStaticSprite(0x825E2827, 1200);
	SetMessageHandler(&Scene2609::handleMessage);
	SetUpdateHandler(&Scene::update);
}

uint32 Scene2609::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !_isBusy)
			leaveScene(0);
		break;
	case 0x2000:
		_isBusy = true;
		break;
	case 0x2001:
		_isBusy = false;
		sendMessage(_asWater, 0x2001, 0);
		break;
	case 0x2002:
		_isBusy = false;
		sendMessage(_asWater, 0x2002, 0);
		break;
	}
	return 0;
}

} // End of namespace Neverhood
