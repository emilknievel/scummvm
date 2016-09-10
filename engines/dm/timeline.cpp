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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "dm/timeline.h"
#include "dm/dungeonman.h"
#include "dm/champion.h"
#include "dm/inventory.h"
#include "dm/group.h"
#include "dm/projexpl.h"
#include "dm/movesens.h"
#include "dm/text.h"
#include "dm/eventman.h"
#include "dm/objectman.h"
#include "dm/sounds.h"


namespace DM {

void Timeline::initConstants() {
	static signed char actionDefense[44] = { // @ G0495_ac_Graphic560_ActionDefense
		0,   /* N */
		36,  /* BLOCK */
		0,   /* CHOP */
		0,   /* X */
		-4,  /* BLOW HORN */
		-10, /* FLIP */
		-10, /* PUNCH */
		-5,  /* KICK */
		4,   /* WAR CRY */
		-20, /* STAB */
		-15, /* CLIMB DOWN */
		-10, /* FREEZE LIFE */
		16,  /* HIT */
		5,   /* SWING */
		-15, /* STAB */
		-17, /* THRUST */
		-5,  /* JAB */
		29,  /* PARRY */
		10,  /* HACK */
		-10, /* BERZERK */
		-7,  /* FIREBALL */
		-7,  /* DISPELL */
		-7,  /* CONFUSE */
		-7,  /* LIGHTNING */
		-7,  /* DISRUPT */
		-5,  /* MELEE */
		-15, /* X */
		-9,  /* INVOKE */
		4,   /* SLASH */
		0,   /* CLEAVE */
		0,   /* BASH */
		5,   /* STUN */
		-15, /* SHOOT */
		-7,  /* SPELLSHIELD */
		-7,  /* FIRESHIELD */
		8,   /* FLUXCAGE */
		-20, /* HEAL */
		-5,  /* CALM */
		0,   /* LIGHT */
		-15, /* WINDOW */
		-7,  /* SPIT */
		-4,  /* BRANDISH */
		0,   /* THROW */
		8    /* FUSE */
	};

	for (int i = 0; i < 44; i++)
		_actionDefense[i] = actionDefense[i];
}

Timeline::Timeline(DMEngine *vm) : _vm(vm) {
	_eventMaxCount = 0;
	_events = nullptr;
	_eventCount = 0;
	_timeline = nullptr;
	_firstUnusedEventIndex = 0;

	initConstants();
}

Timeline::~Timeline() {
	delete[] _events;
	delete[] _timeline;
}

void Timeline::initTimeline() {
	_events = new TimelineEvent[_eventMaxCount];
	_timeline = new uint16[_eventMaxCount];
	if (_vm->_newGameFl) {
		for (int16 i = 0; i < _eventMaxCount; ++i)
			_events[i]._type = k0_TMEventTypeNone;
		_eventCount = 0;
		_firstUnusedEventIndex = 0;
	}
}

void Timeline::deleteEvent(uint16 eventIndex) {
	_events[eventIndex]._type = k0_TMEventTypeNone;
	if (eventIndex < _firstUnusedEventIndex)
		_firstUnusedEventIndex = eventIndex;

	_eventCount--;
	
	uint16 eventCount = _eventCount;
	if (eventCount == 0)
		return;

	uint16 timelineIndex = getIndex(eventIndex);
	if (timelineIndex == eventCount)
		return;

	_timeline[timelineIndex] = _timeline[eventCount];
	fixChronology(timelineIndex);
}

void Timeline::fixChronology(uint16 timelineIndex) {
	uint16 eventCount = _eventCount;
	if (eventCount == 1)
		return;

	uint16 eventIndex = _timeline[timelineIndex];
	TimelineEvent *timelineEvent = &_events[eventIndex];
	bool chronologyFixed = false;
	while (timelineIndex > 0) { /* Check if the event should be moved earlier in the timeline */
		uint16 altTimelineIndex = (timelineIndex - 1) >> 1;
		if (isEventABeforeB(timelineEvent, &_events[_timeline[altTimelineIndex]])) {
			_timeline[timelineIndex] = _timeline[altTimelineIndex];
			timelineIndex = altTimelineIndex;
			chronologyFixed = true;
		} else
			break;
	}
	if (!chronologyFixed) {
		eventCount = ((eventCount - 1) - 1) >> 1;
		while (timelineIndex <= eventCount) { /* Check if the event should be moved later in the timeline */
			uint16 altTimelineIndex = (timelineIndex << 1) + 1;
			if (((altTimelineIndex + 1) < _eventCount) && (isEventABeforeB(&_events[_timeline[altTimelineIndex + 1]], &_events[_timeline[altTimelineIndex]])))
				altTimelineIndex++;
			
			if (isEventABeforeB(&_events[_timeline[altTimelineIndex]], timelineEvent)) {
				_timeline[timelineIndex] = _timeline[altTimelineIndex];
				timelineIndex = altTimelineIndex;
			} else
				break;
		}
	}

	_timeline[timelineIndex] = eventIndex;
}

bool Timeline::isEventABeforeB(TimelineEvent *eventA, TimelineEvent *eventB) {
	bool simultaneousFl = (filterTime(eventA->_mapTime) == filterTime(eventB->_mapTime));

	return (filterTime(eventA->_mapTime) < filterTime(eventB->_mapTime)) ||
		(simultaneousFl && (eventA->getTypePriority() > eventB->getTypePriority())) ||
		(simultaneousFl && (eventA->getTypePriority() == eventB->getTypePriority()) && (eventA <= eventB));
}

uint16 Timeline::getIndex(uint16 eventIndex) {
	uint16 timelineIndex;
	uint16 *timelineEntry = _timeline;

	for (timelineIndex = 0; timelineIndex < _eventMaxCount; timelineIndex++) {
		if (*timelineEntry++ == eventIndex)
			break;
	}

	if (timelineIndex >= _eventMaxCount) /* BUG0_00 Useless code. The function is always called with event indices that are in the timeline */
		timelineIndex = 0; /* BUG0_01 Coding error without consequence. Wrong return value. If the specified event index is not found in the timeline the function returns 0 which is the same value that is returned if the event index is found in the first timeline entry. No consequence because this code is never executed */

	return timelineIndex;
}

uint16 Timeline::addEventGetEventIndex(TimelineEvent *event) {
	if (_eventCount == _eventMaxCount)
		error("Too many events");

	if ((event->_type >= k5_TMEventTypeCorridor) && (event->_type <= k10_TMEventTypeDoor)) {
		TimelineEvent *curEvent = _events;
		for (uint16 eventIndex = 0; eventIndex < _eventMaxCount; eventIndex++, curEvent++) {
			if ((curEvent->_type >= k5_TMEventTypeCorridor) && (curEvent->_type <= k10_TMEventTypeDoor)) {
				if ((event->_mapTime == curEvent->_mapTime) && (event->getMapXY() == curEvent->getMapXY()) && ((curEvent->_type != k6_TMEventTypeWall) || (curEvent->_C.A._cell == event->_C.A._cell))) {
					curEvent->_C.A._effect = event->_C.A._effect;
					return eventIndex;
				}
				continue;
			} else if ((curEvent->_type == k1_TMEventTypeDoorAnimation) && (event->_mapTime == curEvent->_mapTime) && (event->getMapXY() == curEvent->getMapXY())) {
				if (event->_C.A._effect == k2_SensorEffToggle)
					event->_C.A._effect = 1 - curEvent->_C.A._effect;

				deleteEvent(eventIndex);
				break;
			}
		}
	} else if (event->_type == k1_TMEventTypeDoorAnimation) {
		TimelineEvent *curEvent = _events;
		for (uint16 eventIndex = 0; eventIndex < _eventMaxCount; eventIndex++, curEvent++) {
			if ((event->_mapTime == curEvent->_mapTime) && (event->getMapXY() == curEvent->getMapXY())) {
				if (curEvent->_type == k10_TMEventTypeDoor) {
					if (curEvent->_C.A._effect == k2_SensorEffToggle)
						curEvent->_C.A._effect = 1 - event->_C.A._effect;

					return eventIndex;
				}
				if (curEvent->_type == k1_TMEventTypeDoorAnimation) {
					curEvent->_C.A._effect = event->_C.A._effect;
					return eventIndex;
				}
			}
		}
	} else if (event->_type == k2_TMEventTypeDoorDestruction) {
		TimelineEvent *curEvent = _events;
		for (uint16 eventIndex = 0; eventIndex < _eventMaxCount; eventIndex++, curEvent++) {
			if ((event->getMapXY() == curEvent->getMapXY()) && (getMap(event->_mapTime) == getMap(curEvent->_mapTime))) {
				if ((curEvent->_type == k1_TMEventTypeDoorAnimation) || (curEvent->_type == k10_TMEventTypeDoor))
					deleteEvent(eventIndex);
			}
		}
	}

	uint16 newEventIndex = _firstUnusedEventIndex;
	_events[newEventIndex] = *event; /* Copy the event data (Megamax C can assign structures) */
	do {
		if (_firstUnusedEventIndex == _eventMaxCount)
			break;
		_firstUnusedEventIndex++;
	} while ((_events[_firstUnusedEventIndex])._type != k0_TMEventTypeNone);
	_timeline[_eventCount] = newEventIndex;
	fixChronology(_eventCount++);
	return newEventIndex;
}

void Timeline::processTimeline() {
	while (isFirstEventExpiered()) {
		TimelineEvent newEvent;
		TimelineEvent *curEvent = &newEvent;
		extractFirstEvent(curEvent);
		_vm->_dungeonMan->setCurrentMap(getMap(newEvent._mapTime));
		uint16 curEventType = newEvent._type;
		if ((curEventType > (k29_TMEventTypeGroupReactionDangerOnSquare - 1)) && (curEventType < (k41_TMEventTypeUpdateBehaviour_3 + 1)))
			_vm->_groupMan->processEvents29to41(newEvent._B._location._mapX, newEvent._B._location._mapY, curEventType, newEvent._C._ticks);
		else {
			switch (curEventType) {
			case k48_TMEventTypeMoveProjectileIgnoreImpacts:
			case k49_TMEventTypeMoveProjectile:
				_vm->_projexpl->processEvents48To49(curEvent);
				break;
			case k1_TMEventTypeDoorAnimation:
				processEventDoorAnimation(curEvent);
				break;
			case k25_TMEventTypeExplosion:
				_vm->_projexpl->processEvent25(curEvent);
				break;
			case k7_TMEventTypeFakeWall:
				processEventSquareFakewall(curEvent);
				break;
			case k2_TMEventTypeDoorDestruction:
				processEventDoorDestruction(curEvent);
				break;
			case k10_TMEventTypeDoor:
				processEventSquareDoor(curEvent);
				break;
			case k9_TMEventTypePit:
				processEventSquarePit(curEvent);
				break;
			case k8_TMEventTypeTeleporter:
				processEventSquareTeleporter(curEvent);
				break;
			case k6_TMEventTypeWall:
				processEventSquareWall(curEvent);
				break;
			case k5_TMEventTypeCorridor:
				processEventSquareCorridor(curEvent);
				break;
			case k60_TMEventTypeMoveGroupSilent:
			case k61_TMEventTypeMoveGroupAudible:
				processEventsMoveGroup(curEvent);
				break;
			case k65_TMEventTypeEnableGroupGenerator:
				procesEventEnableGroupGenerator(curEvent);
				break;
			case k20_TMEventTypePlaySound:
				_vm->_sound->requestPlay(newEvent._C._soundIndex, newEvent._B._location._mapX, newEvent._B._location._mapY, k1_soundModePlayIfPrioritized);
				break;
			case k24_TMEventTypeRemoveFluxcage:
				if (!_vm->_gameWon) {
					_vm->_dungeonMan->unlinkThingFromList(Thing(newEvent._C._slot), Thing(0), newEvent._B._location._mapX, newEvent._B._location._mapY);
					curEvent = (TimelineEvent *)_vm->_dungeonMan->getThingData(Thing(newEvent._C._slot));
					((Explosion *)curEvent)->setNextThing(Thing::_none);
				}
				break;
			case k11_TMEventTypeEnableChampionAction:
				processEventEnableChampionAction(newEvent._priority);
				if (newEvent._B._slotOrdinal)
					processEventMoveWeaponFromQuiverToSlot(newEvent._priority, _vm->ordinalToIndex(newEvent._B._slotOrdinal));

				_vm->_championMan->drawChampionState((ChampionIndex)newEvent._priority);
				break;
			case k12_TMEventTypeHideDamageReceived:
				processEventHideDamageReceived(newEvent._priority);
				break;
			case k70_TMEventTypeLight:
				_vm->_dungeonMan->setCurrentMap(_vm->_dungeonMan->_partyMapIndex);
				processEventLight(curEvent);
				_vm->_inventoryMan->setDungeonViewPalette();
				break;
			case k71_TMEventTypeInvisibility:
				_vm->_championMan->_party._event71Count_Invisibility--;
				break;
			case k72_TMEventTypeChampionShield:
				_vm->_championMan->_champions[newEvent._priority]._shieldDefense -= newEvent._B._defense;
				setFlag(_vm->_championMan->_champions[newEvent._priority]._attributes, kDMAttributeStatusBox);
				_vm->_championMan->drawChampionState((ChampionIndex)newEvent._priority);
				break;
			case k73_TMEventTypeThievesEye:
				_vm->_championMan->_party._event73Count_ThievesEye--;
				break;
			case k74_TMEventTypePartyShield:
				_vm->_championMan->_party._shieldDefense -= newEvent._B._defense;
				refreshAllChampionStatusBoxes();
				break;
			case k77_TMEventTypeSpellShield:
				_vm->_championMan->_party._spellShieldDefense -= newEvent._B._defense;
				refreshAllChampionStatusBoxes();
				break;
			case k78_TMEventTypeFireShield:
				_vm->_championMan->_party._fireShieldDefense -= newEvent._B._defense;
				refreshAllChampionStatusBoxes();
				break;
			case k75_TMEventTypePoisonChampion: {
				uint16 championIndex = newEvent._priority;
				_vm->_championMan->_champions[championIndex = newEvent._priority]._poisonEventCount--;
				_vm->_championMan->championPoison(championIndex, newEvent._B._attack);
				}
				break;
			case k13_TMEventTypeViAltarRebirth:
				processEventViAltarRebirth(curEvent);
				break;
			case k79_TMEventTypeFootprints:
				_vm->_championMan->_party._event79Count_Footprints--;
			}
		}
		_vm->_dungeonMan->setCurrentMap(_vm->_dungeonMan->_partyMapIndex);
	}
}

bool Timeline::isFirstEventExpiered() {
	return (_eventCount && (filterTime(_events[_timeline[0]]._mapTime) <= _vm->_gameTime));
}

void Timeline::extractFirstEvent(TimelineEvent *event) {
	uint16 eventIndex = _timeline[0];

	*event = _events[eventIndex];
	deleteEvent(eventIndex);
}

void Timeline::processEventDoorAnimation(TimelineEvent *event) {
	uint16 mapX = event->_B._location._mapX;
	uint16 mapY = event->_B._location._mapY;
	Square *curSquare = (Square *)&_vm->_dungeonMan->_currMapData[mapX][mapY];
	int16 doorState = Square(*curSquare).getDoorState();
	if (doorState == k5_doorState_DESTROYED)
		return;

	event->_mapTime++;
	int16 sensorEffect = event->_C.A._effect;
	if (sensorEffect == k1_SensorEffClear) {
		Door *curDoor = (Door *)_vm->_dungeonMan->getSquareFirstThingData(mapX, mapY);
		bool verticalDoorFl = curDoor->opensVertically();
		if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (mapX == _vm->_dungeonMan->_partyMapX)
		 && (mapY == _vm->_dungeonMan->_partyMapY) && (doorState != k0_doorState_OPEN)) {
			if (_vm->_championMan->_partyChampionCount > 0) {
				curSquare->setDoorState(k0_doorState_OPEN);

				// Strangerke
				// Original bug fixed - A closing horizontal door wounds champions to the head instead of to the hands. Missing parenthesis in the condition cause all doors to wound the head in addition to the torso
				// See BUG0_78
				int16 wounds = kDMWoundTorso | (verticalDoorFl ? kDMWoundHead : kDMWoundReadHand | kDMWoundActionHand);
				if (_vm->_championMan->getDamagedChampionCount(5, wounds, kDMAttackTypeSelf))
					_vm->_sound->requestPlay(k18_soundPARTY_DAMAGED, mapX, mapY, k1_soundModePlayIfPrioritized);
			}
			event->_mapTime++;
			addEventGetEventIndex(event);
			return;
		}
		Thing groupThing = _vm->_groupMan->groupGetThing(mapX, mapY);
		uint16 creatureAttributes = _vm->_dungeonMan->getCreatureAttributes(groupThing);
		if ((groupThing != Thing::_endOfList) && !getFlag(creatureAttributes, k0x0040_MaskCreatureInfo_nonMaterial)) {
			if (doorState >= (verticalDoorFl ? CreatureInfo::getHeight(creatureAttributes) : 1)) { /* Creature height or 1 */
				if (_vm->_groupMan->getDamageAllCreaturesOutcome((Group *)_vm->_dungeonMan->getThingData(groupThing), mapX, mapY, 5, true) != k2_outcomeKilledAllCreaturesInGroup)
					_vm->_groupMan->processEvents29to41(mapX, mapY, kM3_TMEventTypeCreateReactionEvent29DangerOnSquare, 0);

				doorState = (doorState == k0_doorState_OPEN) ? k0_doorState_OPEN : (doorState - 1);
				curSquare->setDoorState(doorState);
				_vm->_sound->requestPlay(k04_soundWOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, mapX, mapY, k1_soundModePlayIfPrioritized);
				event->_mapTime++;
				addEventGetEventIndex(event);
				return;
			}
		}
	}
	if ((sensorEffect == k0_SensorEffSet) && (doorState == k0_doorState_OPEN))
		return;

