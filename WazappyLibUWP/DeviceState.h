// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

using namespace Windows::Storage::Streams;

namespace Wazappy
{
	public enum class DeviceState
	{
		Uninitialized,
		InError,
		Discontinuity,
		Flushing,
		Activated,
		Initialized,
		Starting,
		Playing,
		Capturing,
		Pausing,
		Paused,
		Stopping,
		Stopped
	};
}
