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

#ifndef M4_GRAPHICS_GR_SERIES_H
#define M4_GRAPHICS_GR_SERIES_H

#include "m4/m4_types.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

constexpr uint32 SERIES_FORWARD = 0;
constexpr uint32 SERIES_PINGPONG = 1;
constexpr uint32 SERIES_BACKWARD = 2;
constexpr uint32 SERIES_RANDOM = 4;			// series is played in random order, trigger after number of frames in range played
constexpr uint32 SERIES_NO_TOSS = 8;		// series is not tossed at the end of playing
constexpr uint32 SERIES_STICK = 16;			// series sticks on last frame, then sends trigger
constexpr uint32 SERIES_LOOP_TRIGGER = 32;	// get trigger back every loop
constexpr uint32 SERIES_LOAD_PALETTE = 64;	// load master_palette with colours?
constexpr uint32 SERIES_HORZ_FLIP = 128;	// horizontal flip

// Old constants
constexpr uint32 FORWARD = 0;
constexpr uint32 PINGPONG = 1;
constexpr uint32 BACKWARD = 2;
constexpr uint32 STICK = 4;
constexpr uint32 NO_TOSS = 8;

enum {
	HASH_SERIES_PLAY_MACHINE = 0,
	HASH_SERIES_SHOW_MACHINE = 1,
	// HASH_TIMER_MACHINE = 2,  // defined in adv.h

	HASH_STREAM_MACHINE = 6
};

extern int32 series_load(const char *seriesName, int32 assetIndex, RGB8 *myPal = nullptr);
extern void series_unload(int32 assetIndex);
extern bool series_draw_sprite(int32 spriteHash, int32 index, Buffer *destBuff, int32 x, int32 y);

extern bool series_show_frame(int32 spriteHash, int32 index, Buffer *destBuff, int32 x, int32 y);
extern machine *series_place_sprite(char *seriesName, int32 index, int32 x, int32 y, int32 s, int32 layer); // was series_show_sprite
extern machine *series_show_sprite(char *seriesName, int32 index, int32 layer);	// was simple_show_sprite

extern machine *series_play_xy(char *seriesName, int32 loopCount, uint32 flags,
	int32 x, int32 y,
	int32 s, int32 layer, int32 frameRate, int16 triggerNum);

extern machine *series_play(char *seriesName, int32 loopCount, uint32 flags,
	int32 s, int32 layer, int32 frameRate, int16 triggerNum,
	bool stick_when_done);

extern machine *series_simple_play(char *seriesName, frac16 layer, bool stick_when_done);

extern machine *series_ranged_play(char *seriesName, int32 loopCount, uint32 flags,
	int32 firstFrame, int32 lastFrame,
	int32 s, int32 layer, int32 frameRate,
	int16 triggerNum, bool stick_when_done);

machine *series_ranged_play_xy(char *seriesName, int32 loopCount, uint32 flags,
	int32 firstFrame, int32 lastFrame,
	int32 x, int32 y,
	int32 s, int32 layer, int32 frameRate,
	int16 triggerNum, bool stick_when_done);

machine *series_stream(const char *seriesName, int32 frameRate, int32 layer, int32 trigger);
bool series_stream_break_on_frame(machine *m, int32 frameNum, int32 trigger);
void series_set_frame_rate(machine *m, int32 newFrameRate);

extern machine *series_show_(const char *seriesName, frac16 layer, uint32 flags, int16 triggerNum,
	int32 duration, int32 index, int32 s = 100, int32 x = 0, int32 y = 0);
extern machine *series_play_(const char *seriesName, frac16 layer, uint32 flags, int16 triggerNum,
	int32 frameRate, int32 loopCount, int32 s, int32 x, int32 y,
	int32 firstFrame, int32 lastFrame);

} // namespace M4

#endif
