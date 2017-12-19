// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

using namespace Windows::Storage::Streams;

namespace Wazappy
{
	// NB: All states >= DeviceStateInitialized will allow some methods
	// to be called successfully on the Audio Client
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
