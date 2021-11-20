#include "common/scummsys.h"
#include "winuwpsdl-graphics.h"
#include "graphics/scaler/aspect.h"

WinUWPSdlGraphicsManager::WinUWPSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window) :
	SurfaceSdlGraphicsManager::SurfaceSdlGraphicsManager(sdlEventSource, window)
{
}

void WinUWPSdlGraphicsManager::transformMouseCoordinates(Common::Point &point) {

	point.x -= _viewport.x;
	point.y -= _viewport.y;
	
	point.x = point.x * ((float)_videoMode.hardwareWidth / _viewport.w);
	point.y = point.y * ((float)_videoMode.hardwareHeight / _viewport.h);

	if (!_overlayVisible) {
		point.x /= _videoMode.scaleFactor;
		point.y /= _videoMode.scaleFactor;
		if (_videoMode.aspectRatioCorrection)
			point.y = aspect2Real(point.y);
	}
}

WinUWPSdlGraphicsManager::~WinUWPSdlGraphicsManager()
{
}