	if ((sensorEffect == k1_SensorEffClear) && (doorState == k4_doorState_CLOSED))
		return;

	doorState += (sensorEffect == k0_SensorEffSet) ? -1 : 1;
	curSquare->setDoorState(doorState);
	_vm->_sound->requestPlay(k02_soundDOOR_RATTLE, mapX, mapY, k1_soundModePlayIfPrioritized);

	if (sensorEffect == k0_SensorEffSet) {
		if (doorState == k0_doorState_OPEN)
			return;
	} else if (doorState == k4_doorState_CLOSED)
		return;

	addEventGetEventIndex(event);
}

void Timeline::processEventSquareFakewall(TimelineEvent *event) {
	uint16 mapX = event->_B._location._mapX;
	uint16 mapY = event->_B._location._mapY;
	byte *curSquare = &_vm->_dungeonMan->_currMapData[mapX][mapY];
	int16 effect = event->_C.A._effect;
	if (effect == k2_SensorEffToggle)
		effect = getFlag(*curSquare, k0x0004_FakeWallOpen) ? k1_SensorEffClear : k0_SensorEffSet;

	if (effect == k1_SensorEffClear) {
		if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (mapX == _vm->_dungeonMan->_partyMapX) && (mapY == _vm->_dungeonMan->_partyMapY)) {
			event->_mapTime++;
			addEventGetEventIndex(event);
		} else {
			Thing groupThing = _vm->_groupMan->groupGetThing(mapX, mapY);
			if ((groupThing != Thing::_endOfList) && !getFlag(_vm->_dungeonMan->getCreatureAttributes(groupThing), k0x0040_MaskCreatureInfo_nonMaterial)) {
				event->_mapTime++;
				addEventGetEventIndex(event);
			} else
				clearFlag(*curSquare, k0x0004_FakeWallOpen);
		}
	} else
		setFlag(*curSquare, k0x0004_FakeWallOpen);
}

