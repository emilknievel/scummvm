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

#include "common/system.h"
#include "common/macresman.h"

#include "graphics/cursorman.h"
#include "graphics/maccursor.h"
#include "graphics/macega.h"
#include "graphics/primitives.h"
#include "graphics/fonts/macfont.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/gfx_mac.h"
#include "scumm/usage_bits.h"
#include "scumm/verbs.h"

namespace Scumm {

void ScummEngine::mac_markScreenAsDirty(int x, int y, int w, int h) {
	// Mark the virtual screen as dirty. The top and left coordinates are
	// rounded down, while the bottom and right ones are rounded up.

	VirtScreen *vs = &_virtscr[kMainVirtScreen];

	int vsTop = y / 2 - vs->topline;
	int vsBottom = (y + h) / 2 - vs->topline;
	int vsLeft = x / 2;
	int vsRight = (x + w) / 2;

	if ((y + h) & 1)
		vsBottom++;

	if ((x + w) & 1)
		vsRight++;

	markRectAsDirty(kMainVirtScreen, vsLeft, vsRight, vsTop, vsBottom);
}

void ScummEngine::mac_drawStripToScreen(VirtScreen *vs, int top, int x, int y, int width, int height) {
	// The verb screen is completely replaced with a custom GUI. While
	// it is active, all other drawing to that area is suspended.
	if (_macGui && vs->number == kVerbVirtScreen && _macGui->isVerbGuiActive())
		return;

	const byte *pixels = vs->getPixels(x, top);
	const byte *ts = (byte *)_textSurface.getBasePtr(x * 2, y * 2);
	byte *mac = (byte *)_macScreen->getBasePtr(x * 2, y * 2);

	int pixelsPitch = vs->pitch;
	int tsPitch = _textSurface.pitch;
	int macPitch = _macScreen->pitch;

	// In b/w Mac rendering mode, the shadow palette is implemented here,
	// and not as a palette manipulation. See special cases in o5_roomOps()
	// and updatePalette().
	//
	// This is used at the very least for the lightning flashes at Castle
	// Brunwald in Indy 3, as well as the scene where the dragon finds
	// Rusty in Loom.
	//
	// Interestingly, the original Mac interpreter does not seem to do
	// this, and instead just renders the scene as if the palette was
	// unmodified. At least, that's what Mini vMac did when I tried it.

	if (_renderMode == Common::kRenderMacintoshBW) {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				int color = enhancementEnabled(kEnhVisualChanges) ? _shadowPalette[pixels[w]] : pixels[w];
				if (ts[2 * w] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w] = Graphics::macEGADither[color][0];
				if (ts[2 * w + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + 1] = Graphics::macEGADither[color][1];
				if (ts[2 * w + tsPitch] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch] = Graphics::macEGADither[color][2];
				if (ts[2 * w + tsPitch + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch + 1] = Graphics::macEGADither[color][3];
			}

			pixels += pixelsPitch;
			ts += tsPitch * 2;
			mac += macPitch * 2;
		}
	} else {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (ts[2 * w] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w] = pixels[w];
				if (ts[2 * w + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + 1] = pixels[w];
				if (ts[2 * w + tsPitch] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch] = pixels[w];
				if (ts[2 * w + tsPitch + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch + 1] = pixels[w];
			}

			pixels += pixelsPitch;
			ts += tsPitch * 2;
			mac += macPitch * 2;
		}
	}

	_system->copyRectToScreen(_macScreen->getBasePtr(x * 2, y * 2), _macScreen->pitch, x * 2, y * 2, width * 2, height * 2);
}

void ScummEngine::mac_drawLoomPracticeMode() {
	// In practice mode, the game shows the notes as they are being played.
	// In the DOS version, this is drawn by script 27 but the Mac version
	// just sets variables 50 and 54. The box is actually a verb, and it
	// seems that setting variable 50 is pretty much equal to turning verb
	// 53 on or off. I'm not sure what the purpose of variable 54 is.

	int x = 216;
	int y = 377;
	int width = 62;
	int height = 22;
	int var = 50;

	byte *ptr = (byte *)_macScreen->getBasePtr(x,  y);
	int pitch = _macScreen->pitch;

	int slot = getVerbSlot(53, 0);
	VerbSlot *vs = &_verbs[slot];

	vs->curmode = (VAR(var) != 0);
	vs->curRect.left = x / 2;
	vs->curRect.right = (x + width) / 2;
	vs->curRect.top = y / 22;
	vs->curRect.bottom = (y + height) / 2;

	_macScreen->fillRect(Common::Rect(x, y, x + width, y + height), 0);

	if (VAR(var)) {
		for (int w = 1; w < width - 1; w++) {
			ptr[w] = 7;
			ptr[w + pitch * (height - 1)] = 7;
		}

		for (int h = 1; h < height - 1; h++) {
			ptr[h * pitch] = 7;
			ptr[h * pitch + width - 1] = 7;
		}

		// Draw the notes
		int colors[] = { 4, 12, 14, 10, 11, 3, 9, 15 };

		for (int i = 0; i < 4; i++) {
			int note = (VAR(var) >> (4 * i)) & 0x0F;

			if (note >= 2 && note <= 9) {
				_charset->setColor(colors[note - 2]);
				_charset->drawChar(14 + note, *_macScreen, i * 13 + x + 8, y + 4);
			}
		}
	}

	_system->copyRectToScreen(ptr, pitch, x, y, width, height);
}

void ScummEngine::mac_createIndy3TextBox(Actor *a) {
	int width = _macIndy3TextBox->w;
	int height = _macIndy3TextBox->h;

	_macIndy3TextBox->fillRect(Common::Rect(width, height), 0);

	int nameWidth = 0;
	byte color = _charset->getColor();

	if (a) {
		const Graphics::Font *font = _macGui->getFont(MacGui::kIndy3FontSmall);

		const char *name = (const char *)a->getActorName();
		int charX = 25;

		for (int i = 0; name[i] && nameWidth < width - 50; i++) {
			font->drawChar(_macIndy3TextBox, name[i], charX, 0, color);
			nameWidth += font->getCharWidth(name[i]);
			charX += font->getCharWidth(name[i]);
		}

		font->drawChar(_macIndy3TextBox, ':', charX, 0, color);
	}

	if (nameWidth) {
		_macIndy3TextBox->hLine(2, 3, 20, 15);
		_macIndy3TextBox->hLine(32 + nameWidth, 3, width - 3, 15);
	} else
		_macIndy3TextBox->hLine(2, 3, width - 3, 15);

	_macIndy3TextBox->vLine(1, 4, height - 3, 15);
	_macIndy3TextBox->vLine(width - 2, 4, height - 3, 15);
	_macIndy3TextBox->hLine(2, height - 2, width - 3, 15);
}

void ScummEngine::mac_drawIndy3TextBox() {
	// The first two rows of the text box are padding for font rendering.
	// They are not drawn to the screen.

	int x = 96;
	int y = 32;
	int w = _macIndy3TextBox->w;
	int h = _macIndy3TextBox->h - 2;

	// The text box is drawn to the Mac screen and text surface, as if it
	// had been one giant glyph. Note that it will be drawn on the main
	// virtual screen, but we still pretend it's on the text one.

	byte *ptr = (byte *)_macIndy3TextBox->getBasePtr(0, 2);
	int pitch = _macIndy3TextBox->pitch;

	_macScreen->copyRectToSurface(ptr, pitch, x, y, w, h);
	_textSurface.fillRect(Common::Rect(x, y, x + w, y + h), 0);

	mac_markScreenAsDirty(x, y, w, h);
}

void ScummEngine::mac_undrawIndy3TextBox() {
	int x = 96;
	int y = 32;
	int w = _macIndy3TextBox->w;
	int h = _macIndy3TextBox->h - 2;

	_macScreen->fillRect(Common::Rect(x, y, x + w, y + h), 0);
	_textSurface.fillRect(Common::Rect(x, y, x + w, y + h), CHARSET_MASK_TRANSPARENCY);

	mac_markScreenAsDirty(x, y, w, h);
}

void ScummEngine::mac_undrawIndy3CreditsText() {
	// Set _masMask to make the text clear, and _textScreenID to ensure
	// that it's the main area that's cleared.
	_charset->_hasMask = true;
	_charset->_textScreenID = kMainVirtScreen;
	restoreCharsetBg();
}

Common::KeyState ScummEngine::mac_showOldStyleBannerAndPause(const char *msg, int32 waitTime) {
	char bannerMsg[512];

	_messageBannerActive = true;

	// Fetch the translated string for the message...
	convertMessageToString((const byte *)msg, (byte *)bannerMsg, sizeof(bannerMsg));

	MacGui::SimpleWindow *window = _macGui->drawBanner(bannerMsg);

	// Pause shake effect
	_shakeTempSavedState = _shakeEnabled;
	setShake(0);

	// Pause the engine
	PauseToken pt = pauseEngine();

	Common::KeyState ks = Common::KEYCODE_INVALID;
	bool leftBtnPressed = false, rightBtnPressed = false;
	if (waitTime) {
		waitForBannerInput(waitTime, ks, leftBtnPressed, rightBtnPressed);
	}

	delete window;

	// Finally, resume the engine, clear the input state, and restore the charset.
	pt.clear();
	clearClickedStatus();

	_messageBannerActive = false;

	return ks;
}

// Very simple window class

MacGui::SimpleWindow::SimpleWindow(OSystem *system, Graphics::Surface *from, Common::Rect bounds, SimpleWindowStyle style) : _system(system), _from(from), _bounds(bounds) {
	_backup = new Graphics::Surface();
	_backup->create(bounds.width(), bounds.height(), Graphics::PixelFormat::createFormatCLUT8());
	_backup->copyRectToSurface(*_from, 0, 0, bounds);

	_margin = (style == kStyleNormal) ? 6 : 4;

	_surface = _from->getSubArea(bounds);
	bounds.grow(-_margin);
	_innerSurface = _from->getSubArea(bounds);

	_dirtyRects.clear();

	Graphics::Surface *s = surface();
	Common::Rect r = Common::Rect(0, 0, s->w, s->h);

	r.grow(-1);
	s->fillRect(r, kWhite);

	if (style == MacGui::kStyleNormal) {
		int growths[] = { 1, -3, -1 };

		for (int i = 0; i < ARRAYSIZE(growths); i++) {
			r.grow(growths[i]);

			s->hLine(r.left, r.top, r.right - 1, kBlack);
			s->hLine(r.left, r.bottom - 1, r.right - 1, kBlack);
			s->vLine(r.left, r.top + 1, r.bottom - 2, kBlack);
			s->vLine(r.right - 1, r.top + 1, r.bottom - 2, kBlack);
		}
	} else if (style == MacGui::kStyleRounded) {
		r.grow(1);

		for (int i = 0; i < 2; i++) {
			s->hLine(r.left + 2, r.top, r.right - 3, kBlack);
			s->hLine(r.left + 2, r.bottom - 1, r.right - 3, kBlack);
			s->vLine(r.left, r.top + 2, r.bottom - 3, kBlack);
			s->vLine(r.right - 1, r.top + 2, r.bottom - 3, kBlack);
			s->setPixel(r.left + 1, r.top + 1, kBlack);
			s->setPixel(r.left + 1, r.bottom - 2, kBlack);
			s->setPixel(r.right - 2, r.top + 1, kBlack);
			s->setPixel(r.right - 2, r.bottom - 2, kBlack);
			r.grow(-2);
		}
	}
}

MacGui::SimpleWindow::~SimpleWindow() {
	copyToScreen(_backup);
	_backup->free();
	delete _backup;
}

void MacGui::SimpleWindow::copyToScreen(Graphics::Surface *s) {
	if (s) {
		_from->copyRectToSurface(*s, _bounds.left, _bounds.top, Common::Rect(0, 0, _bounds.width(), _bounds.height()));
	}
	_system->copyRectToScreen(_from->getBasePtr(_bounds.left, _bounds.top), _from->pitch, _bounds.left, _bounds.top, _bounds.width(), _bounds.height());
}

void MacGui::SimpleWindow::show() {
	copyToScreen();
	_dirtyRects.clear();
}

void MacGui::SimpleWindow::markRectAsDirty(Common::Rect r) {
	_dirtyRects.push_back(r);
}

void MacGui::SimpleWindow::update() {
	for (uint i = 0; i < _dirtyRects.size(); i++) {
		_system->copyRectToScreen(
			_innerSurface.getBasePtr(_dirtyRects[i].left, _dirtyRects[i].top),
			_innerSurface.pitch,
			_bounds.left + _margin + _dirtyRects[i].left, _bounds.top + _margin + _dirtyRects[i].top,
			_dirtyRects[i].width(), _dirtyRects[i].height());
	}

	_dirtyRects.clear();
}

void MacGui::SimpleWindow::fillPattern(Common::Rect r, uint16 pattern) {
	for (int y = r.top; y < r.bottom; y++) {
		for (int x = r.left; x < r.right; x++) {
			int bit = 0x8000 >> (4 * (y % 4) + (x % 4));
			_innerSurface.setPixel(x, y, (pattern & bit) ? kBlack : kWhite);
		}
	}

	markRectAsDirty(r);
}

void MacGui::SimpleWindow::drawSprite(Graphics::Surface *sprite, int x, int y, Common::Rect(clipRect)) {
	Common::Rect subRect(0, 0, sprite->w, sprite->h);

	if (x < clipRect.left) {
		subRect.left += (clipRect.left - x);
		x = clipRect.left;
	}

	if (y < clipRect.top) {
		subRect.top += (clipRect.top - y);
		y = clipRect.top;
	}

	if (x + sprite->w >= clipRect.right) {
		subRect.right -= (x + sprite->w - clipRect.right);
	}

	if (y + sprite->h >= clipRect.bottom) {
		subRect.bottom -= (y + sprite->h - clipRect.bottom);
	}

	if (subRect.width() > 0 && subRect.height() > 0) {
		_innerSurface.copyRectToSurface(*sprite, x, y, subRect);
		markRectAsDirty(Common::Rect(x, y, x + subRect.width(), y + subRect.height()));
	}
}

void MacGui::SimpleWindow::plotPixel(int x, int y, int color, void *data) {
	MacGui::SimpleWindow *window = (MacGui::SimpleWindow *)data;
	Graphics::Surface *s = window->innerSurface();
	s->setPixel(x, y, color);
}

void MacGui::SimpleWindow::drawTextBox(Common::Rect r) {
	Graphics::drawRoundRect(r, 9, kWhite, true, plotPixel, this);
	Graphics::drawRoundRect(r, 9, kBlack, false, plotPixel, this);
	markRectAsDirty(r);
}

// ===========================================================================
// Macintosh user interface for the Macintosh versions of Loom and Indiana
// Jones and the Last Crusade.
// ===========================================================================

static void menuCallback(int id, Common::String &name, void *data) {
	((MacGui *)data)->handleMenu(id, name);
}

MacGui::MacGui(ScummEngine *vm, Common::String resourceFile) : _vm(vm), _system(_vm->_system), _surface(_vm->_macScreen), _resourceFile(resourceFile) {
	_fonts.clear();
}

MacGui::~MacGui() {
	delete _windowManager;
}

void MacGui::initialize() {
	_windowManager = new Graphics::MacWindowManager(Graphics::kWMModeNoDesktop | Graphics::kWMModeAutohideMenu | Graphics::kWMModalMenuMode | Graphics::kWMModeNoCursorOverride);
	_windowManager->setEngine(_vm);
	_windowManager->setScreen(640, 400);
	_windowManager->setMenuHotzone(Common::Rect(0, 0, 640, 23));
	_windowManager->setMenuDelay(250000);

	Common::MacResManager resource;
	Common::SeekableReadStream *res;
	Graphics::MacMenu *menu = _windowManager->addMenu();

	resource.open(_resourceFile);

	// Add the Apple menu

	const Graphics::MacMenuData menuSubItems[] = {
		{ 0, NULL, 0, 0, false }
	};

	Common::String aboutMenuDef = "About " + name() + "...<B";

	menu->addStaticMenus(menuSubItems);
	menu->createSubMenuFromString(0, aboutMenuDef.c_str(), 0);

	menu->setCommandsCallback(menuCallback, this);

	for (int i = 129; i <= 130; i++) {
		res = resource.getResource(MKTAG('M', 'E', 'N', 'U'), i);

		if (!res)
			continue;

		Common::StringArray *menuDef = Graphics::MacMenu::readMenuFromResource(res);
		Common::String name = menuDef->operator[](0);
		Common::String string = menuDef->operator[](1);
		int id = menu->addMenuItem(nullptr, name);
		menu->createSubMenuFromString(id, string.c_str(), 0);
	}

	resource.close();

	// Assign sensible IDs to the menu items

	int numberOfMenus = menu->numberOfMenus();

	for (int i = 0; i < numberOfMenus; i++) {
		Graphics::MacMenuItem *item = menu->getMenuItem(i);
		int numberOfMenuItems = menu->numberOfMenuItems(item);
		int id = 100 * (i + 1);
		for (int j = 0; j < numberOfMenuItems; j++) {
			Graphics::MacMenuItem *subItem = menu->getSubMenuItem(item, j);
			Common::String name = menu->getName(subItem);

			if (!name.empty())
				menu->setAction(subItem, id++);
		}
	}

	// Register custom fonts. The font family just happens to match the
	// printed name of the game.

	const Common::String fontFamily = name();

	const Common::Array<Graphics::MacFontFamily *> &fontFamilies = _windowManager->_fontMan->getFontFamilies();

	_windowManager->_fontMan->loadFonts(_resourceFile);

	for (uint i = 0; i < fontFamilies.size(); i++) {
		if (fontFamilies[i]->getName() == fontFamily) {
			_gameFontId = _windowManager->_fontMan->registerFontName(fontFamily, fontFamilies[i]->getFontFamilyId());
			break;
		}
	}
}

const Graphics::Font *MacGui::getFont(FontId fontId) {
	const Graphics::Font *font = _fonts.getValOrDefault((int)fontId);

	if (font)
		return font;

	int id;
	int size;
	int slant;

	switch (fontId) {
	case kSystemFont:
		id = Graphics::kMacFontChicago;
		size = 12;
		slant = Graphics::kMacFontRegular;
		break;

	default:
		getFontParams(fontId, id, size, slant);
		break;
	}


	font = _windowManager->_fontMan->getFont(Graphics::MacFont(id, size, slant));
	_fonts[(int)fontId] = font;

	return font;
}

const Graphics::Surface *MacGui::loadPICT(int id) {
	Common::MacResManager resource;

	resource.open(_resourceFile);

	Common::SeekableReadStream *res = resource.getResource(MKTAG('P', 'I', 'C', 'T'), id);

	// The PICT decoder is mainly for v2 pictures, so at least for now we
	// don't use it. The images we need to decode are simple bitmaps
	// anyway.

	uint16 size = res->readUint16BE();

	uint16 top = res->readUint16BE();
	uint16 left = res->readUint16BE();
	uint16 bottom = res->readUint16BE();
	uint16 right = res->readUint16BE();

	Graphics::Surface *s = new Graphics::Surface();
	s->create(right - left, bottom - top, Graphics::PixelFormat::createFormatCLUT8());

	bool endOfPicture = false;

	while (!endOfPicture) {
		byte opcode = res->readByte();
		byte value;
		int x1, x2, y1, y2;
		int rowBytes;
		int mode;

		int x = 0;
		int y = 0;

		switch (opcode) {
		case 0x01: // clipRgn
			// The clip region is not important
			res->skip(res->readUint16BE() - 2);
			break;

		case 0x11: // picVersion
			value = res->readByte();
			assert(value == 1);
			break;

		case 0x99: // PackBitsRgn
			res->skip(2);	// Skip rowBytes

			y1 = res->readSint16BE();
			x1 = res->readSint16BE();
			y2 = res->readSint16BE();
			x2 = res->readSint16BE();

			res->skip(8);	// Skip srcRect
			res->skip(8);	// Skip dstRect
			res->skip(2);	// Skip mode
			res->skip(res->readUint16BE() - 2);	// Skip maskRgn

			for (y = y1; y < y2; y++) {
				x = x1;
				size = res->readByte();

				while (size > 0) {
					byte count = res->readByte();
					size--;

					bool repeat;

					if (count >= 128) {
						// Repeat value
						count = 256 - count;
						repeat = true;
						value = res->readByte();
						size--;
					} else {
						// Copy values
						repeat = false;
					}

					for (int j = 0; j <= count; j++) {
						if (!repeat) {
							value = res->readByte();
							size--;
						}
						for (int k = 7; k >= 0 && x < x2; k--, x++) {
							if (value & (1 << k))
								s->setPixel(x, y, kBlack);
							else
								s->setPixel(x, y, kWhite);
						}
					}
				}
			}

			break;

		case 0xA0: // shortComment
			res->skip(2);
			break;

		case 0xFF: // EndOfPicture
			endOfPicture = true;
			break;

		default:
			debug("Unknown opcode: 0x%02x", opcode);
			break;
		}
	}

	resource.close();

	return s;
}

bool MacGui::handleMenu(int id, Common::String &name) {
	// This menu item (e.g. a menu separator) has no action, so it's
	// handled trivially.
	if (id == 0)
		return true;

	_windowManager->getMenu()->closeMenu();

	if (id == 100) {
		showAboutDialog();
		return true;
	}

	return false;
}

bool MacGui::handleEvent(Common::Event &event) {
	return _windowManager->processEvent(event);
}

void MacGui::setPalette(const byte *palette, uint size) {
	_windowManager->passPalette(palette, size);
}

void MacGui::updateWindowManager() {
	// TODO: Originally, the left Alt button opens the menu. In ScummVM,
	//       it triggers on mouse-over. Preferrably both should work (the
	//       latter should work better for touch screens), but it's hard
	//       to get them to coexist.

	// We want the arrow cursor for menus. Note that the menu triggers even
	// when the mouse is invisible, which may or may not be a bug. But the
	// original did allow you to open the menu with Alt even when the
	// cursor was visible, so for now it's a feature.

	bool isActive = _windowManager->isMenuActive();

	if (isActive) {
		if (!_menuIsActive) {
			_cursorWasVisible = CursorMan.showMouse(true);
			_windowManager->pushCursor(Graphics::kMacCursorArrow);
		}
	} else {
		if (_menuIsActive) {
			_windowManager->popCursor();
			CursorMan.showMouse(_cursorWasVisible);
		}
	}

	_menuIsActive = isActive;
	_windowManager->draw();
}

MacGui::SimpleWindow *MacGui::drawBanner(char *message) {
	MacGui::SimpleWindow *window = openWindow(Common::Rect(70, 189, 570, 211), kStyleRounded);
	const Graphics::Font *font = getFont(_vm->_game.id == GID_INDY3 ? kIndy3FontMedium : kLoomFontMedium);

	Graphics::Surface *s = window->innerSurface();
	font->drawString(s, (char *)message, 0, 0, s->w, kBlack, Graphics::kTextAlignCenter);

	window->show();
	return window;
}

int MacGui::delay(uint32 ms) {
	uint32 to = _system->getMillis() + ms;

	while (_system->getMillis() < to) {
		Common::Event event;

		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				return 2;

			case Common::EVENT_LBUTTONDOWN:
				return 1;

			default:
				break;
			}
		}

