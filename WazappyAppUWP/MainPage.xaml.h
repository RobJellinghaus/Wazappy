// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

#include "MainPage.g.h"
#include "SampleConfiguration.h"

namespace Wazappy
{
	// Class for DeviceStateChanged events
	public ref class DeviceStateChangedEventArgs sealed
	{
	internal:
		DeviceStateChangedEventArgs(DeviceState newState, HRESULT hr) :
			m_DeviceState(newState),
			m_hr(hr)
		{};

		property DeviceState State
		{
			DeviceState get() { return m_DeviceState; }
		};

		property int hr
		{
			int get() { return m_hr; }
		}

	private:
		DeviceState      m_DeviceState;
		HRESULT          m_hr;
	};

	// DeviceStateChanged delegate
	public delegate void DeviceStateChangedHandler(Platform::Object^ sender, DeviceStateChangedEventArgs^ e);

	// DeviceStateChanged Event
	public ref class DeviceStateChangedEvent sealed
	{
	public:
		DeviceStateChangedEvent() :
			m_DeviceState(DeviceState::Uninitialized)
		{};

		DeviceState GetState() { return m_DeviceState; };

	internal:
		void SetState(DeviceState newState, HRESULT hr, Platform::Boolean FireEvent) {
			if (m_DeviceState != newState)
			{
				m_DeviceState = newState;

				if (FireEvent)
				{
					DeviceStateChangedEventArgs^ e = ref new DeviceStateChangedEventArgs(m_DeviceState, hr);
					StateChangedEvent(this, e);
				}
			}
		};

	public:
		static event DeviceStateChangedHandler^    StateChangedEvent;

	private:
		DeviceState     m_DeviceState;
	};

    public enum class NotifyType
    {
        StatusMessage,
        ErrorMessage
    };

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();

    protected:
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:
        void ScenarioControl_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
        void Footer_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    internal:
        static MainPage^ Current;
        void NotifyUser(Platform::String^ strMessage, NotifyType type);
    };
}
