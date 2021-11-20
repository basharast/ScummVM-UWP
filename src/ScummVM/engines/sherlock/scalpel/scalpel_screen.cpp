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

#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel.h"

namespace Sherlock {

namespace Scalpel {

ScalpelScreen::ScalpelScreen(SherlockEngine *vm) : Screen(vm) {
}

void ScalpelScreen::makeButton(const Common::Rect &bounds, int textX,
		const Common::String &buttonText, bool textContainsHotkey) {

	Surface &bb = *_backBuffer;
	bb.fillRect(Common::Rect(bounds.left, bounds.top, bounds.right, bounds.top + 1), BUTTON_TOP);
	bb.fillRect(Common::Rect(bounds.left, bounds.top, bounds.left + 1, bounds.bottom), BUTTON_TOP);
	bb.fillRect(Common::Rect(bounds.right - 1, bounds.top, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	bb.fillRect(Common::Rect(bounds.left + 1, bounds.bottom - 1, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	bb.fillRect(Common::Rect(bounds.left + 1, bounds.top + 1, bounds.right - 1, bounds.bottom - 1), BUTTON_MIDDLE);

	buttonPrint(Common::Point(textX, bounds.top), COMMAND_FOREGROUND, false, buttonText, textContainsHotkey);
}

// ButtonText is supposed to have its hotkey as a prefix. The hotkey will get highlighted.
void ScalpelScreen::buttonPrint(const Common::Point &pt, uint color, bool slamIt,
		const Common::String &buttonText, bool textContainsHotkey) {
	int xStart = pt.x;
	int skipTextOffset = textContainsHotkey ? +1 : 0; // skip first char in case text contains hotkey

	// Center text around given x-coordinate
	if (textContainsHotkey) {
		xStart -= (stringWidth(Common::String(buttonText.c_str() + 1)) / 2);
	} else {
		xStart -= (stringWidth(buttonText) / 2);
	}

	if (color == COMMAND_FOREGROUND) {
		uint16 prefixOffsetX = 0;
		byte hotkey = buttonText[0];

		// Hotkey needs to be highlighted
		if (textContainsHotkey) {
			Common::String prefixText = Common::String(buttonText.c_str() + 1);
			uint16 prefixTextLen = prefixText.size();
			uint16 prefixTextPos = 0;

			// Hotkey was passed additionally, we search for the hotkey inside the button text and
			// remove it from there. We then draw the whole text as highlighted and afterward
			// the processed text again as regular text (without the hotkey)
			while (prefixTextPos < prefixTextLen) {
				if (prefixText[prefixTextPos] == hotkey) {
					// Hotkey found, remove remaining text
					while (prefixTextPos < prefixText.size()) {
						prefixText.deleteLastChar();
					}
					break;
				}
				prefixTextPos++;
			}

			if (prefixTextPos < prefixTextLen) {
				// only adjust in case hotkey character was actually found
				prefixOffsetX = stringWidth(prefixText);
			}
		}

		if (slamIt) {
			print(Common::Point(xStart, pt.y + 1),
				COMMAND_FOREGROUND, "%s", buttonText.c_str() + skipTextOffset);
			print(Common::Point(xStart + prefixOffsetX, pt.y + 1), COMMAND_HIGHLIGHTED, "%c", hotkey);
		} else {
			gPrint(Common::Point(xStart, pt.y),
				COMMAND_FOREGROUND, "%s", buttonText.c_str() + skipTextOffset);
			gPrint(Common::Point(xStart + prefixOffsetX, pt.y), COMMAND_HIGHLIGHTED, "%c", hotkey);
		}
	} else if (slamIt) {
		print(Common::Point(xStart, pt.y + 1), color, "%s", buttonText.c_str() + skipTextOffset);
	} else {
		gPrint(Common::Point(xStart, pt.y), color, "%s", buttonText.c_str() + skipTextOffset);
	}
}

void ScalpelScreen::makePanel(const Common::Rect &r) {
	_backBuffer->fillRect(r, BUTTON_MIDDLE);
	_backBuffer->hLine(r.left, r.top, r.right - 2, BUTTON_TOP);
	_backBuffer->hLine(r.left + 1, r.top + 1, r.right - 3, BUTTON_TOP);
	_backBuffer->vLine(r.left, r.top, r.bottom - 1, BUTTON_TOP);
	_backBuffer->vLine(r.left + 1, r.top + 1, r.bottom - 2, BUTTON_TOP);

	_backBuffer->vLine(r.right - 1, r.top, r.bottom - 1, BUTTON_BOTTOM);
	_backBuffer->vLine(r.right - 2, r.top + 1, r.bottom - 2, BUTTON_BOTTOM);
	_backBuffer->hLine(r.left, r.bottom - 1, r.right - 1, BUTTON_BOTTOM);
	_backBuffer->hLine(r.left + 1, r.bottom - 2, r.right - 1, BUTTON_BOTTOM);
}

void ScalpelScreen::makeField(const Common::Rect &r) {
	_backBuffer->fillRect(r, BUTTON_MIDDLE);
	_backBuffer->hLine(r.left, r.top, r.right - 1, BUTTON_BOTTOM);
	_backBuffer->hLine(r.left + 1, r.bottom - 1, r.right - 1, BUTTON_TOP);
	_backBuffer->vLine(r.left, r.top + 1, r.bottom - 1, BUTTON_BOTTOM);
	_backBuffer->vLine(r.right - 1, r.top + 1, r.bottom - 2, BUTTON_TOP);
}

/*----------------------------------------------------------------*/

void Scalpel3DOScreen::blitFrom(const Graphics::Surface &src, const Common::Point &pt, const Common::Rect &srcBounds) {
	if (!_vm->_isScreenDoubled) {
		ScalpelScreen::blitFrom(src, pt, srcBounds);
		return;
	}

	Common::Rect srcRect = srcBounds;
	Common::Rect destRect(pt.x, pt.y, pt.x + srcRect.width(), pt.y + srcRect.height());

	if (!srcRect.isValidRect() || !clip(srcRect, destRect))
		return;

	// Add dirty area remapped to the 640x200 surface
	addDirtyRect(Common::Rect(destRect.left * 2, destRect.top * 2, destRect.right * 2, destRect.bottom * 2));

	// Transfer the area, doubling each pixel
	for (int yp = 0; yp < srcRect.height(); ++yp) {
		const uint16 *srcP = (const uint16 *)src.getBasePtr(srcRect.left, srcRect.top + yp);
		uint16 *destP = (uint16 *)getBasePtr(destRect.left * 2, (destRect.top + yp) * 2);

		for (int xp = srcRect.left; xp < srcRect.right; ++xp, ++srcP, destP += 2) {
			*destP = *srcP;
			*(destP + 1) = *srcP;
			*(destP + 640) = *srcP;
			*(destP + 640 + 1) = *srcP;
		}
	}
}

void Scalpel3DOScreen::transBlitFromUnscaled(const Graphics::Surface &src, const Common::Point &pt,
		bool flipped, int overrideColor) {
	if (!_vm->_isScreenDoubled) {
		ScalpelScreen::transBlitFromUnscaled(src, pt, flipped, overrideColor);
		return;
	}

	Common::Rect drawRect(0, 0, src.w, src.h);
	Common::Rect destRect(pt.x, pt.y, pt.x + src.w, pt.y + src.h);

	// Clip the display area to on-screen
	if (!clip(drawRect, destRect))
		// It's completely off-screen
		return;

	if (flipped)
		drawRect = Common::Rect(src.w - drawRect.right, src.h - drawRect.bottom,
		src.w - drawRect.left, src.h - drawRect.top);

	Common::Point destPt(destRect.left, destRect.top);
	addDirtyRect(Common::Rect(destPt.x * 2, destPt.y * 2, (destPt.x + drawRect.width()) * 2,
		(destPt.y + drawRect.height()) * 2));

	assert(src.format.bytesPerPixel == 2 && _surface.format.bytesPerPixel == 2);

	for (int yp = 0; yp < drawRect.height(); ++yp) {
		const uint16 *srcP = (const uint16 *)src.getBasePtr(
			flipped ? drawRect.right - 1 : drawRect.left, drawRect.top + yp);
		uint16 *destP = (uint16 *)getBasePtr(destPt.x * 2, (destPt.y + yp) * 2);

		for (int xp = 0; xp < drawRect.width(); ++xp, destP += 2) {
			// RGB 0, 0, 0 -> transparent on 3DO
			if (*srcP) {
				*destP = *srcP;
				*(destP + 1) = *srcP;
				*(destP + 640) = *srcP;
				*(destP + 640 + 1) = *srcP;
			}

			srcP = flipped ? srcP - 1 : srcP + 1;
		}
	}
}

void Scalpel3DOScreen::fillRect(const Common::Rect &r, uint color) {
	if (_vm->_isScreenDoubled)
		ScalpelScreen::fillRect(Common::Rect(r.left * 2, r.top * 2, r.right * 2, r.bottom * 2), color);
	else
		ScalpelScreen::fillRect(r, color);
}

void Scalpel3DOScreen::fadeIntoScreen3DO(int speed) {
	Events &events = *_vm->_events;
	uint16 *currentScreenBasePtr = (uint16 *)getPixels();
	uint16 *targetScreenBasePtr = (uint16 *)_backBuffer->getPixels();
	uint16  currentScreenPixel = 0;
	uint16  targetScreenPixel = 0;

	uint16  currentScreenPixelRed = 0;
	uint16  currentScreenPixelGreen = 0;
	uint16  currentScreenPixelBlue = 0;

	uint16  targetScreenPixelRed = 0;
	uint16  targetScreenPixelGreen = 0;
	uint16  targetScreenPixelBlue = 0;

	uint16  screenWidth = SHERLOCK_SCREEN_WIDTH;
	uint16  screenHeight = SHERLOCK_SCREEN_HEIGHT;
	uint16  screenX = 0;
	uint16  screenY = 0;
	uint16  pixelsChanged = 0;

	clearDirtyRects();

	do {
		pixelsChanged = 0;
		uint16 *currentScreenPtr = currentScreenBasePtr;
		uint16 *targetScreenPtr = targetScreenBasePtr;

		for (screenY = 0; screenY < screenHeight; screenY++) {
			for (screenX = 0; screenX < screenWidth; screenX++) {
				currentScreenPixel = *currentScreenPtr;
				targetScreenPixel = *targetScreenPtr;

				if (currentScreenPixel != targetScreenPixel) {
					// pixel doesn't match, adjust accordingly
					currentScreenPixelRed = currentScreenPixel & 0xF800;
					currentScreenPixelGreen = currentScreenPixel & 0x07E0;
					currentScreenPixelBlue = currentScreenPixel & 0x001F;
					targetScreenPixelRed = targetScreenPixel & 0xF800;
					targetScreenPixelGreen = targetScreenPixel & 0x07E0;
					targetScreenPixelBlue = targetScreenPixel & 0x001F;

					if (currentScreenPixelRed != targetScreenPixelRed) {
						if (currentScreenPixelRed < targetScreenPixelRed) {
							currentScreenPixelRed += 0x0800;
						} else {
							currentScreenPixelRed -= 0x0800;
						}
					}
					if (currentScreenPixelGreen != targetScreenPixelGreen) {
						// Adjust +2/-2 because we are running RGB555 at RGB565
						if (currentScreenPixelGreen < targetScreenPixelGreen) {
							currentScreenPixelGreen += 0x0040;
						} else {
							currentScreenPixelGreen -= 0x0040;
						}
					}
					if (currentScreenPixelBlue != targetScreenPixelBlue) {
						if (currentScreenPixelBlue < targetScreenPixelBlue) {
							currentScreenPixelBlue += 0x0001;
						} else {
							currentScreenPixelBlue -= 0x0001;
						}
					}

					uint16 v = currentScreenPixelRed | currentScreenPixelGreen | currentScreenPixelBlue;
					*currentScreenPtr = v;
					if (_vm->_isScreenDoubled) {
						*(currentScreenPtr + 1) = v;
						*(currentScreenPtr + 640) = v;
						*(currentScreenPtr + 640 + 1) = v;
					}

					pixelsChanged++;
				}

				currentScreenPtr += _vm->_isScreenDoubled ? 2 : 1;
				targetScreenPtr++;
			}

			if (_vm->_isScreenDoubled)
				currentScreenPtr += 640;
		}

		// Too much considered dirty at the moment
		if (_vm->_isScreenDoubled)
			addDirtyRect(Common::Rect(0, 0, screenWidth * 2, screenHeight * 2));
		else
			addDirtyRect(Common::Rect(0, 0, screenWidth, screenHeight));

		events.pollEvents();
		events.delay(10 * speed);
	} while ((pixelsChanged) && (!_vm->shouldQuit()));
}

void Scalpel3DOScreen::blitFrom3DOcolorLimit(uint16 limitColor) {
	uint16 *currentScreenPtr = (uint16 *)getPixels();
	uint16 *targetScreenPtr = (uint16 *)_backBuffer->getPixels();
	uint16  currentScreenPixel = 0;

	uint16  screenWidth = SHERLOCK_SCREEN_WIDTH;
	uint16  screenHeight = SHERLOCK_SCREEN_HEIGHT;
	uint16  screenX = 0;
	uint16  screenY = 0;

	uint16  currentScreenPixelRed = 0;
	uint16  currentScreenPixelGreen = 0;
	uint16  currentScreenPixelBlue = 0;

	uint16  limitPixelRed = limitColor & 0xF800;
	uint16  limitPixelGreen = limitColor & 0x07E0;
	uint16  limitPixelBlue = limitColor & 0x001F;

	for (screenY = 0; screenY < screenHeight; screenY++) {
		for (screenX = 0; screenX < screenWidth; screenX++) {
			currentScreenPixel = *targetScreenPtr;

			currentScreenPixelRed = currentScreenPixel & 0xF800;
			currentScreenPixelGreen = currentScreenPixel & 0x07E0;
			currentScreenPixelBlue = currentScreenPixel & 0x001F;

			if (currentScreenPixelRed < limitPixelRed)
				currentScreenPixelRed = limitPixelRed;
			if (currentScreenPixelGreen < limitPixelGreen)
				currentScreenPixelGreen = limitPixelGreen;
			if (currentScreenPixelBlue < limitPixelBlue)
				currentScreenPixelBlue = limitPixelBlue;

			uint16 v = currentScreenPixelRed | currentScreenPixelGreen | currentScreenPixelBlue;
			*currentScreenPtr = v;
			if (_vm->_isScreenDoubled) {
				*(currentScreenPtr + 1) = v;
				*(currentScreenPtr + 640) = v;
				*(currentScreenPtr + 640 + 1) = v;
			}

			currentScreenPtr += _vm->_isScreenDoubled ? 2 : 1;
			targetScreenPtr++;
		}

		if (_vm->_isScreenDoubled)
			currentScreenPtr += 640;
	}

	// Too much considered dirty at the moment
	if (_vm->_isScreenDoubled)
		addDirtyRect(Common::Rect(0, 0, screenWidth * 2, screenHeight * 2));
	else
		addDirtyRect(Common::Rect(0, 0, screenWidth, screenHeight));
}

uint16 Scalpel3DOScreen::w() const {
	return _vm->_isScreenDoubled ? _surface.w / 2 : _surface.w;
}

uint16 Scalpel3DOScreen::h() const {
	return _vm->_isScreenDoubled ? _surface.h / 2 : _surface.h;
}

void Scalpel3DOScreen::rawBlitFrom(const Graphics::Surface &src, const Common::Point &pt) {
	Common::Rect srcRect(0, 0, src.w, src.h);
	Common::Rect destRect(pt.x, pt.y, pt.x + src.w, pt.y + src.h);

	addDirtyRect(destRect);
	_surface.copyRectToSurface(src, destRect.left, destRect.top, srcRect);
}

} // End of namespace Scalpel

} // End of namespace Sherlock