		uint32 delta = to - _system->getMillis();

		if (delta > 0) {
			_system->delayMillis(MIN<uint32>(delta, 10));
			_system->updateScreen();
		}
	}

	return 0;
}

MacGui::SimpleWindow *MacGui::openWindow(Common::Rect bounds, SimpleWindowStyle style) {
	return new SimpleWindow(_system, _surface, bounds, style);
}

// ===========================================================================
// The Mac Loom GUI. This one is pretty simple.
// ===========================================================================

const Graphics::Font *MacLoomGui::getFontByScummId(int32 id) {
	switch (id) {
	case 0:
		return getFont(kLoomFontLarge);
	default:
		error("MacLoomGui::getFontByScummId: Invalid font id %d", id);
	}
}

void MacLoomGui::getFontParams(FontId fontId, int &id, int &size, int &slant) {
	// Loom uses only font size 13 for in-game text, but size 12 is used
	// for system messages, e.g. the original pause dialog.
	//
	// Special characters:
	//
	// 16-23 are the note names c through c'.
	// 60 is an upside-down note, i.e. the one used for c'.
	// 95 is a used for the rest of the notes.

	switch (fontId) {
	case FontId::kLoomFontSmall:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		break;

	case FontId::kLoomFontMedium:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontRegular;
		break;

	case FontId::kLoomFontLarge:
		id = _gameFontId;
		size = 13;
		slant = Graphics::kMacFontRegular;
		break;

	default:
		error("MacLoomGui: getFontParams: Unknown font id %d", (int)fontId);
	}
}

