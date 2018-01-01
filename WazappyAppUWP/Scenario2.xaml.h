// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

#include "pch.h"
#include "Scenario2.g.h"
#include "MainPage.xaml.h"
#include "WazappyDllInterface.h"

namespace WazappyApp
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class Scenario2 sealed
	{

	public:
		Scenario2();

	protected:
		// Template Support
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
		virtual void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

	private:
		~Scenario2();

		// UI Events
		void btnPlay_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnPause_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnPlayPause_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnStop_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnFilePicker_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void radioTone_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void radioFile_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void sliderFrequency_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sliderVolume_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

		// UI Helpers
		void ShowStatusMessage(Platform::String^ str, NotifyType messageType);
		void UpdateContentProps(Platform::String^ str);
		void UpdateContentUI(Platform::Boolean disableAll);
		void UpdateMediaControlUI(DeviceState deviceState);

		// Handlers
		void OnDeviceStateChange(Object^ sender, DeviceStateChangedEventArgs^ e);
		void OnPickFileAsync();
		void OnSetVolume(UINT32 volume);

		void InitializeDevice();
		void StartDevice();
		void StopDevice();
		void PauseDevice();
		void PlayPauseToggleDevice();

		void MediaButtonPressed(Windows::Media::SystemMediaTransportControls^ sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^ e);

	private:
		MainPage^ rootPage;
		Windows::UI::Core::CoreDispatcher^ m_CoreDispatcher;
		Windows::Media::SystemMediaTransportControls^ m_SystemMediaControls;
		Windows::Foundation::EventRegistrationToken m_deviceStateChangeToken;
		Windows::Foundation::EventRegistrationToken m_SystemMediaControlsButtonToken;

		Platform::Boolean m_IsMFLoaded;
		IRandomAccessStream^ m_ContentStream;
		ContentType m_ContentType;
		DeviceStateChangedEvent^ m_StateChangedEvent;
		WazappyNodeHandle m_renderer;
		CallbackId m_deviceStateChangeCallbackId;
		Platform::Boolean m_deviceSupportsRawMode;
	};
}