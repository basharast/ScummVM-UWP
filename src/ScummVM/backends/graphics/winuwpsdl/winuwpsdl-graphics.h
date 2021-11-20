#include "backends\graphics\surfacesdl\surfacesdl-graphics.h"

class WinUWPSdlGraphicsManager : public SurfaceSdlGraphicsManager {
public:
	WinUWPSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window);
	void transformMouseCoordinates(Common::Point & point);
	virtual ~WinUWPSdlGraphicsManager();
};