void MacLoomGui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	if (_windowManager->getCursorType() == Graphics::kMacCursorCustom)
		return;

	Common::MacResManager resource;

	resource.open(_resourceFile);

	Common::MacResIDArray resArray = resource.getResIDArray(MKTAG('C', 'U', 'R', 'S'));
	Common::SeekableReadStream *curs = resource.getResource(MKTAG('C', 'U', 'R', 'S'), resArray[0]);
	Graphics::MacCursor macCursor;

	if (macCursor.readFromStream(*curs)) {
		width = macCursor.getWidth();
		height = macCursor.getHeight();
		hotspotX = macCursor.getHotspotX();
		hotspotY = macCursor.getHotspotY();
		animate = 0;

		_windowManager->pushCustomCursor(&macCursor);
	}

	delete curs;
}

bool MacLoomGui::handleMenu(int id, Common::String &name) {
	if (MacGui::handleMenu(id, name))
		return true;
	return false;
}

void MacLoomGui::showAboutDialog() {
}

// ===========================================================================
// The Mac GUI for Indiana Jones and the Last Crusade, including the infamous
// verb GUI.
//
// It's likely that the original interpreter used more hooks from the engine
// into the GUI. In particular, the inventory script uses a variable that I
// haven't been able to find in any of the early scripts of the game.
//
// But the design goal here is to keep things as simple as possible, even if
// that means using brute force. So the GUI figures out which verbs are active
// by scanning all the verbs, and which objects are in the inventory by
// scanning all objects.
//
// We used to coerce the Mac verb GUI into something that looked like the
// GUI from all other versions. This used a number of variables and hard-coded
// verbs. The only thing still used of all this is variable 67, to keep track
// of the inventory scroll position. The fake verbs are removed when loading
// old savegames, but the variables are assumed to be harmless.
// ===========================================================================

#define WIDGET_TIMER_JIFFIES	12
#define REPEAT_TIMER_JIFFIES	18

// ---------------------------------------------------------------------------
// The basic building block of the GUI is the Widget. It has a bounding box, a
// timer, a couple of drawing primitives, etc.
// ---------------------------------------------------------------------------

