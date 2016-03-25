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

#include "titanic/messages/messages.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/core/game_object.h"
#include "titanic/core/tree_item.h"
#include "titanic/titanic.h"

namespace Titanic {

CMessage::CMessage() : CSaveableObject() {
}

void CMessage::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
}

void CMessage::load(SimpleFile *file) {
	file->readNumber();
	CSaveableObject::load(file);
}

bool CMessage::execute(CTreeItem *target, const ClassDef *classDef, int flags) {
	// If no target was specified, then there's nothing to do
	if (!target)
		return false;

	bool result = false;
	CTreeItem *item = target;
	CTreeItem *nextItem = nullptr;
	do {
		if (flags & MSGFLAG_SCAN)
			nextItem = item->scan(target);

		if (!classDef || item->isInstanceOf(classDef)) {
			bool handled = perform(item);

			if (handled) {
				result = true;
				if (flags & MSGFLAG_BREAK_IF_HANDLED)
					return true;
			}
		}

		item = nextItem;
	} while (nextItem);

	return result;
}

bool CMessage::execute(const CString &target, const ClassDef *classDef, int flags) {
	// Scan for the target by name
	CProjectItem *project = g_vm->_window->_project;
	for (CTreeItem *treeItem = project; treeItem; treeItem = treeItem->scan(project)) {
		if (treeItem->getName().compareToIgnoreCase(target))
			return execute(treeItem, classDef, flags);
	}

	return false;
}

bool CMessage::isMouseMsg() const {
	return dynamic_cast<const CMouseMsg *>(this) != nullptr;
}

bool CMessage::isButtonDownMsg() const {
	return dynamic_cast<const CMouseButtonDownMsg *>(this) != nullptr;
}

bool CMessage::isButtonUpMsg() const {
	return dynamic_cast<const CMouseButtonUpMsg *>(this) != nullptr;
}

bool CMessage::isMouseMoveMsg() const {
	return dynamic_cast<const CMouseMoveMsg *>(this) != nullptr;
}

bool CMessage::isDoubleClickMsg() const {
	return dynamic_cast<const CMouseDoubleClickMsg *>(this) != nullptr;
}

bool CMessage::isEnterRoomMsg() const {
	return dynamic_cast<const CEnterRoomMsg *>(this) != nullptr;
}

bool CMessage::isPreEnterRoomMsg() const {
	return dynamic_cast<const CPreEnterRoomMsg *>(this) != nullptr;
}

bool CMessage::isleaveRoomMsg() const {
	return dynamic_cast<const CLeaveRoomMsg *>(this) != nullptr;
}

bool CMessage::isEnterNodeMsg() const {
	return dynamic_cast<const CEnterNodeMsg *>(this) != nullptr;
}

bool CMessage::isPreEnterNodeMsg() const {
	return dynamic_cast<const CPreEnterNodeMsg *>(this) != nullptr;
}

bool CMessage::isLeaveNodeMsg() const {
	return dynamic_cast<const CLeaveNodeMsg *>(this) != nullptr;
}

bool CMessage::isEnterViewMsg() const {
	return dynamic_cast<const CEnterViewMsg *>(this) != nullptr;
}

bool CMessage::isPreEnterViewMsg() const {
	return dynamic_cast<const CPreEnterViewMsg *>(this) != nullptr;
}

bool CMessage::isLeaveViewMsg() const {
	return dynamic_cast<const CLeaveViewMsg *>(this) != nullptr;
}

} // End of namespace Titanic