void Timeline::processEventDoorDestruction(TimelineEvent *event) {
	Square *square = (Square *)&_vm->_dungeonMan->_currMapData[event->_B._location._mapX][event->_B._location._mapY];
	square->setDoorState(k5_doorState_DESTROYED);
}

void Timeline::processEventSquareDoor(TimelineEvent *event) {
	int16 doorState = Square(_vm->_dungeonMan->_currMapData[event->_B._location._mapX][event->_B._location._mapY]).getDoorState();
	if (doorState == k5_doorState_DESTROYED)
		return;

	if (event->_C.A._effect == k2_SensorEffToggle)
		event->_C.A._effect = (doorState == k0_doorState_OPEN) ? k1_SensorEffClear : k0_SensorEffSet;
	else if (event->_C.A._effect == k0_SensorEffSet) {
		if ((doorState == k0_doorState_OPEN) || (doorState == k4_doorState_CLOSED))
			return;
	}
	event->_type = k1_TMEventTypeDoorAnimation;
	addEventGetEventIndex(event);
}

void Timeline::processEventSquarePit(TimelineEvent *event) {
	uint16 mapX = event->_B._location._mapX;
	uint16 mapY = event->_B._location._mapY;

	byte *square = &_vm->_dungeonMan->_currMapData[mapX][mapY];
	if (event->_C.A._effect == k2_SensorEffToggle)
		event->_C.A._effect = getFlag(*square, k0x0008_PitOpen) ? k1_SensorEffClear : k0_SensorEffSet;

	if (event->_C.A._effect == k0_SensorEffSet) {
		setFlag(*square, k0x0008_PitOpen);
		moveTeleporterOrPitSquareThings(mapX, mapY);
	} else
		clearFlag(*square, k0x0008_PitOpen);
}

