/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/freescape/gfx.h"

#include "engines/util.h"

#include "common/config-manager.h"

#include "graphics/renderer.h"
#include "graphics/surface.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
#include "graphics/opengl/context.h"
#endif

#include "math/glmath.h"

namespace Freescape {


Renderer::Renderer(OSystem *system)
		: _system(system),
		  _font(nullptr) {

	_currentPixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	_originalPixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	_palettePixelFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);

}

Renderer::~Renderer() {}

void Renderer::initFont(const Graphics::Surface *surface) {
	_font = createTexture(surface);
}

void Renderer::freeFont() {
	if (_font) {
		freeTexture(_font);
		_font = nullptr;
	}
}

Graphics::Surface *Renderer::convertFromPalette(Graphics::PixelBuffer *rawsurf) {
	Graphics::Surface * surf = new Graphics::Surface();
	surf->create(kOriginalWidth, kOriginalHeight, _originalPixelFormat);
	surf->copyRectToSurface(rawsurf->getRawBuffer(), surf->w, 0, 0, surf->w, surf->h);
	surf->convertToInPlace(_currentPixelFormat, _palette->getRawBuffer());
	return surf;
}

Common::Rect Renderer::getFontCharacterRect(uint8 character) {
	uint index = 0;

	if (character == ' ')
		index = 0;
	else if (character >= '0' && character <= '9')
		index = 1 + character - '0';
	else if (character >= 'A' && character <= 'Z')
		index = 1 + 10 + character - 'A';
	else if (character == '|')
		index = 1 + 10 + 26;
	else if (character == '/')
		index = 2 + 10 + 26;
	else if (character == ':')
		index = 3 + 10 + 26;

	return Common::Rect(16 * index, 0, 16 * (index + 1), 32);
}

Common::Rect Renderer::viewport() const {
	return _screenViewport;
}

Common::Rect Renderer::rviewport() const {
	return _rscreenViewport;
}

void Renderer::computeScreenViewport() {
	int32 screenWidth = _system->getWidth();
	int32 screenHeight = _system->getHeight();
	int32 tmargin = 27;
	int32 vmargin = 40;
	int32 bmargin = 90;

	//assert(0);

	/*if (ConfMan.getBool("widescreen_mod")) {
		_screenViewport = Common::Rect(screenWidth, screenHeight);
	} else {*/
		// Aspect ratio correction
		int32 viewportWidth = MIN<int32>(screenWidth, screenHeight * kOriginalWidth / kOriginalHeight);
		int32 viewportHeight = MIN<int32>(screenHeight, screenWidth * kOriginalHeight / kOriginalWidth);
		_screenViewport = Common::Rect(viewportWidth, viewportHeight);
		_rscreenViewport = Common::Rect(vmargin, tmargin, _screenViewport.right - vmargin, _screenViewport.bottom - bmargin);

		// Pillarboxing
		_screenViewport.translate((screenWidth - viewportWidth) / 2,
			(screenHeight - viewportHeight) / 2);
	//}
}

Renderer *createRenderer(OSystem *system) {
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	Graphics::RendererType desiredRendererType = Graphics::kRendererTypeTinyGL; //Graphics::parseRendererTypeCode(rendererConfig);
	Graphics::RendererType matchingRendererType = Graphics::kRendererTypeTinyGL; //Graphics::getBestMatchingAvailableRendererType(desiredRendererType);

	bool isAccelerated = 0; //matchingRendererType != Graphics::kRendererTypeTinyGL;

	uint width = Renderer::kOriginalWidth;
	uint height = Renderer::kOriginalHeight;

	if (isAccelerated) {
		initGraphics3d(width, height);
	} else {
		initGraphics(width, height, &pixelFormat);
	}

/*#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
	bool backendCapableOpenGL = g_system->hasFeature(OSystem::kFeatureOpenGLForGame);
#endif

#if defined(USE_OPENGL_GAME)
	// Check the OpenGL context actually supports shaders
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGLShaders && !OpenGLContext.shadersSupported) {
		matchingRendererType = Graphics::kRendererTypeOpenGL;
	}
#endif*/

	if (matchingRendererType != desiredRendererType && desiredRendererType != Graphics::kRendererTypeDefault) {
		// Display a warning if unable to use the desired renderer
		warning("Unable to create a '%s' renderer", rendererConfig.c_str());
	}
/*
#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGLShaders) {
		return CreateGfxOpenGLShader(system);
	}
#endif
#if defined(USE_OPENGL_GAME) && !defined(USE_GLES2)
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGL) {
		return CreateGfxOpenGL(system);
	}
#endif*/
	if (matchingRendererType == Graphics::kRendererTypeTinyGL) {
		return CreateGfxTinyGL(system);
	}

	error("Unable to create a '%s' renderer", rendererConfig.c_str());
}

