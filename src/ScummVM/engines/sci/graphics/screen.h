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

#ifndef SCI_GRAPHICS_SCREEN_H
#define SCI_GRAPHICS_SCREEN_H

#include "sci/sci.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/view.h"

#include "graphics/sjis.h"

namespace Sci {

#define SCI_SCREEN_UPSCALEDMAXHEIGHT 200
#define SCI_SCREEN_UPSCALEDMAXWIDTH  320

enum GfxScreenUpscaledMode {
	GFX_SCREEN_UPSCALED_DISABLED	= 0,
	GFX_SCREEN_UPSCALED_480x300     = 1,
	GFX_SCREEN_UPSCALED_640x400		= 2,
	GFX_SCREEN_UPSCALED_640x440		= 3,
	GFX_SCREEN_UPSCALED_640x480		= 4
};

enum GfxScreenMasks {
	GFX_SCREEN_MASK_VISUAL		= 1,
	GFX_SCREEN_MASK_PRIORITY	= 2,
	GFX_SCREEN_MASK_CONTROL		= 4,
	GFX_SCREEN_MASK_DISPLAY		= 8, // not official sierra sci, only used internally
	GFX_SCREEN_MASK_ALL			= GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY|GFX_SCREEN_MASK_CONTROL
};

enum {
	DITHERED_BG_COLORS_SIZE = 256
};

/**
 * Screen class, actually creates 3 (4) screens internally:
 * - visual/display (for the user),
 * - priority (contains priority information) and
 * - control (contains control information).
 * Handles all operations to it and copies parts of visual/display screen to
 * the actual screen, so the user can really see it.
 */
class GfxScreen {
public:
	GfxScreen(ResourceManager *resMan);
	~GfxScreen();

	uint16 getWidth() { return _width; }
	uint16 getHeight() { return _height; }
	uint16 getScriptWidth() { return _scriptWidth; }
	uint16 getScriptHeight() { return _scriptHeight; }
	uint16 getDisplayWidth() { return _displayWidth; }
	uint16 getDisplayHeight() { return _displayHeight; }
	byte getColorWhite() { return _colorWhite; }
	byte getColorDefaultVectorData() { return _colorDefaultVectorData; }

	void clearForRestoreGame();
	void copyToScreen();
	void copyFromScreen(byte *buffer);
	void kernelSyncWithFramebuffer();
	void copyRectToScreen(const Common::Rect &rect);
	void copyDisplayRectToScreen(const Common::Rect &rect);
	void copyRectToScreen(const Common::Rect &rect, int16 x, int16 y);

	// calls to code pointers
	void inline vectorAdjustCoordinate (int16 *x, int16 *y) {
		(this->*_vectorAdjustCoordinatePtr)(x, y);
	}
	void inline vectorAdjustLineCoordinates (int16 *left, int16 *top, int16 *right, int16 *bottom, byte drawMask, byte color, byte priority, byte control) {
		(this->*_vectorAdjustLineCoordinatesPtr)(left, top, right, bottom, drawMask, color, priority, control);
	}
	byte inline vectorIsFillMatch (int16 x, int16 y, byte screenMask, byte t_color, byte t_pri, byte t_con, bool isEGA) {
		return (this->*_vectorIsFillMatchPtr)(x, y, screenMask, t_color, t_pri, t_con, isEGA);
	}
	void inline vectorPutPixel(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
		(this->*_vectorPutPixelPtr)(x, y, drawMask, color, priority, control);
	}
	void inline vectorPutLinePixel(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
		(this->*_vectorPutLinePixelPtr)(x, y, drawMask, color, priority, control);
	}
	byte inline vectorGetVisual(int16 x, int16 y) {
		return (this->*_vectorGetPixelPtr)(_visualScreen, x, y);
	}
	byte inline vectorGetPriority(int16 x, int16 y) {
		return (this->*_vectorGetPixelPtr)(_priorityScreen, x, y);
	}
	byte inline vectorGetControl(int16 x, int16 y) {
		return (this->*_vectorGetPixelPtr)(_controlScreen, x, y);
	}


