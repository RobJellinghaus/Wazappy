// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

#include "pch.h"
#include "Scenario1.g.h"
#include "MainPage.xaml.h"

using namespace Windows::Devices::Enumeration;

namespace WazappyApp
{
	// Custom properties defined in mmdeviceapi.h in the format "{GUID} PID"
	static Platform::String^ PKEY_AudioEndpoint_Supports_EventDriven_Mode = "{1da5d803-d492-4edd-8c23-e0c0ffee7f0e} 7";

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class Scenario1 sealed
	{
	public:
		Scenario1();

	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

	private:
		void ShowStatusMessage(Platform::String^ str, NotifyType messageType);
		void Enumerate_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void EnumerateAudioDevicesAsync();

	private:
		MainPage^ rootPage;
		Windows::UI::Xaml::Controls::ListBox^ m_DevicesList;
	};
}