void Timeline::moveTeleporterOrPitSquareThings(uint16 mapX, uint16 mapY) {
	if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex)
	 && (mapX == _vm->_dungeonMan->_partyMapX) && (mapY == _vm->_dungeonMan->_partyMapY)) {
		_vm->_moveSens->getMoveResult(Thing::_party, mapX, mapY, mapX, mapY);
		_vm->_championMan->drawChangedObjectIcons();
	}

	Thing curThing = _vm->_groupMan->groupGetThing(mapX, mapY);
	if (curThing != Thing::_endOfList)
		_vm->_moveSens->getMoveResult(curThing, mapX, mapY, mapX, mapY);

	curThing = _vm->_dungeonMan->getSquareFirstObject(mapX, mapY);
	Thing nextThing = curThing;
	int16 thingsToMoveCount = 0;
	while (curThing != Thing::_endOfList) {
		if (curThing.getType() > k4_GroupThingType)
			thingsToMoveCount++;

		curThing = _vm->_dungeonMan->getNextThing(curThing);
	}
	curThing = nextThing;
	while ((curThing != Thing::_endOfList) && thingsToMoveCount) {
		thingsToMoveCount--;
		nextThing = _vm->_dungeonMan->getNextThing(curThing);
		uint16 curThingType = curThing.getType();
		if (curThingType > k4_GroupThingType)
			_vm->_moveSens->getMoveResult(curThing, mapX, mapY, mapX, mapY);

		if (curThingType == k14_ProjectileThingType) {
			Projectile *projectile = (Projectile *)_vm->_dungeonMan->getThingData(curThing);
			TimelineEvent *newEvent;
			newEvent = &_events[projectile->_eventIndex];
			newEvent->_C._projectile.setMapX(_vm->_moveSens->_moveResultMapX);
			newEvent->_C._projectile.setMapY(_vm->_moveSens->_moveResultMapY);
			newEvent->_C._projectile.setDir((Direction)_vm->_moveSens->_moveResultDir);
			newEvent->_B._slot = thingWithNewCell(curThing, _vm->_moveSens->_moveResultCell).toUint16();
			M31_setMap(newEvent->_mapTime, _vm->_moveSens->_moveResultMapIndex);
		} else if (curThingType == k15_ExplosionThingType) {
			TimelineEvent *newEvent = _events;
			for (uint16 i = 0; i < _eventMaxCount; newEvent++, i++) {
				if ((newEvent->_type == k25_TMEventTypeExplosion) && (newEvent->_C._slot == curThing.toUint16())) { /* BUG0_23 A Fluxcage explosion remains on a square forever. If you open a pit or teleporter on a square where there is a Fluxcage explosion, the Fluxcage explosion is moved but the associated event is not updated (because Fluxcage explosions do not use k25_TMEventTypeExplosion but rather k24_TMEventTypeRemoveFluxcage) causing the Fluxcage explosion to remain in the dungeon forever on its destination square. When the k24_TMEventTypeRemoveFluxcage expires the explosion thing is not removed, but it is marked as unused. Consequently, any objects placed on the Fluxcage square after it was moved but before it expires become orphans upon expiration. After expiration, any object placed on the fluxcage square is cloned when picked up */
					newEvent->_B._location._mapX = _vm->_moveSens->_moveResultMapX;
					newEvent->_B._location._mapY = _vm->_moveSens->_moveResultMapY;
					newEvent->_C._slot = thingWithNewCell(curThing, _vm->_moveSens->_moveResultCell).toUint16();
					M31_setMap(newEvent->_mapTime, _vm->_moveSens->_moveResultMapIndex);
				}
			}
		}
		curThing = nextThing;
	}
}