ScummEngine *MacIndy3Gui::Widget::_vm = nullptr;
Graphics::Surface *MacIndy3Gui::Widget::_surface = nullptr;
MacIndy3Gui *MacIndy3Gui::Widget::_gui = nullptr;

MacIndy3Gui::Widget::Widget(int x, int y, int width, int height) {
	_bounds.left = x;
	_bounds.top = y;
	_bounds.right = x + width;
	_bounds.bottom = y + height;
}

void MacIndy3Gui::Widget::reset() {
	clearTimer();
	_enabled = false;
	setRedraw(false);
}

void MacIndy3Gui::Widget::updateTimer(int delta) {
	if (hasTimer()) {
		if (delta > _timer)
			delta = _timer;

		_timer -= delta;

		if (_timer == 0)
			timeOut();
	}
}

void MacIndy3Gui::Widget::draw() {
	markScreenAsDirty(_bounds);
	_redraw = false;
}

void MacIndy3Gui::Widget::undraw() {
	fill(_bounds);
	markScreenAsDirty(_bounds);
	_redraw = false;
}

void MacIndy3Gui::Widget::markScreenAsDirty(Common::Rect r) const {
	_gui->markScreenAsDirty(r);
}

byte MacIndy3Gui::Widget::translateChar(byte c) const {
	if (c == '^')
		return 0xC9;
	return c;
}

void MacIndy3Gui::Widget::fill(Common::Rect r) const {
	_gui->fill(r);
}

void MacIndy3Gui::Widget::drawBitmap(Common::Rect r, const uint16 *bitmap, Color color) const {
	_gui->drawBitmap(r, bitmap, color);
}

// The shadow box is the basic outline of the verb buttons and the inventory
// widget. A slightly rounded rectangle with a shadow and a highlight.

void MacIndy3Gui::Widget::drawShadowBox(Common::Rect r) const {
	_surface->hLine(r.left + 1, r.top, r.right - 3, kBlack);
	_surface->hLine(r.left + 1, r.bottom - 2, r.right - 3, kBlack);
	_surface->vLine(r.left, r.top + 1, r.bottom - 3, kBlack);
	_surface->vLine(r.right - 2, r.top + 1, r.bottom - 3, kBlack);

	_surface->hLine(r.left + 2, r.bottom - 1, r.right - 1, kBlack);
	_surface->vLine(r.right - 1, r.top + 2, r.bottom - 2, kBlack);

	_surface->hLine(r.left + 1, r.top + 1, r.right - 3, kWhite);
	_surface->vLine(r.left + 1, r.top + 2, r.bottom - 3, kWhite);
}

// The shadow frame is a rectangle with a highlight. It can be filled or
// unfilled.

void MacIndy3Gui::Widget::drawShadowFrame(Common::Rect r, Color shadowColor, Color fillColor) const {
	_surface->hLine(r.left, r.top, r.right - 1, kBlack);
	_surface->hLine(r.left, r.bottom - 1, r.right - 1, kBlack);
	_surface->vLine(r.left, r.top + 1, r.bottom - 2, kBlack);
	_surface->vLine(r.right - 1, r.top + 1, r.bottom - 2, kBlack);

	_surface->hLine(r.left + 1, r.top + 1, r.right - 2, shadowColor);
	_surface->vLine(r.left + 1, r.top + 2, r.bottom - 2, shadowColor);

	if (fillColor != kTransparency) {
		Common::Rect fillRect(r.left + 2, r.top + 2, r.right - 1, r.bottom - 1);

		if (fillColor == kBackground)
			fill(fillRect);
		else
			_surface->fillRect(fillRect, fillColor);
	}
}

// ---------------------------------------------------------------------------
// The VerbWidget is what the user interacts with. Each one is connected to a
// verb id and slot. A VerbWidget can consist of several Widgets.
// ---------------------------------------------------------------------------

void MacIndy3Gui::VerbWidget::reset() {
	MacIndy3Gui::Widget::reset();
	_verbslot = -1;
	_visible = false;
	_kill = false;
}

void MacIndy3Gui::VerbWidget::updateVerb(int verbslot) {
	VerbSlot *vs = &_vm->_verbs[verbslot];
	bool enabled = (vs->curmode == 1);

	if (!_visible || _enabled != enabled)
		setRedraw(true);

	_verbslot = verbslot;
	_verbid = vs->verbid;
	_enabled = enabled;
	_kill = false;
}

void MacIndy3Gui::VerbWidget::draw() {
	Widget::draw();
	_visible = true;
}

void MacIndy3Gui::VerbWidget::undraw() {
	debug(1, "VerbWidget: Undrawing [%d]", _verbid);

	Widget::undraw();
	_visible = false;
}

// ---------------------------------------------------------------------------
// The most common type of VerbWidget is the button, which is used for verbs
// and conversation options.
// ---------------------------------------------------------------------------

void MacIndy3Gui::Button::reset() {
	MacIndy3Gui::VerbWidget::reset();
	_text.clear();
}

bool MacIndy3Gui::Button::handleEvent(Common::Event &event) {
	if (!_enabled || !_verbid)
		return false;

	VerbSlot *vs = &_vm->_verbs[_verbslot];

	if (vs->saveid)
		return false;

	bool caughtEvent = false;

	if (event.type == Common::EVENT_KEYDOWN) {
		if (!(event.kbd.flags & Common::KBD_NON_STICKY) && event.kbd.keycode == vs->key)
			caughtEvent = true;
	} else if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (_bounds.contains(event.mouse))
			caughtEvent = true;
	}

	// Events are handled at the end of the animation. This blatant attack
	// on speedrunners all over the world was done because that's what the
	// original seems to do, and it looks better. Based on tests in Mac
	// emulators, the speed of the animation depended on the speed of your
	// computer, so we just set something that looks good here.

	if (caughtEvent) {
		setRedraw(true);
		setTimer(WIDGET_TIMER_JIFFIES);
	}

	return caughtEvent;
}

void MacIndy3Gui::Button::timeOut() {
	if (_visible) {
		_vm->runInputScript(kVerbClickArea, _verbid, 1);
		setRedraw(true);
	}
}

void MacIndy3Gui::Button::updateVerb(int verbslot) {
	MacIndy3Gui::VerbWidget::updateVerb(verbslot);

	const byte *ptr = _vm->getResourceAddress(rtVerb, verbslot);
	byte buf[270];

	_vm->convertMessageToString(ptr, buf, sizeof(buf));
	if (_text != (char *)buf) {
		_text = (char *)buf;
		clearTimer();
		setRedraw(true);
	}
}

void MacIndy3Gui::Button::draw() {
	if (!getRedraw())
		return;

	debug(1, "Button: Drawing [%d] %s", _verbid, _text.c_str());

	MacIndy3Gui::VerbWidget::draw();
	fill(_bounds);

	if (!hasTimer()) {
		drawShadowBox(_bounds);
	} else {
		// I have only been able to capture a screenshot of the pressed
		// button in black and white, where the checkerboard background
		// makes it hard to see exactly which pixels should be drawn.
		// Basilisk II runs it too fast, and I haven't gotten Mini vMac
		// to run it in 16-color mode.
		//
		// All I can say for certain is that the upper left corner is
		// rounded while the lower right is not. I'm going to assume
		// that the shadow is always drawn, and the rest of the button
		// is just shifted down to the right. That would make the other
		// two corners rounded, so only the lower right one isn't.

		int x0 = _bounds.left + 1;
		int x1 = _bounds.right - 1;
		int y0 = _bounds.top + 1;
		int y1 = _bounds.bottom - 1;

		_surface->hLine(x0 + 1, y0, x1 - 1, kBlack);
		_surface->hLine(x0 + 1, y1, x1, kBlack);
		_surface->vLine(x0, y0 + 1, y1 - 1, kBlack);
		_surface->vLine(x1, y0 + 1, y1 - 1, kBlack);

		_surface->hLine(x0 + 1, y0 + 1, x1 - 1, kWhite);
		_surface->vLine(x0 + 1, y0 + 2, y1 - 1, kWhite);
	}

	// The text is drawn centered. Based on experimentation, I think the
	// width is always based on the outlined font, and the button shadow is
	// not counted as part of the button width.
	//
	// This gives us pixel perfect rendering for the English verbs.

	if (!_text.empty()) {
		const Graphics::Font *boldFont = _gui->getFont(kIndy3VerbFontBold);
		const Graphics::Font *outlineFont = _gui->getFont(kIndy3VerbFontOutline);

		int stringWidth = 0;
		for (uint i = 0; i < _text.size(); i++)
			stringWidth += outlineFont->getCharWidth(_text[i]);

		int x = (_bounds.left + (_bounds.width() - 1 - stringWidth) / 2) - 1;
		int y = _bounds.top + 2;
		Color color = _enabled ? kWhite : kBlack;

		if (hasTimer()) {
			x++;
			y++;
		}

		for (uint i = 0; i < _text.size() && x < _bounds.right; i++) {
			byte c = translateChar(_text[i]);
			if (x >= _bounds.left) {
				if (_enabled)
					outlineFont->drawChar(_surface, c, x, y, kBlack);
				boldFont->drawChar(_surface, c, x + 1, y, color);
			}
			x += boldFont->getCharWidth(c);
		}
	}
}

