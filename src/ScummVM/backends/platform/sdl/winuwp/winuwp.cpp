#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include <string>
#include <algorithm>
#include <Windows.h>
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "backends\platform\sdl\winuwp\winuwp.h"
#include "backends\fs\winuwp\winuwp-fs.h"
#include "backends\fs\winuwp\winuwp-fs-factory.h"
#include "backends\mixer\sdl\sdl-mixer.h"
#include "backends\graphics\winuwpsdl\winuwpsdl-graphics.h"

using namespace Windows::Storage;
using namespace Windows::System::Display;
using namespace Windows::System::Profile;

OSystem_WinUWP::OSystem_WinUWP() : _virtualKbd(false) {
	try {
		_fsFactory = new WinUWPFilesystemFactory();
	}
	catch (...) {

	}
	try {
		_displayRequest = ref new DisplayRequest();
	}
	catch (...) {

	}
}

void OSystem_WinUWP::initBackend() {

	try {
		StorageFolder^ installedLocation = Windows::ApplicationModel::Package::Current->InstalledLocation;
		StorageFolder^ local = ApplicationData::Current->LocalFolder;

		WinUWPFilesystemNode::createDir(local, "saves");

		ConfMan.set("vkeybdpath", Common::String(WinUWPFilesystemNode::toAscii(installedLocation->Path->Data())) + "\\Assets\\vkeybd\\");
		ConfMan.set("themepath", Common::String(WinUWPFilesystemNode::toAscii(installedLocation->Path->Data())) + "\\Assets\\themes\\");
		ConfMan.set("gui_theme", Common::String(WinUWPFilesystemNode::toAscii(installedLocation->Path->Data())) + "\\Assets\\themes\\scummmodern.zip");
		ConfMan.set("savepath", Common::String(WinUWPFilesystemNode::toAscii(local->Path->Data())) + "\\saves\\");
	}
	catch (...) {

	}
	try {
		if (!_eventSource) {
			_eventSource = new SdlEventSource();
		}
	}
	catch (...) {

	}
	try {
		if (!_graphicsManager) {
			_graphicsManager = new WinUWPSdlGraphicsManager(_eventSource, _window);
		}
	}
	catch (...) {

	}
	try {
		OSystem_SDL::initBackend();
	}
	catch (...) {

	}
	try {
		_gesture = ref new WinUWPGesture();
	}
	catch (...) {

	}
	try {
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
		SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
		SDL_ShowCursor(SDL_TRUE);
	}
	catch (...) {

	}
	
}

void OSystem_WinUWP::engineInit()
{
	try {
	OSystem_SDL::engineInit();
	_displayRequest->RequestActive();
	}
	catch (...) {

	}
}

void OSystem_WinUWP::engineDone()
{
	try {
	OSystem_SDL::engineDone();
	_displayRequest->RequestRelease();
	}
	catch (...) {

	}
}

Common::String OSystem_WinUWP::getDefaultConfigFileName() {
	StorageFolder^ local = ApplicationData::Current->LocalFolder;
	return Common::String(WinUWPFilesystemNode::toAscii(local->Path->Data())) + "\\scummvm.ini";
}

Common::String OSystem_WinUWP::getSystemLanguage() const
{
	if (Windows::System::UserProfile::GlobalizationPreferences::Languages->Size == 0) {
		return Common::String("en_US");
	}

	auto language = Windows::System::UserProfile::GlobalizationPreferences::Languages->GetAt(0)->Data();
	static char asciiString[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, language, wcslen(language) + 1, asciiString, sizeof(asciiString), NULL, NULL);
	std::string str = std::string(asciiString);
	std::replace(str.begin(), str.end(), '-', '_');
	return Common::String(str.c_str());
}
