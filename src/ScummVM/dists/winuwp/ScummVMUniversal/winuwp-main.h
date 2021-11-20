#pragma once
#include "winuwp-main.h"
namespace ScummVMUniversal
{
#ifdef UWPNEW
    // Main entry point for our app. Connects the app with the Windows shell and handles application lifecycle events.
    ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
    {
    public:
        App();

        // IFrameworkView methods.
        virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
        virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
        virtual void Load(Platform::String^ entryPoint);
        virtual void Run();
        virtual void Uninitialize();

    protected:
        // Application lifecycle event handlers.
        void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
        void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
        void OnResuming(Platform::Object^ sender, Platform::Object^ args);

        void OnBackRequested(Platform::Object^ sender, Windows::UI::Core::BackRequestedEventArgs^ args);

        // Window event handlers.
        void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
        void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
        void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
        void OnWindowActivated(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowActivatedEventArgs^ args);
        void OnKey(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
        void OnPointer(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);

        // DisplayInformation event handlers.
        void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
        void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
        void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

        void OnPackageInstalling(Windows::ApplicationModel::PackageCatalog^ sender, Windows::ApplicationModel::PackageInstallingEventArgs^ args);

    public:
        bool IsInitialized() { return m_initialized; }
        bool IsWindowClosed() { return m_windowClosed; }
        bool IsWindowVisible() { return m_windowVisible; }
        bool IsWindowFocused() { return m_windowFocused; }
        bool CheckWindowResized() { bool resized = m_windowResized; m_windowResized = false; return resized; }
        void SetWindowResized() { m_windowResized = true; }
        static App^ GetInstance() { return m_instance; }

    private:
        bool m_initialized;
        bool m_windowClosed;
        bool m_windowVisible;
        bool m_windowFocused;
        bool m_windowResized;
        static App^ m_instance;
    };
#endif
}