// ---------------------------------------------------------------------------
// Unlike in the DOS version, where each inventory object shown on screen is
// its own verb, in the Macintosh version the entire Inventory widget is one
// single verb. It consists of the widget itself, the inventory slots, and
// the scrollbar.
// ---------------------------------------------------------------------------

MacIndy3Gui::Inventory::Inventory(int x, int y, int width, int height) : MacIndy3Gui::VerbWidget(x, y, width, height) {
	x = _bounds.left + 6;
	y = _bounds.top + 6;

	// There are always six slots, no matter how many objects you are
	// carrying.
	//
	// Each slot is 12 pixels tall (as seen when they are highlighted),
	// which means they have to overlap slightly to fit. It is assumed
	// that this will never interfere with the text drawing.

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		_slots[i] = new Slot(i, x, y, 128, 12);
		y += 11;
	}

	x = _bounds.right - 20;

	_scrollBar = new ScrollBar(x, _bounds.top + 19, 16, 40);

	_scrollButtons[0] = new ScrollButton(x, _bounds.top + 4, 16, 16, kScrollUp);
	_scrollButtons[1] = new ScrollButton(x, _bounds.bottom - 20, 16, 16, kScrollDown);
}

MacIndy3Gui::Inventory::~Inventory() {
	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		delete _slots[i];

	delete _scrollBar;

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		delete _scrollButtons[i];
}

void MacIndy3Gui::Inventory::reset() {
	MacIndy3Gui::VerbWidget::reset();

	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		_slots[i]->reset();

	_scrollBar->reset();

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->reset();
}

void MacIndy3Gui::Inventory::setRedraw(bool redraw) {
	MacIndy3Gui::Widget::setRedraw(redraw);

	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		_slots[i]->setRedraw(redraw);

	_scrollBar->setRedraw(redraw);

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->setRedraw(redraw);
}

bool MacIndy3Gui::Inventory::handleEvent(Common::Event &event) {
	if (!_enabled || !_verbid || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->handleEvent(event))
			return true;
	}

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
		ScrollButton *b = _scrollButtons[i];

		// Scrolling is done by one object at a time, though you can
		// hold down the mouse button to repeat.

		if (b->handleEvent(event)) {
			_scrollBar->scroll(b->_direction);
			return true;
		}
	}

	if (_scrollBar->handleEvent(event))
		return true;

	return false;
}

bool MacIndy3Gui::Inventory::handleMouseHeld(Common::Point &pressed, Common::Point &held) {
	if (!_enabled || !_verbid)
		return false;

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->handleMouseHeld(pressed, held))
			return true;
	}

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
		ScrollButton *b = _scrollButtons[i];

		if (b->handleMouseHeld(pressed, held)) {
			_scrollBar->scroll(b->_direction);
			return true;
		}
	}

	// It would be possible to handle dragging the scrollbar handle, but
	// the original didn't.

	if (_scrollBar->handleMouseHeld(pressed, held))
		return true;

	return false;
}

void MacIndy3Gui::Inventory::updateTimer(int delta) {
	Widget::updateTimer(delta);

	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		_slots[i]->updateTimer(delta);

	_scrollBar->updateTimer(delta);

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->updateTimer(delta);
}

void MacIndy3Gui::Inventory::updateVerb(int verbslot) {
	MacIndy3Gui::VerbWidget::updateVerb(verbslot);

	int owner = _vm->VAR(_vm->VAR_EGO);

	int invCount = _vm->getInventoryCount(owner);
	int invOffset = _gui->getInventoryScrollOffset();
	int numSlots = ARRAYSIZE(_slots);

	// The scroll offset must be non-negative and if there are numSlots or
	// less objects in the inventory, the inventory is fixed in the top
	// position.

	if (invOffset < 0 || invCount <= numSlots)
		invOffset = 0;

	// If there are more than numSlots objects in the inventory, clamp the
	// scroll offset so that the inventory does not go past the last
	// numSlots objets.

	if (invCount > numSlots && invOffset > invCount - numSlots)
		invOffset = invCount - numSlots;

	_scrollButtons[0]->setEnabled(invOffset > 0);
	_scrollButtons[1]->setEnabled(invCount > numSlots && invOffset < invCount - numSlots);

	_scrollBar->setEnabled(invCount > numSlots);
	_scrollBar->setInventoryParameters(invCount, invOffset);

	_gui->setInventoryScrollOffset(invOffset);

	int invSlot = 0;

	// Assign the objects to the inventory slots

	for (int i = 0; i < _vm->_numInventory && invSlot < numSlots; i++) {
		int obj = _vm->_inventory[i];
		if (obj && _vm->getOwner(obj) == owner) {
			if (--invOffset < 0) {
				_slots[invSlot]->setObject(obj);
				invSlot++;
			}
		}
	}

	// Clear the remaining slots

	for (int i = invSlot; i < numSlots; i++)
		_slots[i]->clearObject();
}

void MacIndy3Gui::Inventory::draw() {
	if (getRedraw()) {
		debug(1, "Inventory: Drawing [%d]", _verbid);

		MacIndy3Gui::VerbWidget::draw();

		drawShadowBox(_bounds);
		drawShadowFrame(Common::Rect(_bounds.left + 4, _bounds.top + 4, _bounds.right - 22, _bounds.bottom - 4), kBlack, kWhite);

		const uint16 upArrow[] = {
			0x0000, 0x0000, 0x0000, 0x0080,
			0x0140, 0x0220, 0x0410, 0x0808,
			0x1C1C, 0x0410, 0x0410, 0x0410,
			0x07F0, 0x0000, 0x0000, 0x0000
		};

		const uint16 downArrow[] = {
			0x0000, 0x0000, 0x0000, 0x0000,
			0x07F0, 0x0410, 0x0410, 0x0410,
			0x1C1C, 0x0808, 0x0410, 0x0220,
			0x0140, 0x0080, 0x0000, 0x0000
		};

		for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
			ScrollButton *s = _scrollButtons[i];
			const uint16 *arrow = (s->_direction == kScrollUp) ? upArrow : downArrow;

			drawShadowFrame(s->_bounds, kWhite, kTransparency);
			drawBitmap(s->_bounds, arrow, kBlack);
			s->draw();
		}
	}

	// Since the inventory slots overlap, draw the highlighted ones (and
	// there should really only be one at a time) last.

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (!_slots[i]->hasTimer())
			_slots[i]->draw();
	}

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->hasTimer())
			_slots[i]->draw();
	}

	_scrollBar->draw();

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->draw();
}

// ---------------------------------------------------------------------------
// Inventory::Slot is a widget for a single inventory object.
// ---------------------------------------------------------------------------

MacIndy3Gui::Inventory::Slot::Slot(int slot, int x, int y, int width, int height) : MacIndy3Gui::Widget(x, y, width, height) {
	_slot = slot;
}

void MacIndy3Gui::Inventory::Slot::reset() {
	Widget::reset();
	clearName();
	clearObject();
}

void MacIndy3Gui::Inventory::Slot::clearObject() {
	_obj = -1;
	setEnabled(false);

	if (hasName()) {
		clearName();
		setRedraw(true);
	}
}

void MacIndy3Gui::Inventory::Slot::setObject(int obj) {
	_obj = obj;

	const byte *ptr = _vm->getObjOrActorName(obj);

	if (ptr) {
		byte buf[270];
		_vm->convertMessageToString(ptr, buf, sizeof(buf));

		if (_name != (const char *)buf) {
			setEnabled(true);
			_name = (const char *)buf;
			clearTimer();
			setRedraw(true);
		}
	} else if (hasName()) {
		setEnabled(false);
		clearName();
		clearTimer();
		setRedraw(true);
	}
}

