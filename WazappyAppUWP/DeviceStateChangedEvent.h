// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

#include "pch.h"
#include "windows.h"
#include "DeviceState.h"

namespace WazappyApp
{
	using namespace Wazappy;

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
		DeviceState m_DeviceState;
		HRESULT m_hr;
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
		static event DeviceStateChangedHandler^ StateChangedEvent;

	private:
		DeviceState m_DeviceState;
	};
}