	void inline putPixel(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
		(this->*_putPixelPtr)(x, y, drawMask, color, priority, control);
	}

	byte inline getVisual(int16 x, int16 y) {
		return (this->*_getPixelPtr)(_visualScreen, x, y);
	}
	byte inline getPriority(int16 x, int16 y) {
		return (this->*_getPixelPtr)(_priorityScreen, x, y);
	}
	byte inline getControl(int16 x, int16 y) {
		return (this->*_getPixelPtr)(_controlScreen, x, y);
	}

	byte getDrawingMask(byte color, byte prio, byte control);
	//void putPixel(int16 x, int16 y, byte drawMask, byte color, byte prio, byte control);
	void putFontPixel(int16 startingY, int16 x, int16 y, byte color);
	void putPixelOnDisplay(int16 x, int16 y, byte color);
	void drawLine(Common::Point startPoint, Common::Point endPoint, byte color, byte prio, byte control);
	void drawLine(int16 left, int16 top, int16 right, int16 bottom, byte color, byte prio, byte control) {
		drawLine(Common::Point(left, top), Common::Point(right, bottom), color, prio, control);
	}

	GfxScreenUpscaledMode getUpscaledHires() const {
		return _upscaledHires;
	}

	bool isUnditheringEnabled() const {
		return _unditheringEnabled;
	}
	void enableUndithering(bool flag);

	void putKanjiChar(Graphics::FontSJIS *commonFont, int16 x, int16 y, uint16 chr, byte color);

	int bitsGetDataSize(Common::Rect rect, byte mask);
	void bitsSave(Common::Rect rect, byte mask, byte *memoryPtr);
	void bitsGetRect(byte *memoryPtr, Common::Rect *destRect);
	void bitsRestore(byte *memoryPtr);

	void scale2x(const byte *src, byte *dst, int16 srcWidth, int16 srcHeight, byte bytesPerPixel = 1);

	void adjustToUpscaledCoordinates(int16 &y, int16 &x, Sci32ViewNativeResolution viewScalingType = SCI_VIEW_NATIVERES_NONE);
	void adjustBackUpscaledCoordinates(int16 &y, int16 &x, Sci32ViewNativeResolution viewScalingType = SCI_VIEW_NATIVERES_NONE);

	void dither(bool addToFlag);

	// Force a color combination as a dithered color
	void ditherForceDitheredColor(byte color);
	int16 *unditherGetDitheredBgColors();

	void debugShowMap(int mapNo);

	int _picNotValid; // possible values 0, 1 and 2
	int _picNotValidSci11; // another variable that is used by kPicNotValid in sci1.1

	int16 kernelPicNotValid(int16 newPicNotValid);
	void kernelShakeScreen(uint16 shakeCount, uint16 direction);

	void setFontIsUpscaled(bool isUpscaled) { _fontIsUpscaled = isUpscaled; }
	bool fontIsUpscaled() const { return _fontIsUpscaled; }

private:
	uint16 _width;
	uint16 _height;
	uint _pixels;
	uint16 _scriptWidth;
	uint16 _scriptHeight;
	uint16 _displayWidth;
	uint16 _displayHeight;
	uint _displayPixels;

	byte _colorWhite;
	byte _colorDefaultVectorData;

	void bitsRestoreScreen(Common::Rect rect, byte *&memoryPtr, byte *screen, uint16 screenWidth);
	void bitsRestoreDisplayScreen(Common::Rect rect, byte *&memoryPtr);
	void bitsSaveScreen(Common::Rect rect, byte *screen, uint16 screenWidth, byte *&memoryPtr);
	void bitsSaveDisplayScreen(Common::Rect rect, byte *&memoryPtr);

	void setVerticalShakePos(uint16 shakePos);

