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

#include "common/scummsys.h"

#include "trecision/nl/extern.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {
/*-------------------------------------------------------------------------*/
/*                               PROCESSTIME          					   */
/*-------------------------------------------------------------------------*/
void ProcessTime() {
	static uint8 OldRegInvSI  = 0xFF;
	static uint8 OldRegInvSL  = 0xFF;
	static uint8 OldLightIcon = 0xFF;

	TheTime = g_vm->ReadTime();
	g_vm->_animMgr->refreshAllAnimations();

	if (TheTime >= g_vm->_nextRefresh) {
		g_vm->drawString();

		if ((g_vm->_inventoryStatus == INV_PAINT) || (g_vm->_inventoryStatus == INV_DEPAINT))
			g_vm->rollInventory(g_vm->_inventoryStatus);

		if ((g_vm->_inventoryStatus != INV_OFF)
			&& ((OldRegInvSI != g_vm->_inventoryRefreshStartIcon) || (OldRegInvSL != g_vm->_inventoryRefreshStartLine) || (OldLightIcon != g_vm->_lightIcon))) {
			g_vm->refreshInventory(g_vm->_inventoryRefreshStartIcon, g_vm->_inventoryRefreshStartLine);
			OldRegInvSI = g_vm->_inventoryRefreshStartIcon;
			OldRegInvSL = g_vm->_inventoryRefreshStartLine;
			OldLightIcon = g_vm->_lightIcon;
		}

		PaintScreen(0);
		g_vm->_textStackTop = -1;

		g_vm->_flagScreenRefreshed = true;
		uint32 PaintTime = g_vm->ReadTime();
		if (PaintTime - TheTime >= 5)
			g_vm->_nextRefresh = PaintTime + 1;
		else
			g_vm->_nextRefresh = TheTime + 5;
	}
}

/*-------------------------------------------------------------------------*/
/*                              PROCESSMOUSE          					   */
/*-------------------------------------------------------------------------*/
void ProcessMouse() {
	static bool MaskMouse;
	static uint16 oldmx;
	static uint16 oldmy;
	static bool LastMouseON = true;
	int16 mx = g_vm->_mouseX;
	int16 my = g_vm->_mouseY;

	if (LastMouseON && !g_vm->isCursorVisible()) {
		oldmx = 0;    // Switch off
		oldmy = 0;
	} else if (!LastMouseON && g_vm->isCursorVisible()) {
		oldmx = 0;    // Switch on
		oldmy = 0;
	}

	LastMouseON = g_vm->isCursorVisible();
	g_vm->checkSystem();

	if (!g_vm->isCursorVisible())
		return;

	if (g_vm->_mouseLeftBtn || g_vm->_mouseRightBtn) {
		if (!MaskMouse) {
			doEvent(MC_MOUSE, g_vm->_mouseRightBtn ? ME_MRIGHT : ME_MLEFT, MP_DEFAULT, mx, my, 0, 0);
			MaskMouse = true;
		}
	} else {
		MaskMouse = false;

		if (!g_vm->_flagscriptactive) {
			if (mx != oldmx || my != oldmy) {
				doEvent(MC_MOUSE, ME_MMOVE, MP_DEFAULT, mx, my, 0, 0);
				oldmx = mx;
				oldmy = my;
			}
		}
	}
}

} // End of namespace Trecision
