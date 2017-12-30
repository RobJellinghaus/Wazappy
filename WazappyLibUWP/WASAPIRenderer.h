// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "WASAPIDevice.h"
#include "ToneSampleGenerator.h"
#include "MFSampleGenerator.h"

using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;

#pragma once

namespace Wazappy
{
    // Primary WASAPI Renderering Class
    class WASAPIRenderDevice : public WASAPIDevice
	{
    public:
        WASAPIRenderDevice();

		HRESULT SetProperties(DEVICEPROPS props);
		
		HRESULT StartPlaybackAsync();
        HRESULT StopPlaybackAsync();
        HRESULT PausePlaybackAsync();

        METHODASYNCCALLBACK( WASAPIRenderDevice, StartPlayback, OnStartPlayback );
        METHODASYNCCALLBACK( WASAPIRenderDevice, StopPlayback, OnStopPlayback );
        METHODASYNCCALLBACK( WASAPIRenderDevice, PausePlayback, OnPausePlayback );

    private:
        ~WASAPIRenderDevice();

        HRESULT OnStartPlayback( IMFAsyncResult* pResult );
        HRESULT OnStopPlayback( IMFAsyncResult* pResult );
        HRESULT OnPausePlayback( IMFAsyncResult* pResult );

        HRESULT ConfigureDeviceInternal();
        HRESULT ValidateBufferValue();

        virtual HRESULT OnAudioSampleRequested( Platform::Boolean IsSilence = false );
		virtual bool IsDeviceActive(DeviceState deviceState);

        HRESULT ConfigureSource();
        UINT32 GetBufferFramesPerPeriod();

        HRESULT GetToneSample( UINT32 FramesAvailable );
        HRESULT GetMFSample( UINT32 FramesAvailable );

    private:
        IAudioRenderClient *m_AudioRenderClient;
        IMFAsyncResult *m_SampleReadyAsyncResult;

		DEVICEPROPS m_DeviceProps;
		
		ToneSampleGenerator *m_ToneSource;
        MFSampleGenerator *m_MFSource;
    };
}

