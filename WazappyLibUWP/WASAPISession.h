// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "WazappyDllInterface.h"
#include "WASAPIDevice.h"

using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;

#pragma once

namespace Wazappy
{
	// WASAPI Session 
	class WASAPISession
	{
	private:
		static std::mutex s_mutex;

		// Central session-scoped node-id-to-device mapping; owns all the WASAPIDevices.
		static std::map<NodeId, ComPtr<WASAPIDevice>> s_deviceMap;

		static NodeId s_nextNodeId;

	public: 
		// Get next unallocated node ID.
		static NodeId GetNextNodeId();

		// Register the given device (which must already have a node ID that has not yet registered).
		static void RegisterDevice(const ComPtr<WASAPIDevice>& device);

		// Unregister the given device.
		static void UnregisterDevice(NodeId nodeId);

		// Get the device with the given ID; it must exist.
		static WASAPIDevice* GetDevice(NodeId nodeId);
	};
}

