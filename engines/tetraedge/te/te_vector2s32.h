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

#ifndef TETRAEDGE_TE_TE_VECTOR2S32_H
#define TETRAEDGE_TE_TE_VECTOR2S32_H

#include "common/rect.h"

namespace Tetraedge {

class TeVector2s32 {
public:
	TeVector2s32();
	TeVector2s32(int x_, int y_) : _x(x_), _y(y_) {};
	TeVector2s32(const Common::Point &pt) : _x(pt.x), _y(pt.y) {};

	bool operator!=(const TeVector2s32 &other) const {
		return _x != other._x || _y != other._y;
	}
	bool operator==(const TeVector2s32 &other) const {
		return _x == other._x && _y == other._y;
	}

	TeVector2s32 &operator+=(const TeVector2s32 &other) {
		_x += other._x;
		_y += other._y;
		return *this;
	}

public:
	int _x;
	int _y;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_VECTOR2S32_H
