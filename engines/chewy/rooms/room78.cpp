/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/room.h"
#include "chewy/rooms/room78.h"

namespace Chewy {
namespace Rooms {

void Room78::entry() {
	_G(spieler).r76_word18DB1A = -1;
	det->enable_sound(0, 1);
	det->play_sound(0, 1);
	hide_cur();
	_G(spieler).ScrollxStep = 1;
	_G(spieler).PersonHide[P_CHEWY] = 1;
	_G(spieler).PersonHide[P_HOWARD] = 1;
	_G(spieler).PersonHide[P_NICHELLE] = 1;
	_G(spieler).scrollx = 320;
	set_person_pos(0, 0, P_CHEWY, P_RIGHT);

	int delay = 0;
	int destRoom;

	if (_G(spieler).flags29_80) {
		det->start_detail(3, 255, false);

		int destX = 592;
		while (destX > 0) {
			det->set_detail_pos(3, destX, 77);
			if (delay)
				--delay;
			else {
				delay = _G(spieler).DelaySpeed / 2;
				destX -= 4;
			}
			set_up_screen(DO_SETUP);
		}
		flags.NoPalAfterFlc = true;
		flic_cut(101, CFO_MODE);
		destRoom = 79;
	} else {
		det->start_detail(0, 255, false);
		det->start_detail(4, 255, false);
		int det0DestX = 608;
		int det4DestX = 570;
		bool exitLoopFlag = false;
		bool flag1 = false;
		bool flag2 = false;

		while (exitLoopFlag == 0) {
			det->set_detail_pos(0, det0DestX, 93);
			det->set_detail_pos(4, det4DestX, 57);
			
			if (delay)
				--delay;
			else {
				det0DestX -= 4;
				if (det0DestX <= 276 && flag1 == 0) {
					if (_G(spieler).flags29_20) {
						if (det0DestX > 0)
							flag1 = true;
					} else {
						flag1 = true;
						det->stop_detail(0);
						det->start_detail(1, 1, false);
					}
				}

				det4DestX -= 4;
				
				if (det4DestX <= 222 && flag2 == 0) {
					if (_G(spieler).flags29_10) {
						flag2 = true;
						det->stop_detail(4);
						if (flag1 == 0) {
							det->start_detail(5, 1, false);
						} else {
							start_detail_wait(5, 1, CFO_MODE);
						}
					} else {
						if (det4DestX <= 0)
							flag2 = true;
					}
				}

				delay = _G(spieler).DelaySpeed / 3;
			}

			if (flag1 && flag2)
				exitLoopFlag = true;

			set_up_screen(DO_SETUP);
		}

		if (_G(spieler).flags29_10 && _G(spieler).flags29_20) {
			_G(spieler).r76_word18DB1A = 1;
			destRoom = 77;
		} else {
			_G(spieler).r76_word18DB1A = 0;
			destRoom = 76;
		}		
	}

	switch_room(destRoom);
	show_cur();
}

void Room78::xit() {
	_G(spieler).ScrollxStep = 1;
	_G(spieler).PersonHide[P_CHEWY] = 0;
	_G(spieler).PersonHide[P_HOWARD] = 0;
	_G(spieler).PersonHide[P_NICHELLE] = 0;
}

} // namespace Rooms
} // namespace Chewy
