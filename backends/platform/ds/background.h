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

#ifndef DS_BACKGROUND_H
#define DS_BACKGROUND_H

#include "graphics/surface.h"

namespace DS {

class Background : public Graphics::Surface {
public:
	void create(uint16 width, uint16 height, bool isRGB, int layer, bool isSub, int mapBase);

	void update();
	void reset();

	void show();
	void hide();
	inline bool isVisible() const { return _visible; }

	inline void clear() {
		memset(getPixels(), 0, pitch * h);
	}

	inline void grab(byte *dst, int dstPitch) {
		for (int y = 0; y < h; ++y) {
			memcpy(dst, getBasePtr(0, y), w * format.bytesPerPixel);
			dst += dstPitch;
		}
	}

protected:
	int _bg;
	bool _visible;
	int _realPitch;
};

} // End of namespace DS

#endif // #ifndef DS_BACKGROUND_H