void Timeline::processEventSquareTeleporter(TimelineEvent *event) {
	uint16 mapX = event->_B._location._mapX;
	uint16 mapY = event->_B._location._mapY;

	byte *curSquare = &_vm->_dungeonMan->_currMapData[mapX][mapY];
	if (event->_C.A._effect == k2_SensorEffToggle)
		event->_C.A._effect = getFlag(*curSquare, k0x0008_TeleporterOpen) ? k1_SensorEffClear : k0_SensorEffSet;

	if (event->_C.A._effect == k0_SensorEffSet) {
		setFlag(*curSquare, k0x0008_TeleporterOpen);
		moveTeleporterOrPitSquareThings(mapX, mapY);
	} else
		clearFlag(*curSquare, k0x0008_TeleporterOpen);
}

void Timeline::processEventSquareWall(TimelineEvent *event) {
	int16 mapX = event->_B._location._mapX;
	int16 mapY = event->_B._location._mapY;
	Thing curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
	uint16 curCell = event->_C.A._cell;
	while (curThing != Thing::_endOfList) {
		int16 curThingType = curThing.getType();
		if ((curThingType == k2_TextstringType) && (curThing.getCell() == event->_C.A._cell)) {
			TextString *textString = (TextString *)_vm->_dungeonMan->getThingData(curThing);
			if (event->_C.A._effect == k2_SensorEffToggle)
				textString->setVisible(!textString->isVisible());
			else
				textString->setVisible(event->_C.A._effect == k0_SensorEffSet);
		} else if (curThingType == k3_SensorThingType) {
			Sensor *curThingSensor = (Sensor *)_vm->_dungeonMan->getThingData(curThing);
			uint16 curSensorType = curThingSensor->getType();
			uint16 curSensorData = curThingSensor->getData();
			if (curSensorType == k6_SensorWallCountdown) {
				if (curSensorData > 0) {
					if (event->_C.A._effect == k0_SensorEffSet) {
						if (curSensorData < 511)
							curSensorData++;
					} else
						curSensorData--;

					curThingSensor->setData(curSensorData);
					if (curThingSensor->getAttrEffectA() == k3_SensorEffHold) {
						int16 triggerSetEffect = ((curSensorData == 0) != curThingSensor->getAttrRevertEffectA());
						_vm->_moveSens->triggerEffect(curThingSensor, triggerSetEffect ? k0_SensorEffSet : k1_SensorEffClear, mapX, mapY, curCell);
					} else if (curSensorData == 0)
						_vm->_moveSens->triggerEffect(curThingSensor, curThingSensor->getAttrEffectA(), mapX, mapY, curCell);
				}
			} else if (curSensorType == k5_SensorWallAndOrGate) {
				int16 bitMask = 1 << (event->_C.A._cell);
				if (event->_C.A._effect == k2_SensorEffToggle) {
					if (getFlag(curSensorData, bitMask))
						clearFlag(curSensorData, bitMask);
					else
						setFlag(curSensorData, bitMask);
				} else if (event->_C.A._effect)
					clearFlag(curSensorData, bitMask);
				else
					setFlag(curSensorData, bitMask);
	
				curThingSensor->setData(curSensorData);
				bool triggerSetEffect = (Sensor::getDataMask1(curSensorData) == Sensor::getDataMask2(curSensorData)) != curThingSensor->getAttrRevertEffectA();
				if (curThingSensor->getAttrEffectA() == k3_SensorEffHold)
					_vm->_moveSens->triggerEffect(curThingSensor, triggerSetEffect ? k0_SensorEffSet : k1_SensorEffClear, mapX, mapY, curCell);
				else if (triggerSetEffect)
					_vm->_moveSens->triggerEffect(curThingSensor, curThingSensor->getAttrEffectA(), mapX, mapY, curCell);
			} else if ((((curSensorType >= k7_SensorWallSingleProjLauncherNewObj) && (curSensorType <= k10_SensorWallDoubleProjLauncherExplosion)) || (curSensorType == k14_SensorWallSingleProjLauncherSquareObj) || (curSensorType == k15_SensorWallDoubleProjLauncherSquareObj)) && (curThing.getCell() == event->_C.A._cell)) {
				triggerProjectileLauncher(curThingSensor, event);
				if (curThingSensor->getAttrOnlyOnce())
					curThingSensor->setTypeDisabled();
			} else if (curSensorType == k18_SensorWallEndGame) {
				_vm->delay(60 * curThingSensor->getAttrValue());
				_vm->_restartGameAllowed = false;
				_vm->_gameWon = true;
				_vm->endGame(true);
			}
		}
		curThing = _vm->_dungeonMan->getNextThing(curThing);
	}
	_vm->_moveSens->processRotationEffect();
}

