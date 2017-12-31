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
		std::map<NodeId, ComPtr<WASAPIDevice>> s_deviceMap;

	public: 

	};
}

