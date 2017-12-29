// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

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
