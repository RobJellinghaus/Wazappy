#pragma once

#include <Windows.h>
#include <wrl\implements.h>
#include <mfapi.h>
#include <AudioClient.h>
#include <mmdeviceapi.h>

#include "DeviceState.h"

namespace Wazappy
{
	// User Configurable Arguments for Scenario
	struct DEVICEPROPS
	{
		Platform::Boolean       IsHWOffload;
		Platform::Boolean       IsTonePlayback;
		Platform::Boolean       IsBackground;
		Platform::Boolean       IsRawSupported;
		Platform::Boolean       IsRawChosen;
		Platform::Boolean       IsLowLatency;
		REFERENCE_TIME          hnsBufferDuration;
		DWORD                   Frequency;
		IRandomAccessStream^    ContentStream;
	};

	public enum class ContentType
	{
		ContentTypeTone,
		ContentTypeFile
	};

	class __declspec(dllexport) IWASAPIRenderer : public IUnknown
	{
	public:
		virtual HRESULT SetVolumeOnSession(UINT32 volume) = 0;
		virtual DeviceStateChangedEvent^ GetDeviceStateEvent() = 0;
		virtual HRESULT SetProperties(DEVICEPROPS props) = 0;
		virtual HRESULT InitializeAudioDeviceAsync() = 0;
		virtual HRESULT StartPlaybackAsync() = 0;
		virtual HRESULT StopPlaybackAsync() = 0;
		virtual HRESULT PausePlaybackAsync() = 0;
	};

	class __declspec(dllexport) IWASAPICapture : public IUnknown
	{
	public:
	};

	class __declspec(dllexport) IWASAPIClient : public IUnknown
	{
	public:
		virtual ComPtr<IWASAPIRenderer> CreateRenderer() = 0;
	};

	class __declspec(dllexport) WASAPIClientFactory
	{
	public:
		static ComPtr<IWASAPIClient> CreateClient();
	};
}
