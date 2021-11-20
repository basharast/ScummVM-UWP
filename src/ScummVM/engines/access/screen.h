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

#ifndef ACCESS_SCREEN_H
#define ACCESS_SCREEN_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/stream.h"
#include "access/asurface.h"

namespace Access {

class AccessEngine;

#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

struct ScreenSave {
	int _clipWidth;
	int _clipHeight;
	int _windowXAdd;
	int _windowYAdd;
	Common::Point _scroll;
	int _scrollCol;
	int _scrollRow;
	Common::Point _bufferStart;
	int _screenYOff;
};

class Screen : public ASurface {
private:
	AccessEngine *_vm;
	byte _tempPalette[PALETTE_SIZE];
	byte _rawPalette[PALETTE_SIZE];
	byte _savedPalettes[2][PALETTE_SIZE];
	int _savedPaletteCount;
	int _vesaCurrentWin;
	int _currentPanel;
	Common::Point _msVirtualOffset;
	Common::Point _virtualOffsetsTable[4];
	bool _hideFlag;
	ScreenSave _screenSave;
	int _startCycle;
	int _cycleStart;
	int _endCycle;
	Common::List<Common::Rect> _dirtyRects;

	void updatePalette();

	void mergeDirtyRects();

	bool unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2);
public:
	int _vesaMode;
	int _startColor, _numColors;
	Common::Point _bufferStart;
	int _windowXAdd, _windowYAdd;
	int _screenYOff;
	byte _manPal[0x60];
	byte _scaleTable1[256];
	byte _scaleTable2[256];
	int _vWindowWidth;
	int _vWindowHeight;
	int _vWindowBytesWide;
	int _bufferBytesWide;
	int _vWindowLinesTall;
	bool _screenChangeFlag;
	bool _fadeIn;
public:
	virtual void copyBlock(ASurface *src, const Common::Rect &bounds);

	virtual void restoreBlock();

	virtual void drawRect();

	virtual void drawBox();

	virtual void transBlitFrom(ASurface *src, const Common::Point &destPos);

	virtual void transBlitFrom(ASurface *src, const Common::Rect &bounds);

	virtual void blitFrom(const Graphics::Surface &src);

	virtual void copyBuffer(Graphics::Surface *src);

	virtual void addDirtyRect(const Common::Rect &r);
public:
	Screen(AccessEngine *vm);

	virtual ~Screen() {}

	void setDisplayScan();

	void setPanel(int num);

	/**
	 * Update the underlying screen
	 */
	void updateScreen();

	/**
	 * Fade out screen
	 */
	void forceFadeOut();

	/**
	 * Fade in screen
	 */
	void forceFadeIn();

	void fadeOut() { forceFadeOut(); }
	void fadeIn() { forceFadeIn(); }
	void clearScreen();

	/**
	 * Set the initial palette
	 */
	void setInitialPalettte();

	/**
	 * Set icon palette
	 */
	void setIconPalette();

	/**
	 * Set Tex palette (Martian Memorandum)
	 */
	void setManPalette();

	void loadPalette(int fileNum, int subfile);

	void setPalette();

	void loadRawPalette(Common::SeekableReadStream *stream);

	void savePalette();

	void restorePalette();

	void getPalette(byte *pal);

	void flashPalette(int count);

	/**
	 * Copy a buffer to the screen
	 */
	void copyBuffer(const byte *data);

	void setBufferScan();

	void setScaleTable(int scale);

	/**
	 * Save all the screen display state variables
	 */
	void saveScreen();

	/**
	 * Restores previously saved screen display state variables
	 */
	void restoreScreen();

	void setPaletteCycle(int startCycle, int endCycle, int timer);

	void cyclePaletteForward();

	void cyclePaletteBackwards();
};

} // End of namespace Access

#endif /* ACCESS_SCREEN_H */