void Timeline::triggerProjectileLauncher(Sensor *sensor, TimelineEvent *event) {
	int16 mapX = event->_B._location._mapX;
	int16 mapY = event->_B._location._mapY;
	uint16 cell = event->_C.A._cell;
	uint16 projectileCell = returnOppositeDir((Direction)cell);
	int16 sensorType = sensor->getType();
	int16 sensorData = sensor->getData();
	int16 kineticEnergy = sensor->getActionKineticEnergy();
	int16 stepEnergy = sensor->getActionStepEnergy();
	bool launchSingleProjectile = (sensorType == k7_SensorWallSingleProjLauncherNewObj) ||
		(sensorType == k8_SensorWallSingleProjLauncherExplosion) ||
		(sensorType == k14_SensorWallSingleProjLauncherSquareObj);

	Thing firstProjectileAssociatedThing;
	Thing secondProjectileAssociatedThing;
	if ((sensorType == k8_SensorWallSingleProjLauncherExplosion) || (sensorType == k10_SensorWallDoubleProjLauncherExplosion))
		firstProjectileAssociatedThing = secondProjectileAssociatedThing = Thing(sensorData + Thing::_firstExplosion.toUint16());
	else if ((sensorType == k14_SensorWallSingleProjLauncherSquareObj) || (sensorType == k15_SensorWallDoubleProjLauncherSquareObj)) {
		firstProjectileAssociatedThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
		while (firstProjectileAssociatedThing != Thing::_none) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. Thing::_none should be Thing::_endOfList. If there are no more objects on the square then this loop may return an undefined value, this can crash the game. In the original DM and CSB dungeons, the number of times that these sensors are triggered is always controlled to be equal to the number of available objects (with a countdown sensor or a number of once only sensors) */
			uint16 projectiveThingCell = firstProjectileAssociatedThing.getCell();
			if ((firstProjectileAssociatedThing.getType() > k3_SensorThingType) && ((projectiveThingCell == cell) || (projectiveThingCell == returnNextVal(cell))))
				break;
			firstProjectileAssociatedThing = _vm->_dungeonMan->getNextThing(firstProjectileAssociatedThing);
		}
		if (firstProjectileAssociatedThing == Thing::_none) /* BUG0_19 The game crashes when an object launcher sensor is triggered. Thing::_none should be Thing::_endOfList */
			return;

		_vm->_dungeonMan->unlinkThingFromList(firstProjectileAssociatedThing, Thing(0), mapX, mapY); /* The object is removed without triggering any sensor effects */
		if (!launchSingleProjectile) {
			secondProjectileAssociatedThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
			while (secondProjectileAssociatedThing != Thing::_none) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. Thing::_none should be Thing::_endOfList. If there are no more objects on the square then this loop may return an undefined value, this can crash the game */
				uint16 projectiveThingCell = secondProjectileAssociatedThing.getCell();
				if ((secondProjectileAssociatedThing.getType() > k3_SensorThingType) && ((projectiveThingCell == cell) || (projectiveThingCell == returnNextVal(cell))))
					break;
				secondProjectileAssociatedThing = _vm->_dungeonMan->getNextThing(secondProjectileAssociatedThing);
			}
			if (secondProjectileAssociatedThing == Thing::_none) /* BUG0_19 The game crashes when an object launcher sensor is triggered. Thing::_none should be Thing::_endOfList */
				launchSingleProjectile = true;
			else
				_vm->_dungeonMan->unlinkThingFromList(secondProjectileAssociatedThing, Thing::_none, mapX, mapY); /* The object is removed without triggering any sensor effects */
		}
	} else {
		firstProjectileAssociatedThing = _vm->_dungeonMan->getObjForProjectileLaucherOrObjGen(sensorData);
		if ((firstProjectileAssociatedThing) == Thing::_none)
			return;

		secondProjectileAssociatedThing = _vm->_dungeonMan->getObjForProjectileLaucherOrObjGen(sensorData);
		if (!launchSingleProjectile && (secondProjectileAssociatedThing == Thing::_none))
			launchSingleProjectile = true;
	}
	if (launchSingleProjectile)
		projectileCell = normalizeModulo4(projectileCell + _vm->getRandomNumber(2));

	/* BUG0_20 The game crashes if the launcher sensor is on a map boundary and shoots in a direction outside the map */
	mapX += _vm->_dirIntoStepCountEast[cell];
	mapY += _vm->_dirIntoStepCountNorth[cell];
	_vm->_projexpl->_createLauncherProjectile = true;
	_vm->_projexpl->createProjectile(firstProjectileAssociatedThing, mapX, mapY, projectileCell, (Direction)cell, kineticEnergy, 100, stepEnergy);
	if (!launchSingleProjectile)
		_vm->_projexpl->createProjectile(secondProjectileAssociatedThing, mapX, mapY, returnNextVal(projectileCell), (Direction)cell, kineticEnergy, 100, stepEnergy);

	_vm->_projexpl->_createLauncherProjectile = false;
}

void Timeline::processEventSquareCorridor(TimelineEvent *event) {
	uint16 mapX = event->_B._location._mapX;
	uint16 mapY = event->_B._location._mapY;
	Thing curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
	while (curThing != Thing::_endOfList) {
		int16 curThingType = curThing.getType();
		if (curThingType == k2_TextstringType) {
			TextString *textString = (TextString *)_vm->_dungeonMan->getThingData(curThing);
			bool textCurrentlyVisible = textString->isVisible();
			if (event->_C.A._effect == k2_SensorEffToggle)
				textString->setVisible(!textCurrentlyVisible);
			else
				textString->setVisible((event->_C.A._effect == k0_SensorEffSet));

			if (!textCurrentlyVisible && textString->isVisible() && (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (mapX == _vm->_dungeonMan->_partyMapX) && (mapY == _vm->_dungeonMan->_partyMapY)) {
				_vm->_dungeonMan->decodeText(_vm->_stringBuildBuffer, curThing, k1_TextTypeMessage);
				_vm->_textMan->printMessage(k15_ColorWhite, _vm->_stringBuildBuffer);
			}
		} else if (curThingType == k3_SensorThingType) {
			Sensor *curSensor = (Sensor *)_vm->_dungeonMan->getThingData(curThing);
			if (curSensor->getType() == k6_SensorFloorGroupGenerator) {
				int16 creatureCount = curSensor->getAttrValue();
				if (getFlag(creatureCount, k0x0008_randomizeGeneratedCreatureCount))
					creatureCount = _vm->getRandomNumber(getFlag(creatureCount, k0x0007_generatedCreatureCount));
				else
					creatureCount--;

				uint16 healthMultiplier = curSensor->getActionHealthMultiplier();
				if (healthMultiplier == 0)
					healthMultiplier = _vm->_dungeonMan->_currMap->_difficulty;

				_vm->_groupMan->groupGetGenerated(curSensor->getData(), healthMultiplier, creatureCount, (Direction)_vm->getRandomNumber(4), mapX, mapY);
				if (curSensor->getAttrAudibleA())
					_vm->_sound->requestPlay(k17_soundBUZZ, mapX, mapY, k1_soundModePlayIfPrioritized);

				if (curSensor->getAttrOnlyOnce())
					curSensor->setTypeDisabled();
				else {
					uint16 actionTicks = curSensor->getActionTicks();
					if (actionTicks != 0) {
						curSensor->setTypeDisabled();
						if (actionTicks > 127)
							actionTicks = (actionTicks - 126) << 6;

						TimelineEvent newEvent;
						newEvent._type = k65_TMEventTypeEnableGroupGenerator;
						setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_currMapIndex, _vm->_gameTime + actionTicks);
						newEvent._priority = 0;
						newEvent._B._location._mapX = mapX;
						newEvent._B._location._mapY = mapY;
						newEvent._B._location._mapY = mapY;
						addEventGetEventIndex(&newEvent);
					}
				}
			}
		}
		curThing = _vm->_dungeonMan->getNextThing(curThing);
	}
}

