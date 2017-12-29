// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.

#pragma once

#include "WazappyDllInterface.h"
#include "DeviceStateChangedEvent.h"

namespace WazappyApp
{
	// Provide services for registering CallbackId objects statically for use from Wazappy.
	class WazappyClient
	{
	private:
		// Actual callback function which we register with the DLL.
		// Is thread-safe (takes a global lock).
		static void DeviceStateChangedEventCallback(Wazappy::CallbackId id, Wazappy::DeviceState deviceState);

	public:
		// Get a CallbackId for the given event.
		// Is thread-safe (takes a global lock).
		static Wazappy::CallbackId RegisterCallback(DeviceStateChangedEvent^ deviceStateChangedEvent);

		// Unregister the given callback.
		// Is thread-safe (takes a global lock).
		static void UnregisterCallback(Wazappy::CallbackId id);
	};
}
