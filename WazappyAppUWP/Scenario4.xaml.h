﻿// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

#include "pch.h"
#include "Scenario4.g.h"
#include "MainPage.xaml.h"
#include "WazappyDllInterface.h"

#define OSC_START_X  100
#define OSC_START_Y  100
#define OSC_X_LENGTH 700
#define OSC_TOTAL_HEIGHT 200

namespace Wazappy
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class Scenario4 sealed
	{

	public:
		Scenario4();

	protected:
		// Template Support
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
		virtual void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

	private:
		~Scenario4();

		// UI Events
		void btnStartCapture_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnStopCapture_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		// UI Helpers
		void ShowStatusMessage(Platform::String^ str, NotifyType messageType);
		void UpdateMediaControlUI(DeviceState deviceState);

		// Handlers
		void OnDeviceStateChange(Object^ sender, DeviceStateChangedEventArgs^ e);
		void OnPlotDataReady(Object^ sender, PlotDataReadyEventArgs^ e);

		void InitializeCapture(Object^ sender, Object^ e);
		void StopCapture(Object^ sender, Object^ e);

	private:
		MainPage ^ rootPage;
		Windows::UI::Core::CoreDispatcher^              m_CoreDispatcher;
		Windows::UI::Xaml::Shapes::Polyline^            m_Oscilloscope;

		Windows::Foundation::EventRegistrationToken     m_deviceStateChangeToken;
		Windows::Foundation::EventRegistrationToken     m_plotDataReadyToken;

		int                         m_DiscontinuityCount;
		Platform::Boolean           m_IsMFLoaded;
		Platform::Boolean           m_IsLowLatency;
		DeviceStateChangedEvent^    m_StateChangedEvent;
		ComPtr<IWASAPIClient>       m_spClient;
		ComPtr<IWASAPICapture>      m_spCapture;
	};
}