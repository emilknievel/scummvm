/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gargoyle/fonts.h"
#include "gargoyle/glk_types.h"
#include "gargoyle/conf.h"
#include "common/file.h"
#include "graphics/fonts/ttf.h"

namespace Gargoyle {

const char *gli_conf_propr = "NotoSerif-Regular";
const char *gli_conf_propb = "NotoSerif-Bold";
const char *gli_conf_propi = "NotoSerif-Italic";
const char *gli_conf_propz = "NotoSerif-BoldItalic";

const char *gli_conf_monor = "GoMono-Regular";
const char *gli_conf_monob = "GoMono-Bold";
const char *gli_conf_monoi = "GoMono-Italic";
const char *gli_conf_monoz = "GoMono-BoldItalic";

#ifdef BUNDLED_FONTS
const char *gli_conf_monofont = "";
const char *gli_conf_propfont = "";
const double gli_conf_monosize = 12.5;	///< good size for GoMono
const double gli_conf_propsize = 13.4;	///< good size for NotoSerif
#else
const char *gli_conf_monofont = "Liberation Mono";
const char *gli_conf_propfont = "Linux Libertine O";
const double gli_conf_monosize = 12.5;	///< good size for LiberationMono
const double gli_conf_propsize = 15.5;	///< good size for Libertine
#endif

Fonts::Fonts(Graphics::ManagedSurface *surface) : _surface(surface) {
	double monoAspect = g_conf->_monoAspect;
	double propAspect = g_conf->_propAspect;
	double monoSize = g_conf->_monoSize;
	double propSize = g_conf->_propSize;

	_fontTable[0] = loadFont(MONOR, monoSize, monoAspect, FONTR);
	_fontTable[1] = loadFont(MONOB, monoSize, monoAspect, FONTB);
	_fontTable[2] = loadFont(MONOI, monoSize, monoAspect, FONTI);
	_fontTable[3] = loadFont(MONOZ, monoSize, monoAspect, FONTZ);

	_fontTable[4] = loadFont(PROPR, propSize, propAspect, FONTR);
	_fontTable[5] = loadFont(PROPB, propSize, propAspect, FONTB);
	_fontTable[6] = loadFont(PROPI, propSize, propAspect, FONTI);
	_fontTable[7] = loadFont(PROPZ, propSize, propAspect, FONTZ);
}

Fonts::~Fonts() {
	for (int idx = 0; idx < FONTS_TOTAL; ++idx)
		delete _fontTable[idx];
}

FACES Fonts::getId(const Common::String &name) {
	if (name == "monor") return MONOR;
	if (name == "monob") return MONOB;
	if (name == "monoi") return MONOI;
	if (name == "monoz") return MONOZ;
	if (name == "propr") return PROPR;
	if (name == "propb") return PROPB;
	if (name == "propi") return PROPI;
	if (name == "propz") return PROPZ;
	return MONOR;
}

Graphics::Font *Fonts::loadFont(FACES face, double size, double aspect, int style) {
	static const char *const MAP[8] = {
		"Go-Mono-Regular",
		"Go-Mono-Bold",
		"Go-Mono-Italic",
		"Go-Mono-BoldItalic",
		"NotoSerif-Regular",
		"NotoSerif-Bold",
		"NotoSerif-Italic",
		"NotoSerif-BoldItalic"
	};

	Common::File f;
	if (!f.open(Common::String::format("%s.ttf", MAP[face])))
		return nullptr;

	return Graphics::loadTTFFont(f, size, Graphics::kTTFSizeModeCharacter);
}

int Fonts::drawString(const Point &pos, int fontIdx, const byte *rgb, const Common::String &text, int spw) {
	Graphics::Font *font = _fontTable[fontIdx];
	const uint32 color = _surface->format.RGBToColor(rgb[0], rgb[1], rgb[2]);
	font->drawString(_surface, text, pos.x, pos.y, _surface->w - pos.x, color);
	return font->getBoundingBox(text, pos.x, pos.y, _surface->w - pos.x).right;
}

int Fonts::drawStringUni(const Point &pos, int fontIdx, const byte *rgb, const Common::U32String &text, int spw) {
	Graphics::Font *font = _fontTable[fontIdx];
	const uint32 color = _surface->format.RGBToColor(rgb[0], rgb[1], rgb[2]);
	font->drawString(_surface, text, pos.x, pos.y, _surface->w - pos.x, color);

	return font->getBoundingBox(text, pos.x, pos.y, _surface->w - pos.x).right;
}

size_t Fonts::stringWidth(int fontIdx, const Common::String &text, int spw) {
	// TODO: Handle spw
	Graphics::Font *font = _fontTable[fontIdx];
	return font->getStringWidth(text);
}

size_t Fonts::stringWidthUni(int fontIdx, const Common::U32String &text, int spw) {
	// TODO: Handle spw
	Graphics::Font *font = _fontTable[fontIdx];
	return font->getStringWidth(text);
}

} // End of namespace Gargoyle