void Renderer::renderDrawable(Drawable *drawable, Window *window) {
	// if (drawable->isConstrainedToWindow()) {
	// 	selectTargetWindow(window, drawable->is3D(), drawable->isScaled());
	// } else {
	// 	selectTargetWindow(nullptr, drawable->is3D(), drawable->isScaled());
	// }
	// drawable->draw();
}

void Renderer::renderDrawableOverlay(Drawable *drawable, Window *window) {
	// // Overlays are always 2D
	// if (drawable->isConstrainedToWindow()) {
	// 	selectTargetWindow(window, drawable->is3D(), drawable->isScaled());
	// } else {
	// 	selectTargetWindow(nullptr, drawable->is3D(), drawable->isScaled());
	// }
	// drawable->drawOverlay();
}

void Renderer::renderWindow(Window *window) {
	renderDrawable(window, window);
}

void Renderer::renderWindowOverlay(Window *window) {
	renderDrawableOverlay(window, window);
}

Drawable::Drawable() :
		_isConstrainedToWindow(true),
		_is3D(false),
		_scaled(true) {
}

Common::Point Window::getCenter() const {
	Common::Rect frame = getPosition();

	return Common::Point((frame.left + frame.right) / 2, (frame.top + frame.bottom) / 2);
}

Common::Point Window::screenPosToWindowPos(const Common::Point &screen) const {
	Common::Rect frame = getPosition();

	return Common::Point(screen.x - frame.left, screen.y - frame.top);
}

Common::Point Window::scalePoint(const Common::Point &screen) const {
	Common::Rect viewport = getPosition();
	Common::Rect originalViewport = getOriginalPosition();

	Common::Point scaledPosition = screen;
	scaledPosition.x -= viewport.left;
	scaledPosition.y -= viewport.top;
	scaledPosition.x = CLIP<int16>(scaledPosition.x, 0, viewport.width());
	scaledPosition.y = CLIP<int16>(scaledPosition.y, 0, viewport.height());

	if (_scaled) {
		scaledPosition.x *= originalViewport.width() / (float) viewport.width();
		scaledPosition.y *= originalViewport.height() / (float) viewport.height();
	}

	return scaledPosition;
}

FrameLimiter::FrameLimiter(OSystem *system, const uint framerate) :
	_system(system),
	_speedLimitMs(0),
	_startFrameTime(0) {
	// The frame limiter is disabled when vsync is enabled.
	_enabled = !_system->getFeatureState(OSystem::kFeatureVSync) && framerate != 0;

	if (_enabled) {
		_speedLimitMs = 1000 / CLIP<uint>(framerate, 0, 100);
	}
}

void FrameLimiter::startFrame() {
	_startFrameTime = _system->getMillis();
}

void FrameLimiter::delayBeforeSwap() {
	uint endFrameTime = _system->getMillis();
	uint frameDuration = endFrameTime - _startFrameTime;

	if (_enabled && frameDuration < _speedLimitMs) {
		_system->delayMillis(_speedLimitMs - frameDuration);
	}
}

const Graphics::PixelFormat Texture::getRGBAPixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
}
} // End of namespace Myst3
