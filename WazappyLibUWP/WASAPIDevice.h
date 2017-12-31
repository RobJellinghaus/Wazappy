// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "WazappyDllInterface.h"
#include "ToneSampleGenerator.h"
#include "MFSampleGenerator.h"

using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;

#pragma once

namespace Wazappy
{
	// Represents a WASAPI device, either input or output.
	class WASAPIDevice :
		public RuntimeClass< RuntimeClassFlags< ClassicCom >, FtmBase, IActivateAudioInterfaceCompletionHandler >
	{
	public:
		WASAPIDevice();

		virtual HRESULT InitializeAudioDeviceAsync();

		virtual Platform::String^ GetDeviceId() = 0;
		virtual HRESULT ConfigureDeviceInternal() = 0;

		HRESULT SetVolumeOnSession(UINT32 volume);

		METHODASYNCCALLBACK(WASAPIDevice, SampleReady, OnSampleReady);

		// IActivateAudioInterfaceCompletionHandler
		STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation *operation);

		// Subtypes override this method to perform additional logic on activation.
		virtual HRESULT ActivateCompletedInternal() = 0;

	private:
		HRESULT OnSampleReady(IMFAsyncResult* pResult);

		// An audio sample is requested by the device.
		virtual HRESULT OnAudioSampleRequested(Platform::Boolean IsSilence = false) = 0;

		// Returns true if the device is currently active (e.g. if sample-ready work items should continue to be queued).
		virtual bool IsDeviceActive(DeviceState deviceState) = 0;

	protected:
		virtual ~WASAPIDevice();

		Platform::String^ m_DeviceIdString;
		UINT32 m_BufferFrames;

		HANDLE m_SampleReadyEvent;
		MFWORKITEM_KEY m_SampleReadyKey;
		IMFAsyncResult *m_SampleReadyAsyncResult;

		CRITICAL_SECTION m_CritSec;

		DeviceState m_DeviceState;

		WAVEFORMATEX *m_MixFormat;

		IAudioClient3 *m_AudioClient;
		UINT32 m_DefaultPeriodInFrames;
		UINT32 m_FundamentalPeriodInFrames;
		UINT32 m_MaxPeriodInFrames;
		UINT32 m_MinPeriodInFrames;
	};
}