void Timeline::processEventsMoveGroup(TimelineEvent *event) {
	bool randomDirectionMoveRetried = false;
	uint16 mapX = event->_B._location._mapX;
	uint16 mapY = event->_B._location._mapY;

T0252001:
	if (((_vm->_dungeonMan->_currMapIndex != _vm->_dungeonMan->_partyMapIndex) || (mapX != _vm->_dungeonMan->_partyMapX) || (mapY != _vm->_dungeonMan->_partyMapY)) && (_vm->_groupMan->groupGetThing(mapX, mapY) == Thing::_endOfList)) { /* BUG0_24 Lord Chaos may teleport into one of the Black Flames and become invisible until the Black Flame is killed. In this case, _vm->_groupMan->f175_groupGetThing returns the Black Flame thing and the Lord Chaos thing is not moved into the dungeon until the Black Flame is killed */
		if (event->_type == k61_TMEventTypeMoveGroupAudible)
			_vm->_sound->requestPlay(k17_soundBUZZ, mapX, mapY, k1_soundModePlayIfPrioritized);

		_vm->_moveSens->getMoveResult(Thing(event->_C._slot), kM1_MapXNotOnASquare, 0, mapX, mapY);
	} else {
		if (!randomDirectionMoveRetried) {
			randomDirectionMoveRetried = true;
			Group *group = (Group *)_vm->_dungeonMan->getThingData(Thing(event->_C._slot));
			if ((group->_type == k23_CreatureTypeLordChaos) && !_vm->getRandomNumber(4)) {
				switch (_vm->getRandomNumber(4)) {
				case 0:
					mapX--;
					break;
				case 1:
					mapX++;
					break;
				case 2:
					mapY--;
					break;
				case 3:
					mapY++;
				}
				if (_vm->_groupMan->isSquareACorridorTeleporterPitOrDoor(mapX, mapY))
					goto T0252001;
			}
		}
		event->_mapTime += 5;
		addEventGetEventIndex(event);
	}
}

void Timeline::procesEventEnableGroupGenerator(TimelineEvent *event) {
	Thing curThing = _vm->_dungeonMan->getSquareFirstThing(event->_B._location._mapX, event->_B._location._mapY);
	while (curThing != Thing::_none) {
		if ((curThing.getType()) == k3_SensorThingType) {
			Sensor *curSensor = (Sensor *)_vm->_dungeonMan->getThingData(curThing);
			if (curSensor->getType() == k0_SensorDisabled) {
				curSensor->setDatAndTypeWithOr(k6_SensorFloorGroupGenerator);
				return;
			}
		}
		curThing = _vm->_dungeonMan->getNextThing(curThing);
	}
}

void Timeline::processEventEnableChampionAction(uint16 champIndex) {
	Champion *curChampion = &_vm->_championMan->_champions[champIndex];
	curChampion->_enableActionEventIndex = -1;
	clearFlag(curChampion->_attributes, kDMAttributeDisableAction);
	if (curChampion->_actionIndex != kDMActionNone) {
		curChampion->_actionDefense -= _actionDefense[curChampion->_actionDefense];
	}
	if (curChampion->_currHealth) {
		if ((curChampion->_actionIndex == kDMActionShoot) && (curChampion->_slots[kDMSlotReadyHand] == Thing::_none)) {
			int16 slotIndex = kDMSlotQuiverLine1_1;
			if (_vm->_championMan->isAmmunitionCompatibleWithWeapon(champIndex, kDMSlotActionHand, slotIndex))
				_vm->_championMan->addObjectInSlot((ChampionIndex)champIndex, _vm->_championMan->getObjectRemovedFromSlot(champIndex, slotIndex), kDMSlotReadyHand);
			else {
				for (int16 quiverSlotIndex = 0; quiverSlotIndex < 3; quiverSlotIndex++) {
					slotIndex = quiverSlotIndex + kDMSlotQuiverLine2_1;
					if (_vm->_championMan->isAmmunitionCompatibleWithWeapon(champIndex, kDMSlotActionHand, slotIndex))
						_vm->_championMan->addObjectInSlot((ChampionIndex)champIndex, _vm->_championMan->getObjectRemovedFromSlot(champIndex, slotIndex), kDMSlotReadyHand);
				}
			}
		}
		setFlag(curChampion->_attributes, kDMAttributeActionHand);
		_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
	}
	curChampion->_actionIndex = kDMActionNone;
}

void Timeline::processEventMoveWeaponFromQuiverToSlot(uint16 champIndex, uint16 slotIndex) {
	Champion *curChampion = &_vm->_championMan->_champions[champIndex];
	if (curChampion->_slots[slotIndex] != Thing::_none)
		return;

	if (hasWeaponMovedSlot(champIndex, curChampion, kDMSlotQuiverLine1_1, slotIndex))
		return;

	for (uint16 srcSlotIndex = kDMSlotQuiverLine2_1; srcSlotIndex <= kDMSlotQuiverLine2_2; srcSlotIndex++) {
		if (hasWeaponMovedSlot(champIndex, curChampion, srcSlotIndex, slotIndex))
			break;
	}
}

