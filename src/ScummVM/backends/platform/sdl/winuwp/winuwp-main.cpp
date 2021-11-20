#include <Windows.h>
#include <ppltasks.h>
#include <pplawait.h>

#include "SDL_main.h"
#include <wrl.h>

#include "backends/platform/sdl/winuwp/winuwp.h"
#include "base/main.h"

using namespace Windows::Storage;
using namespace Windows::System::Profile;  
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::Devices::Enumeration;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Devices::Input;
using namespace Windows::System;


bool DeviceIsPhone()
{
#if defined (_M_ARM)
	return true;
#else
	return false;
#endif
}


int WINAPI  WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED))) {
		return 1;
	}

	SDL_WinRTRunApp(SDL_main, NULL);
	return 0;
}



Platform::String^ GetDataFromLocalSettings(Platform::String^ key) {
	ApplicationDataContainer^ localSettings{ ApplicationData::Current->LocalSettings };
	IPropertySet^ values{ localSettings->Values };

	Platform::Object^ tokenRetrive = values->Lookup(key);
	if (tokenRetrive != nullptr) {
		Platform::String^ ConvertedToken = (Platform::String^)tokenRetrive;
		return ConvertedToken;
	}
	return nullptr;
}

bool AddDataToLocalSettings(Platform::String^ key, Platform::String^ data, bool replace) {
	ApplicationDataContainer^ localSettings{ ApplicationData::Current->LocalSettings };
	IPropertySet^ values{ localSettings->Values };

	Platform::String^ testResult = GetDataFromLocalSettings(key);
	if (testResult == nullptr) {
		values->Insert(key, PropertyValue::CreateString(data));
		return true;
	}
	else if (replace) {
		values->Remove(key);
		values->Insert(key, PropertyValue::CreateString(data));
		return true;
	}

	return false;
}


int main(int argc, char** argv){
	auto tipState = GetDataFromLocalSettings("tipState");
	if (tipState == nullptr || tipState == "1" || tipState == "2") {
		setTipState(true);
		if (tipState == nullptr) {
			AddDataToLocalSettings("tipState", "1", true);
		}
		else if (tipState == "1") {
			AddDataToLocalSettings("tipState", "2", true);
		}
		else {
			AddDataToLocalSettings("tipState", "end", true);
		}
	}
	auto myDispatcher = CoreWindow::GetForCurrentThread()->Dispatcher;
	//Below 'StatusBar' for Windows Mobile Only
	if (DeviceIsPhone()) {
		try {
			StatusBar^ statusBar = StatusBar::GetForCurrentView();
			statusBar->ProgressIndicator->Text = "ScummVM Starting, Please wait...";
			statusBar->ShowAsync();
			statusBar->ProgressIndicator->ShowAsync();
		}
		catch (...) {
		}
		Windows::UI::Core::CoreWindow^ corewindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
		int i = 0;
		while (i < 10000)
		{
			i++;
		}
		auto t = concurrency::create_task([myDispatcher]() {
			Sleep(1000);
			try {
				Windows::UI::Core::CoreWindow^ corewindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
				while (!getLoadState())
				{
					if (corewindow) {
						corewindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
					}
				}
				try {
					myDispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([]() {
						try {
							StatusBar^ statusBar = StatusBar::GetForCurrentView();
							statusBar->ProgressIndicator->HideAsync();
							statusBar->ProgressIndicator->Text = "";
							statusBar->HideAsync();
						}
						catch (...) {
						}
						}));
				}
				catch (...) {
				}
			}
			catch (...) {

			}
			});
	}
	g_system = new OSystem_WinUWP();
	((OSystem_WinUWP*)g_system)->init();
	int res = scummvm_main(argc, argv);
	return res;
}