bool MacIndy3Gui::Inventory::Slot::handleEvent(Common::Event &event) {
	if (!_enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	if (_bounds.contains(event.mouse)) {
		setRedraw(true);
		if (hasTimer())
			timeOut();
		setTimer(WIDGET_TIMER_JIFFIES);
		return true;
	}

	return false;
}

void MacIndy3Gui::Inventory::Slot::timeOut() {
	_vm->runInputScript(kInventoryClickArea, getObject(), 1);
	setRedraw(true);
}

void MacIndy3Gui::Inventory::Slot::draw() {
	if (!getRedraw())
		return;

	debug(1, "Inventory::Slot: Drawing [%d] %s", _slot, _name.c_str());

	Widget::draw();

	Color fg, bg;

	if (hasTimer()) {
		fg = kWhite;
		bg = kBlack;
	} else {
		fg = kBlack;
		bg = kWhite;
	}

	_surface->fillRect(_bounds, bg);

	if (hasName()) {
		const Graphics::Font *font = _gui->getFont(kIndy3VerbFontRegular);

		int y = _bounds.top - 1;
		int x = _bounds.left + 4;

		for (uint i = 0; i < _name.size() && x < _bounds.right; i++) {
			byte c = translateChar(_name[i]);

			font->drawChar(_surface, c, x, y, fg);
			x += font->getCharWidth(c);
		}
	}
}

// ---------------------------------------------------------------------------
// Inventory::ScrollBar is the slider which shows if the inventory contains
// more objects than are visible on screen.
// ---------------------------------------------------------------------------

// NB: This class makes several references to ARRAYSIZE(_slots), but accessing
//     members of the enclosing class like that should be ok in C++11.

MacIndy3Gui::Inventory::ScrollBar::ScrollBar(int x, int y, int width, int height) : MacIndy3Gui::Widget(x, y, width, height) {
}

bool MacIndy3Gui::Inventory::ScrollBar::handleEvent(Common::Event &event) {
	if (!_enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	// Clicking on the scrollbar scrolls it to the top or the bottom, not
	// one page as one might suspect. Though you're rarely carrying enough
	// objects for this to make a difference.
	//
	// The direction depends on if you click above or below the handle.
	// Clicking on the handle also works, though the behavior strikes me
	// as a bit unintuitive. If you click on Y coordinate pos + 5, nothing
	// happens at all.

	if (_bounds.contains(event.mouse)) {
		int pos = _bounds.top + getHandlePosition();

		if (event.mouse.y <= pos + 4)
			_invOffset = 0;
		else if (event.mouse.y >= pos + 6)
			_invOffset = _invCount - ARRAYSIZE(_slots);

		_gui->setInventoryScrollOffset(_invOffset);
		setRedraw(true);
	}

	return false;
}

void MacIndy3Gui::Inventory::ScrollBar::setInventoryParameters(int invCount, int invOffset) {
	if (invOffset != _invOffset)
		setRedraw(true);

	if (invCount != _invCount && _invCount >= ARRAYSIZE(_slots))
		setRedraw(true);

	_invCount = invCount;
	_invOffset = invOffset;
}

void MacIndy3Gui::Inventory::ScrollBar::scroll(ScrollDirection dir) {
	int newOffset = _invOffset;
	int maxOffset = _invCount - ARRAYSIZE(_slots);

	if (dir == kScrollUp)
		newOffset--;
	else
		newOffset++;

	if (newOffset < 0)
		newOffset = 0;
	else if (newOffset > maxOffset)
		newOffset = maxOffset;

	if (newOffset != _invOffset) {
		_invOffset = newOffset;
		_gui->setInventoryScrollOffset(_invOffset);
		setRedraw(true);
	}
}

int MacIndy3Gui::Inventory::ScrollBar::getHandlePosition() {
	if (_invOffset == 0)
		return 0;

	// Hopefully this matches the original scroll handle position.

	int maxPos = _bounds.height() - 8;
	int maxOffset = _invCount - ARRAYSIZE(_slots);

	if (_invOffset >= maxOffset)
		return maxPos;

	return maxPos * _invOffset / maxOffset;
}

void MacIndy3Gui::Inventory::ScrollBar::reset() {
	MacIndy3Gui::Widget::reset();
	_invCount = 0;
	_invOffset = 0;
}

void MacIndy3Gui::Inventory::ScrollBar::draw() {
	if (!getRedraw())
		return;

	debug(1, "Inventory::Scrollbar: Drawing");

	Widget::draw();
	drawShadowFrame(_bounds, kBlack, kBackground);

	// The scrollbar handle is only drawn when there are enough inventory
	// objects to scroll.

	if (_enabled) {
		debug(1, "Inventory::Scrollbar: Drawing handle");

		int y = _bounds.top + getHandlePosition();

		// The height of the scrollbar handle never changes, regardless
		// of how many items you are carrying.
		drawShadowFrame(Common::Rect(_bounds.left, y, _bounds.right, y + 8), kWhite, kTransparency);
	}

	setRedraw(false);
	markScreenAsDirty(_bounds);
}

// ---------------------------------------------------------------------------
// Inventory::ScrollButton is the buttons used to scroll the inventory up and
// down. They're only responsible for drawing the filled part of the arrow,
// since the rest of the buttons are drawn by the Inventory widget itself.
// ---------------------------------------------------------------------------

MacIndy3Gui::Inventory::ScrollButton::ScrollButton(int x, int y, int width, int height, ScrollDirection direction) : MacIndy3Gui::Widget(x, y, width, height) {
	_direction = direction;
}

bool MacIndy3Gui::Inventory::ScrollButton::handleEvent(Common::Event &event) {
	if (!_enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	if (_bounds.contains(event.mouse)) {
		setRedraw(true);
		setTimer(WIDGET_TIMER_JIFFIES);
		return true;
	}

	return false;
}

bool MacIndy3Gui::Inventory::ScrollButton::handleMouseHeld(Common::Point &pressed, Common::Point &held) {
	if (!_enabled)
		return false;

	// The scroll button doesn't care if the mouse has moved outside while
	// being held.

	return _bounds.contains(pressed);
}

void MacIndy3Gui::Inventory::ScrollButton::timeOut() {
	// Nothing happens, but the button changes state.
	setRedraw(true);
}

void MacIndy3Gui::Inventory::ScrollButton::draw() {
	if (!getRedraw())
		return;

	debug(1, "Inventory::ScrollButton: Drawing [%d]", _direction);

	Widget::draw();

	const uint16 upArrow[] = {
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0080, 0x01C0, 0x03E0, 0x07F0,
		0x03E0, 0x03E0, 0x03E0, 0x03E0,
		0x0000, 0x0000, 0x0000, 0x0000
	};

	const uint16 downArrow[] = {
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x03E0, 0x03E0, 0x03E0,
		0x03E0, 0x07F0,	0x03E0, 0x01C0,
		0x0080, 0x0000, 0x0000, 0x0000
	};

	const uint16 *arrow = (_direction == kScrollUp) ? upArrow : downArrow;
	Color color = hasTimer() ? kBlack : kWhite;

	drawBitmap(_bounds, arrow, color);

	setRedraw(false);
	markScreenAsDirty(_bounds);
}

// ---------------------------------------------------------------------------
// The MacIndy3Gui class ties the whole thing together, mostly by delegating
// the work to the individual widgets.
// ---------------------------------------------------------------------------

MacIndy3Gui::MacIndy3Gui(ScummEngine *vm, Common::String resourceFile) :
	MacGui(vm, resourceFile), _visible(false) {

	// There is one widget for every verb in the game. Verbs include the
	// inventory widget and conversation options.

	Widget::_vm = _vm;
	Widget::_surface = _surface;
	Widget::_gui = this;

	_widgets[  1] = new Button(137, 312,  68, 18); // Open
	_widgets[  2] = new Button(137, 332,  68, 18); // Close
	_widgets[  3] = new Button( 67, 352,  68, 18); // Give
	_widgets[  4] = new Button(277, 332,  68, 18); // Turn on
	_widgets[  5] = new Button(277, 352,  68, 18); // Turn off
	_widgets[  6] = new Button( 67, 312,  68, 18); // Push
	_widgets[  7] = new Button( 67, 332,  68, 18); // Pull
	_widgets[  8] = new Button(277, 312,  68, 18); // Use
	_widgets[  9] = new Button(137, 352,  68, 18); // Look at
	_widgets[ 10] = new Button(207, 312,  68, 18); // Walk to
	_widgets[ 11] = new Button(207, 332,  68, 18); // Pick up
	_widgets[ 12] = new Button(207, 352,  68, 18); // What is
	_widgets[ 13] = new Button(347, 312,  68, 18); // Talk
	_widgets[ 14] = new Button( 97, 312, 121, 18); // Never mind.
	_widgets[ 32] = new Button(347, 332,  68, 18); // Travel
	_widgets[ 33] = new Button(347, 352,  68, 18); // To Indy
	_widgets[ 34] = new Button(347, 352,  68, 18); // To Henry
	_widgets[ 90] = new Button( 67, 292, 507, 18); // Travel 1
	_widgets[ 91] = new Button( 67, 312, 507, 18); // Travel 2
	_widgets[ 92] = new Button( 67, 332, 507, 18); // Travel 3
	_widgets[100] = new Button( 67, 292, 348, 18); // Sentence
	_widgets[101] = new Inventory(417, 292, 157, 78);
	_widgets[119] = new Button(324, 312,  91, 18); // Take this:
	_widgets[120] = new Button( 67, 292, 507, 18); // Converse 1
	_widgets[121] = new Button( 67, 312, 507, 18); // Converse 2
	_widgets[122] = new Button( 67, 332, 507, 18); // Converse 3
	_widgets[123] = new Button( 67, 352, 507, 18); // Converse 4
	_widgets[124] = new Button( 67, 352, 151, 18); // Converse 5
	_widgets[125] = new Button(423, 352, 151, 18); // Converse 6

	for (auto &it: _widgets)
		it._value->setVerbid(it._key);

	_dirtyRects.clear();
}

MacIndy3Gui::~MacIndy3Gui() {
	for (auto &it: _widgets)
		delete it._value;
}

void MacIndy3Gui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	if (_windowManager->getCursorType() == Graphics::kMacCursorCustom)
		return;

	const byte buf[15 * 15] = {
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 0, 3, 0, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3
	};

	width = height = 15;
	hotspotX = hotspotY = 7;
	animate = 0;

	_windowManager->pushCustomCursor(buf, width, height, hotspotX, hotspotY, 3);
}

const Graphics::Font *MacIndy3Gui::getFontByScummId(int32 id) {
	switch (id) {
	case 0:
		return getFont(kIndy3FontMedium);
	default:
		error("MacIndy3Gui::getFontByScummId: Invalid font id %d", id);
	}
}

void MacIndy3Gui::getFontParams(FontId fontId, int &id, int &size, int &slant) {
	// Indy 3 provides an "Indy" font in two sizes, 9 and 12, which are
	// used for the text boxes. The smaller font can be used for a
	// headline. The rest of the Indy 3 verb GUI uses Geneva.

	switch (fontId) {
	case FontId::kIndy3AboutFontHeader:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontItalic | Graphics::kMacFontBold | Graphics::kMacFontShadow | Graphics::kMacFontCondense;
		break;

	case FontId::kIndy3AboutFontBold:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		break;

	case FontId::kIndy3AboutFontRegular:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontRegular;
		break;

	case FontId::kIndy3FontSmall:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		break;

	case FontId::kIndy3FontMedium:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontRegular;
		break;

	case FontId::kIndy3VerbFontRegular:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontRegular;
		break;

	case FontId::kIndy3VerbFontBold:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontBold;
		break;

	case FontId::kIndy3VerbFontOutline:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontBold | Graphics::kMacFontOutline | Graphics::kMacFontCondense;
		break;

	default:
		error("MacIndy3Gui: getFontParams: Unknown font id %d", (int)fontId);
	}
}

bool MacIndy3Gui::handleMenu(int id, Common::String &name) {
	if (MacGui::handleMenu(id, name))
		return true;
	return false;
}

void MacIndy3Gui::showAboutDialog() {
	// The About window is not a a dialog resource. Its size appears to be
	// hard-coded (416x166), and it's drawn centered. The graphics are in
	// PICT 2000.

	int width = 416;
	int height = 166;
	int x = (640 - width) / 2;
	int y = (400 - height) / 2;

	Common::Rect bounds(x, y, x + width, y + height);
	SimpleWindow *window = openWindow(bounds);
	const Graphics::Surface *pict = loadPICT(2000);

	// For the background of the sprites to match the background of the
	// window, we have to move them at multiples of 4 pixels each step. We
	// cut out enough of the background so that each time they are drawn,
	// the visible remains of the previous frame is overdrawn.

	Graphics::Surface train = pict->getSubArea(Common::Rect(0, 0, 249, 93));

	Graphics::Surface trolley[3];

	for (int i = 0; i < 3; i++)
		trolley[i] = pict->getSubArea(Common::Rect(251 + 92 * i, 38, 335 + 92 * i, 93));

	clearAboutDialog(window);
	window->show();

	bool skip = false;

	Graphics::Surface *s = window->innerSurface();
	Common::Rect clipRect(2, 2, s->w - 4, s->h - 4);

	Common::Rect r1(22, 6, 382, 102);
	Common::Rect r2(22, 6, 382, 70);

	// Judging by recordings of Basilisk II, the internal frame rate is
	// 10 fps.

	int scene = 0;

	int trainX = -2;
	int trolleyX = width + 1;
	int trolleyFrame = 1;
	int trolleyFrameDelta = 1;
	int trolleyWaitFrames = 50;
	int textWaitFrames = 53;

	// Header texts aren't rendered correctly. Apparently the original does
	// its own shadowing by drawing the text twice, but that ends up
	// looking even worse. Perhaps I have to draw the text three times
	// once to fill it), or maybe our Mac text rendering just isn't as good
	// as it needs to be yet.

	const Graphics::Font *fontHeader = getFont(kIndy3AboutFontHeader);
	const Graphics::Font *fontBold = getFont(kIndy3AboutFontBold);
	const Graphics::Font *fontRegular = getFont(kIndy3AboutFontRegular);

	bool changeScene = false;

	while (true && !_vm->shouldQuit()) {
		switch (scene) {
		case 0:
			window->drawSprite(&train, trainX, 40, clipRect);
			trainX -= 12;

			if (trainX < -train.w)
				changeScene = true;

			break;

		case 1:
			if (--textWaitFrames == 0)
				changeScene = true;
			break;

		case 2:
		case 3:
			window->drawSprite(&trolley[trolleyFrame], trolleyX, 78, clipRect);

			if (scene == 2 && trolleyX == 161 && trolleyWaitFrames > 0) {
				trolleyWaitFrames--;
			} else {
				trolleyX -= 4;
				trolleyFrame += trolleyFrameDelta;
				if (trolleyFrame < 0 || trolleyFrame > 2) {
					trolleyFrame = 1;
					trolleyFrameDelta = -trolleyFrameDelta;
				}

				if (trolleyX < -85)
					changeScene = true;
			}

			break;
		}

		window->update();

		int status = delay(100);
		if (status == 2)
			break;

		if (status == 1 || changeScene) {
			changeScene = false;

			scene++;

			switch (scene) {
			case 1:
				clearAboutDialog(window);
				window->drawTextBox(r1);

				// These strings are part of the STRS resource,
				// but I don't know how to safely read them
				// from there.

				fontHeader->drawString(s, "Indiana Jones and the Last Crusade", r1.left - 2, 10, r1.width(), kBlack, Graphics::kTextAlignCenter);
				fontBold->drawString(s, "The Graphic Adventure", r1.left, 28, r1.width(), kBlack, Graphics::kTextAlignCenter);
				fontBold->drawString(s, "Mac 1.7 8/17/90, Interpreter version 5.1.6", r1.left, 55, r1.width(), kBlack, Graphics::kTextAlignCenter);
				fontRegular->drawString(s, "TM & \xA9 1990 LucasArts Entertainment Company.  All rights reserved.", r1.left + 1, 88, r1.width(), kBlack, Graphics::kTextAlignCenter);
				break;

			case 2:
				clearAboutDialog(window);
				window->drawTextBox(r2);
				break;

			case 3:
				// Don't clear. The trolley is still on screen
				// and only the text changes.
				window->drawTextBox(r2);
				break;

			case 4:
				clearAboutDialog(window);
				window->drawTextBox(r1);
				break;
			}

			window->update();

			if (scene > 7)
				break;
		}
	}

#if 0
	// The train seems to move at 12 pixels at a time. When recorded at
	// 30 fps, it moves every 3 frames. I'm going to assume that's the
	// intended frame rate.

	for (x = 0; x < train.w; x += 12) {
		window->drawSprite(&train, 2 - x, 40, clipRect);
		window->update();

		if (delay(100)) {
			skip = true;
			break;
		}
	}

	if (skip) {
		clearAboutDialog(window);
		window->update();
		skip = false;
	}

	// Page 2 - First text box

	window->drawTextBox(r1);

	window->update();

	if (delay(5333))
		skip = true;

	clearAboutDialog(window);

	// Page 3 - The trolley

	// The trolley moves 4 pixels at a time every three frames, and the
	// animation frame changes every time it moves. But the animation cycle
	// seems semi-random to me. I think this is a case of not caring too
	// deeply about pixel accuracy as long as everything stops and starts
	// at the right time, moves at the right speed, and stops and starts
	// on the correct animation frame.

	window->drawTextBox(r2);

	int trolleyFrame = 1;
	int trolleyFrameDelta = 1;

	for (x = width + 1; !skip && x >= -85; x -= 4) {
		window->drawSprite(&trolley[trolleyFrame], x, 78, clipRect);
		window->update();

		if (x == 161) {
			if (delay(5000)) {
				skip = true;
				break;
			}

			window->drawTextBox(r2);
		}

		trolleyFrame += trolleyFrameDelta;
		if (trolleyFrame < 0 || trolleyFrame > 2) {
			trolleyFrame = 1;
			trolleyFrameDelta = -trolleyFrameDelta;
		}

		if (delay(100)) {
			skip = true;
			break;
		}
	}

	if (delay(5333))
		skip = true;

	window->drawTextBox(r1);
	window->update();

	if (skip) {
		clearAboutDialog(window);
		window->update();
		skip = false;
	}
#endif

	_windowManager->pushCursor(Graphics::kMacCursorArrow, nullptr);

	bool shouldQuit = false;
	bool shouldQuitEngine = false;

	while (!shouldQuit) {
		Common::Event event;

		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				shouldQuit = true;
				shouldQuitEngine = true;
				break;

			case Common::EVENT_LBUTTONDOWN:
				shouldQuit = true;
				break;

			default:
				break;
			}
		}

		_system->updateScreen();
		_system->delayMillis(10);
	}

	_windowManager->popCursor();

	delete window;

	if (shouldQuitEngine)
		debug("Quit everything");
}