bool Timeline::hasWeaponMovedSlot(int16 champIndex, Champion *champ, uint16 sourceSlotIndex, int16 destSlotIndex) {
	if (Thing(champ->_slots[sourceSlotIndex]).getType() == k5_WeaponThingType) {
		_vm->_championMan->addObjectInSlot((ChampionIndex)champIndex, _vm->_championMan->getObjectRemovedFromSlot(champIndex, sourceSlotIndex),
			(ChampionSlot)destSlotIndex);
		return true;
	}
	return false;
}

void Timeline::processEventHideDamageReceived(uint16 champIndex) {
	Champion *curChampion = &_vm->_championMan->_champions[champIndex];
	curChampion->_hideDamageReceivedIndex = -1;
	if (!curChampion->_currHealth)
		return;

	if (_vm->indexToOrdinal(champIndex) == _vm->_inventoryMan->_inventoryChampionOrdinal) {
		_vm->_eventMan->showMouse();
		_vm->_inventoryMan->drawStatusBoxPortrait((ChampionIndex)champIndex);
		_vm->_eventMan->hideMouse();
	} else {
		setFlag(curChampion->_attributes, kDMAttributeNameTitle);
		_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
	}
}

void Timeline::processEventLight(TimelineEvent *event) {
	int16 lightPower = event->_B._lightPower;
	if (lightPower == 0)
		return;

	bool negativeLightPower = (lightPower < 0);
	if (negativeLightPower)
		lightPower = -lightPower;

	int16 weakerLightPower = lightPower - 1;
	int16 lightAmount = _vm->_championMan->_lightPowerToLightAmount[lightPower] - _vm->_championMan->_lightPowerToLightAmount[weakerLightPower];
	if (negativeLightPower) {
		lightAmount = -lightAmount;
		weakerLightPower = -weakerLightPower;
	}
	_vm->_championMan->_party._magicalLightAmount += lightAmount;
	if (weakerLightPower) {
		TimelineEvent newEvent;
		newEvent._type = k70_TMEventTypeLight;
		newEvent._B._lightPower = weakerLightPower;
		setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + 4);
		newEvent._priority = 0;
		addEventGetEventIndex(&newEvent);
	}
}

void Timeline::refreshAllChampionStatusBoxes() {
	for (uint16 idx = kDMChampionFirst; idx < _vm->_championMan->_partyChampionCount; idx++)
		setFlag(_vm->_championMan->_champions[idx]._attributes, kDMAttributeStatusBox);

	_vm->_championMan->drawAllChampionStates();
}

void Timeline::processEventViAltarRebirth(TimelineEvent *event) {
	int16 mapX = event->_B._location._mapX;
	int16 mapY = event->_B._location._mapY;
	uint16 cell = event->_C.A._cell;
	uint16 championIndex = event->_priority;
	uint16 rebirthStep = event->_C.A._effect;
	switch (rebirthStep) { /* Rebirth is a 3 steps process (Step 2 -> Step 1 -> Step 0). Step is stored in the Effect value of the event */
	case 2:
		_vm->_projexpl->createExplosion(Thing::_explRebirthStep1, 0, mapX, mapY, cell);
		event->_mapTime += 5;
T0255002:
		rebirthStep--;
		event->_C.A._effect = rebirthStep;
		addEventGetEventIndex(event);
		break;
	case 1: {
		Thing curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
		while (curThing != Thing::_endOfList) {
			if ((curThing.getCell() == cell) && (curThing.getType() == k10_JunkThingType)) {
				int16 iconIndex = _vm->_objectMan->getIconIndex(curThing);
				if (iconIndex == kDMIconIndiceJunkChampionBones) {
					Junk *junkData = (Junk *)_vm->_dungeonMan->getThingData(curThing);
					if (junkData->getChargeCount() == championIndex) {
						_vm->_dungeonMan->unlinkThingFromList(curThing, Thing(0), mapX, mapY); /* BUG0_25 When a champion dies, no bones object is created so it is not possible to bring the champion back to life at an altar of Vi. Each time a champion is brought back to life, the bones object is removed from the dungeon but it is not marked as unused and thus becomes an orphan. After a large number of champion deaths, all JUNK things are exhausted and the game cannot create any more. This also affects the creation of JUNK things dropped by some creatures when they die (Screamer, Rockpile, Magenta Worm, Pain Rat, Red Dragon) */
						junkData->setNextThing(Thing::_none);
						event->_mapTime += 1;
						goto T0255002;
					}
				}
			}
			curThing = _vm->_dungeonMan->getNextThing(curThing);
		}
		}
		break;
	case 0:
		_vm->_championMan->viAltarRebirth(event->_priority);
	}
}

void Timeline::saveEventsPart(Common::OutSaveFile *file) {
	for (uint16 i = 0; i < _eventMaxCount; ++i) {
		TimelineEvent *event = &_events[i];
		file->writeSint32BE(event->_mapTime);
		file->writeByte(event->_type);
		file->writeByte(event->_priority);
		file->writeByte(event->_B._location._mapX); // writing bytes of the union I think should preserve the union's identity
		file->writeByte(event->_B._location._mapY);
		file->writeUint16BE(event->_C.A._cell); // writing bytes of the union I think should preserve the union's identity
		file->writeUint16BE(event->_C.A._effect);
	}
}

void Timeline::saveTimelinePart(Common::OutSaveFile *file) {
	for (uint16 i = 0; i < _eventMaxCount; ++i)
		file->writeUint16BE(_timeline[i]);
}

void Timeline::loadEventsPart(Common::InSaveFile *file) {
	for (uint16 i = 0; i < _eventMaxCount; ++i) {
		TimelineEvent *event = &_events[i];
		event->_mapTime = file->readSint32BE();
		event->_type = file->readByte();
		event->_priority = file->readByte();
		event->_B._location._mapX = file->readByte();
		event->_B._location._mapY = file->readByte();
		event->_C.A._cell = file->readUint16BE();
		event->_C.A._effect = file->readUint16BE();
	}
}

void Timeline::loadTimelinePart(Common::InSaveFile *file) {
	for (uint16 i = 0; i < _eventMaxCount; ++i)
		_timeline[i] = file->readUint16BE();
}

}