	/**
	 * If this flag is true, undithering is enabled, otherwise disabled.
	 */
	bool _unditheringEnabled;
	int16 _ditheredPicColors[DITHERED_BG_COLORS_SIZE];

	// These screens have the real resolution of the game engine (320x200 for
	// SCI0/SCI1/SCI11 games, 640x480 for SCI2 games). SCI0 games will be
	// dithered in here at any time.
	byte *_visualScreen;
	byte *_priorityScreen;
	byte *_controlScreen;

	/**
	 * This screen is the one that is actually displayed to the user. It may be
	 * 640x400 for japanese SCI1 games. SCI0 games may be undithered in here.
	 * Only read from this buffer for Save/ShowBits usage.
	 */
	byte *_displayScreen;

	ResourceManager *_resMan;

	/**
	 * Pointer to the currently active screen (changing it only required for
	 * debug purposes).
	 */
	byte *_activeScreen;

	/**
	 * This variable defines, if upscaled hires is active and what upscaled mode
	 * is used.
	 */
	GfxScreenUpscaledMode _upscaledHires;

	/**
	 * This here holds a translation for vertical+horizontal coordinates between native
	 * (visual) and actual (display) screen.
	 */
	int16 _upscaledHeightMapping[SCI_SCREEN_UPSCALEDMAXHEIGHT + 1];
	int16 _upscaledWidthMapping[SCI_SCREEN_UPSCALEDMAXWIDTH + 1];

	/**
	 * This defines whether or not the font we're drawing is already scaled
	 * to the screen size (and we therefore should not upscale it ourselves).
	 */
	bool _fontIsUpscaled;

	// dynamic code
	void (GfxScreen::*_vectorAdjustCoordinatePtr) (int16 *x, int16 *y);
	void vectorAdjustCoordinateNOP (int16 *x, int16 *y);
	void vectorAdjustCoordinate480x300Mac (int16 *x, int16 *y);

	void (GfxScreen::*_vectorAdjustLineCoordinatesPtr) (int16 *left, int16 *top, int16 *right, int16 *bottom, byte drawMask, byte color, byte priority, byte control);
	void vectorAdjustLineCoordinatesNOP (int16 *left, int16 *top, int16 *right, int16 *bottom, byte drawMask, byte color, byte priority, byte control);
	void vectorAdjustLineCoordinates480x300Mac (int16 *left, int16 *top, int16 *right, int16 *bottom, byte drawMask, byte color, byte priority, byte control);
	
	byte (GfxScreen::*_vectorIsFillMatchPtr) (int16 x, int16 y, byte screenMask, byte t_color, byte t_pri, byte t_con, bool isEGA);
	byte vectorIsFillMatchNormal (int16 x, int16 y, byte screenMask, byte t_color, byte t_pri, byte t_con, bool isEGA);
	byte vectorIsFillMatch480x300Mac (int16 x, int16 y, byte screenMask, byte t_color, byte t_pri, byte t_con, bool isEGA);

	void (GfxScreen::*_vectorPutPixelPtr) (int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);
	void vectorPutPixel480x300Mac (int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);

	void (GfxScreen::*_vectorPutLinePixelPtr) (int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);
	void vectorPutLinePixel480x300Mac (int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);

	byte (GfxScreen::*_vectorGetPixelPtr) (byte *screen, int16 x, int16 y);

	void (GfxScreen::*_putPixelPtr) (int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);
	void putPixelNormal (int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);
	void putPixelDisplayUpscaled (int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);
	void putPixelAllUpscaled (int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);

	byte (GfxScreen::*_getPixelPtr) (byte *screen, int16 x, int16 y);
	byte getPixelNormal (byte *screen, int16 x, int16 y);
	byte getPixelUpscaled (byte *screen, int16 x, int16 y);

	// pixel helper
	void putScaledPixelOnScreen(byte *screen, int16 x, int16 y, byte color);
};

} // End of namespace Sci

#endif