void MacIndy3Gui::clearAboutDialog(SimpleWindow *window) {
	Graphics::Surface *s = window->innerSurface();

	window->fillPattern(Common::Rect(2, 2, s->w - 2, 132), 0x8020);
	window->fillPattern(Common::Rect(2, 130, s->w - 2, 133), 0xA5A5);
	window->fillPattern(Common::Rect(2, 133, s->w - 2, 136), 0xFFFF);
	window->fillPattern(Common::Rect(2, 136, s->w - 2, s->h - 4), 0xA5A5);
}

// Before the GUI rewrite, the scroll offset was saved in variable 67. Let's
// continue that tradition, just in case. If nothing else, it gives us an easy
// way to still store it in savegames.

int MacIndy3Gui::getInventoryScrollOffset() const {
	return _vm->VAR(67);
}

void MacIndy3Gui::setInventoryScrollOffset(int n) const {
	_vm->VAR(67) = n;
}

bool MacIndy3Gui::isVerbGuiAllowed() const {
	// The GUI is only allowed if the verb area has the expected size. That
	// really seems to be all that's needed.

	VirtScreen *vs = &_vm->_virtscr[kVerbVirtScreen];
	if (vs->topline != 144 || vs->h != 56)
		return false;

	return true;
}

bool MacIndy3Gui::isVerbGuiActive() const {
	// The visibility flag may not have been updated yet, so better check
	// that the GUI is still allowed.

	return _visible && isVerbGuiAllowed();
}

