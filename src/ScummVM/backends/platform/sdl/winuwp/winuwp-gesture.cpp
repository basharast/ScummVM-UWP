#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "winuwp-gesture.h"
#include <windows.h>
#include <agile.h>
#include <..\um\debugapi.h>

using namespace Windows::Devices::Input;
using namespace Windows::UI::Core;

WinUWPGesture::WinUWPGesture()
{
	Init();
}

void WinUWPGesture::Init()
{
	try {
		_eventMan = g_system->getEventManager();

		_recognizer = ref new GestureRecognizer();
		_recognizer->GestureSettings = GestureSettings::RightTap | GestureSettings::ManipulationTranslateX | GestureSettings::ManipulationTranslateY | GestureSettings::ManipulationScale | GestureSettings::DoubleTap;

		_recognizer->RightTapped += ref new TypedEventHandler<GestureRecognizer^, RightTappedEventArgs^>(this, &WinUWPGesture::OnRightTab);
		_recognizer->Tapped += ref new TypedEventHandler<GestureRecognizer^, TappedEventArgs^>(this, &WinUWPGesture::OnTapped);

		_recognizer->ManipulationStarted += ref new TypedEventHandler<GestureRecognizer^, ManipulationStartedEventArgs^>(this, &WinUWPGesture::OnManipulationStarted);
		_recognizer->ManipulationUpdated += ref new TypedEventHandler<GestureRecognizer^, ManipulationUpdatedEventArgs^>(this, &WinUWPGesture::OnManipulationUpdated);
		_recognizer->ManipulationCompleted += ref new TypedEventHandler<GestureRecognizer^, ManipulationCompletedEventArgs^>(this, &WinUWPGesture::OnManipulationCompleted);

		Platform::Agile<CoreWindow^> _agileWindow;
		auto _window = _agileWindow->GetForCurrentThread();

		_cursor = ref new CoreCursor(CoreCursorType::Custom, 101);

		_window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinUWPGesture::OnPointerPressed);
		_window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinUWPGesture::OnPointerReleased);
		_window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinUWPGesture::OnPointerMoved);

		SystemNavigationManager::GetForCurrentView()->BackRequested += ref new EventHandler<BackRequestedEventArgs^>(this, &WinUWPGesture::OnBackRequested);
	}
	catch (...) {}
}

void WinUWPGesture::OnManipulationStarted(GestureRecognizer^ sender, ManipulationStartedEventArgs^ e)
{
}

void WinUWPGesture::OnManipulationUpdated(GestureRecognizer^ sender, ManipulationUpdatedEventArgs^ e)
{
}

void WinUWPGesture::OnManipulationCompleted(GestureRecognizer^ sender, ManipulationCompletedEventArgs^ e)
{
	try {
		Common::Event event;
		if (e->Velocities.Linear.X >= 2.5) {
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = Common::ASCII_ESCAPE;
			event.type = Common::EVENT_KEYDOWN;
			_eventMan->pushEvent(event);
			event.type = Common::EVENT_KEYUP;
			_eventMan->pushEvent(event);
			return;
		}

		if (e->Cumulative.Expansion < -100) {
			event.type = Common::EVENT_MAINMENU;
			_eventMan->pushEvent(event);
			return;
		}

		if (e->Cumulative.Expansion > 100) {
			event.type = Common::EVENT_VIRTUAL_KEYBOARD;
			_eventMan->pushEvent(event);
			return;
		}
	}
	catch (...) {}
}

void WinUWPGesture::OnBackRequested(Object ^ sender, BackRequestedEventArgs ^ e)
{
	try {
		Common::Event event;
		event.mouse = _eventMan->getMousePos();
		event.type = Common::EVENT_RBUTTONDOWN;
		_eventMan->pushEvent(event);
		event.type = Common::EVENT_RBUTTONUP;
		_eventMan->pushEvent(event);
		e->Handled = true;
	}
	catch (...) {
		e->Handled = true;
	}
}

void WinUWPGesture::OnRightTab(GestureRecognizer^ sender, RightTappedEventArgs^ e)
{
	try {
		Common::Event event;
		event.mouse = _eventMan->getMousePos();
		event.type = Common::EVENT_RBUTTONDOWN;
		_eventMan->pushEvent(event);
		event.type = Common::EVENT_RBUTTONUP;
		_eventMan->pushEvent(event);
	}
	catch (...) {}
}

void WinUWPGesture::OnTapped(GestureRecognizer ^ sender, TappedEventArgs ^ e)
{
	try {
		if (e->TapCount > 1) {
			Common::Event event;
			event.mouse = _eventMan->getMousePos();
			event.type = Common::EVENT_LBUTTONDOWN;
			_eventMan->pushEvent(event);
			event.type = Common::EVENT_LBUTTONUP;
			_eventMan->pushEvent(event);
			event.type = Common::EVENT_LBUTTONDOWN;
			_eventMan->pushEvent(event);
			event.type = Common::EVENT_LBUTTONUP;
			_eventMan->pushEvent(event);
		}
	}
	catch (...) {}
}

void WinUWPGesture::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ e)
{
	try {
		_recognizer->ProcessDownEvent(e->CurrentPoint);
	}
	catch (...) {}
}

void WinUWPGesture::OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ e)
{
	try {
		_recognizer->ProcessUpEvent(e->CurrentPoint);
	}
	catch (...) {}
}

void WinUWPGesture::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ e)
{
	try {
		sender->PointerCursor = _cursor;
		_recognizer->ProcessMoveEvents(e->GetIntermediatePoints());
	}
	catch (...) {}
}