void MacIndy3Gui::reset() {
	_visible = false;

	for (auto &it: _widgets)
		it._value->reset();
}

void MacIndy3Gui::resetAfterLoad() {
	reset();

	// In the DOS version, verb ID 102-106 were used for the visible
	// inventory items, and 107-108 for inventory arrow buttons. In the
	// Macintosh version, the entire inventory widget is verb ID 101.
	//
	// In old savegames, the DOS verb IDs may still be present, and have
	// to be removed.

	for (int i = 0; i < _vm->_numVerbs; i++) {
		if (_vm->_verbs[i].verbid >= 102 && _vm->_verbs[i].verbid <= 108)
			_vm->killVerb(i);
	}
}

void MacIndy3Gui::update(int delta) {
	if (isVerbGuiAllowed() && updateVerbs(delta)) {
		if (!_visible)
			show();

		updateMouseHeldTimer(delta);
		drawVerbs();
	} else {
		if (_visible)
			hide();
	}

	copyDirtyRectsToScreen();
}

bool MacIndy3Gui::updateVerbs(int delta) {
	// Tentatively mark the verb widgets for removal. Any widget that wants
	// to stay has to say so.

	for (auto &it: _widgets) {
		VerbWidget *w = it._value;

		if (delta > 0)
			w->updateTimer(delta);

		w->threaten();
	}

	bool hasActiveVerbs = false;

	// Collect all active verbs. Verb slot 0 is special, apparently, so we
	// don't look at that one.

	for (int i = 1; i < _vm->_numVerbs; i++) {
		VerbSlot *vs = &_vm->_verbs[i];

		if (!vs->saveid && vs->curmode && vs->verbid) {
			VerbWidget *w = _widgets.getValOrDefault(vs->verbid);

			if (w) {
				w->updateVerb(i);
				hasActiveVerbs = true;
			} else {
				const byte *ptr = _vm->getResourceAddress(rtVerb, i);
				byte buf[270];
				_vm->convertMessageToString(ptr, buf, sizeof(buf));
				warning("MacIndy3Gui: Unknown verb: %d %s", vs->verbid, buf);
			}
		}
	}

	return hasActiveVerbs;
}

void MacIndy3Gui::updateMouseHeldTimer(int delta) {
	if (delta > 0 && _leftButtonIsPressed) {
		_timer -= delta;

		if (_timer <= 0) {
			debug(2, "MacIndy3Gui: Left button still down");

			_timer = REPEAT_TIMER_JIFFIES;

			for (auto &it: _widgets) {
				if (it._value->handleMouseHeld(_leftButtonPressed, _leftButtonHeld))
					break;
			}
		}
	}
}

void MacIndy3Gui::drawVerbs() {
	// The possible verbs overlap each other. Remove the dead ones first, then draw the live ones.

	for (auto &it: _widgets) {
		VerbWidget *w = it._value;

		if (w->isDying() && w->isVisible()) {
			w->undraw();
			w->reset();
		}
	}

	for (auto &it: _widgets) {
		VerbWidget *w = it._value;

		if (w->hasVerb())
			w->draw();
	}
}

bool MacIndy3Gui::handleEvent(Common::Event &event) {
	if (MacGui::handleEvent(event))
		return true;

	if (!isVerbGuiActive() || _vm->_userPut <= 0)
		return false;

	if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (!_leftButtonIsPressed) {
			debug(2, "MacIndy3Gui: Left button down");

			_leftButtonIsPressed = true;
			_leftButtonPressed = event.mouse;
			_leftButtonHeld = event.mouse;
			_timer = REPEAT_TIMER_JIFFIES;
		}
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		if (_leftButtonIsPressed) {
			debug(2, "MacIndy3Gui: Left button up");

			_leftButtonIsPressed = false;
			_timer = 0;
		}
	} else if (event.type == Common::EVENT_MOUSEMOVE) {
		if (_leftButtonIsPressed)
			_leftButtonHeld = event.mouse;
	}

	// It probably doesn't make much of a difference, but if a widget
	// responds to an event, and marks itself as wanting to be redraw,
	// we do that redrawing immediately, not on the next update.

	for (auto &it: _widgets) {
		Widget *w = it._value;

		if (w->handleEvent(event)) {
			if (w->getRedraw()) {
				w->draw();
				copyDirtyRectsToScreen();
			}
			return true;
		}
	}

	return false;
}

void MacIndy3Gui::show() {
	if (_visible)
		return;

	debug(1, "MacIndy3Gui: Showing");

	_visible = true;

	_surface->fillRect(Common::Rect(0, 288, 640, 289), kBlack);
	_surface->fillRect(Common::Rect(0, 373, 640, 400), kBlack);

	fill(Common::Rect(0, 290, 640, 373));

	const uint16 ulCorner[] = { 0xF000, 0xC000, 0x8000, 0x8000 };
	const uint16 urCorner[] = { 0xF000, 0x3000, 0x1000, 0x1000 };
	const uint16 llCorner[] = { 0x8000, 0x8000, 0xC000, 0xF000 };
	const uint16 lrCorner[] = { 0x1000, 0x1000, 0x3000, 0xF000 };

	drawBitmap(Common::Rect(  0, 290,   4, 294), ulCorner, kBlack);
	drawBitmap(Common::Rect(636, 290, 640, 294), urCorner, kBlack);
	drawBitmap(Common::Rect(  0, 369,   4, 373), llCorner, kBlack);
	drawBitmap(Common::Rect(636, 369, 640, 373), lrCorner, kBlack);

	markScreenAsDirty(Common::Rect(0, 288, 640, 400));
}

void MacIndy3Gui::hide() {
	if (!_visible)
		return;

	debug(1, "MacIndy3Gui: Hiding");

	_leftButtonIsPressed = false;
	_timer = 0;

	reset();

	if (isVerbGuiAllowed()) {
		_surface->fillRect(Common::Rect(0, 288, 640, 400), kBlack);
		markScreenAsDirty(Common::Rect(0, 288, 640, 400));
	}
}

void MacIndy3Gui::markScreenAsDirty(Common::Rect r) {
	// As long as we always call this with the most encompassing rect
	// first, it is trivial to filter out unnecessary calls.

	for (uint i = 0; i < _dirtyRects.size(); i++) {
		if (_dirtyRects[i].contains(r))
			return;
	}

	_dirtyRects.push_back(r);
}

void MacIndy3Gui::copyDirtyRectsToScreen() {
	for (uint i = 0; i < _dirtyRects.size(); i++) {
		_system->copyRectToScreen(
			_surface->getBasePtr(_dirtyRects[i].left, _dirtyRects[i].top),
			_surface->pitch,
			_dirtyRects[i].left, _dirtyRects[i].top,
			_dirtyRects[i].width(), _dirtyRects[i].height());
	}

	_dirtyRects.clear();
}

void MacIndy3Gui::fill(Common::Rect r) const {
	int pitch = _surface->pitch;

	// Fill the screen with either gray of a checkerboard pattern.

	if (_vm->_renderMode == Common::kRenderMacintoshBW) {
		byte *row = (byte *)_surface->getBasePtr(r.left, r.top);

		for (int y = r.top; y < r.bottom; y++) {
			byte *ptr = row;
			for (int x = r.left; x < r.right; x++)
				*ptr++ = ((x + y) & 1) ? kWhite : kBlack;
			row += pitch;
		}
	} else
		_surface->fillRect(r, kLightGray);
}

void MacIndy3Gui::drawBitmap(Common::Rect r, const uint16 *bitmap, Color color) const {
	byte *ptr = (byte *)_surface->getBasePtr(r.left, r.top);
	int pitch = _surface->pitch;

	assert(r.width() <= 16);

	for (int y = 0; y < r.height(); y++) {
		uint16 bit = 0x8000;
		for (int x = 0; x < r.width(); x++) {
			if (bitmap[y] & bit)
				ptr[x] = color;
			bit >>= 1;
		}
		ptr += pitch;
	}
}

} // End of namespace Scumm
