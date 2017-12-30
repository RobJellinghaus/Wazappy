// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "WASAPIDevice.h"

using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;

#define AUDIO_FILE_NAME "WASAPIAudioCapture.wav"
#define FLUSH_INTERVAL_SEC 3


#pragma once

namespace Wazappy
{
    // Primary WASAPI Capture Class
    class WASAPICaptureDevice : public WASAPIDevice
    {
    public:
        WASAPICaptureDevice();

        HRESULT SetProperties(CAPTUREDEVICEPROPS props);

        HRESULT InitializeAudioDeviceAsync();
        HRESULT StartCaptureAsync();
        HRESULT StopCaptureAsync();
        HRESULT FinishCaptureAsync();

        METHODASYNCCALLBACK( WASAPICaptureDevice, StartCapture, OnStartCapture );
        METHODASYNCCALLBACK( WASAPICaptureDevice, StopCapture, OnStopCapture );
        METHODASYNCCALLBACK( WASAPICaptureDevice, SampleReady, OnSampleReady );
        METHODASYNCCALLBACK( WASAPICaptureDevice, FinishCapture, OnFinishCapture );
        METHODASYNCCALLBACK( WASAPICaptureDevice, SendScopeData, OnSendScopeData );

        // IActivateAudioInterfaceCompletionHandler
        STDMETHOD(ActivateCompleted)( IActivateAudioInterfaceAsyncOperation *operation );

    private:
        ~WASAPICaptureDevice();

        HRESULT OnStartCapture( IMFAsyncResult* pResult );
        HRESULT OnStopCapture( IMFAsyncResult* pResult );
        HRESULT OnFinishCapture( IMFAsyncResult* pResult );
        HRESULT OnSampleReady( IMFAsyncResult* pResult );
        HRESULT OnSendScopeData( IMFAsyncResult* pResult );

        HRESULT CreateWAVFile();
        HRESULT FixWAVHeader();

		virtual HRESULT OnAudioSampleRequested( Platform::Boolean IsSilence = false );
		virtual bool IsDeviceActive(DeviceState deviceState);

        HRESULT InitializeScopeData();
        HRESULT ProcessScopeData( BYTE* pData, DWORD cbBytes );
        
    private:
        DWORD m_dwQueueID;

        DWORD m_cbHeaderSize;
        DWORD m_cbDataSize;
        DWORD m_cbFlushCounter;
        BOOL m_fWriting;

        IRandomAccessStream^ m_ContentStream;
        IOutputStream^ m_OutputStream;
        DataWriter^ m_WAVDataWriter;

        IAudioCaptureClient *m_AudioCaptureClient;

        Platform::Array<int, 1>^ m_PlotData;
        UINT32 m_cPlotDataMax;
        UINT32 m_cPlotDataFilled;

        CAPTUREDEVICEPROPS m_DeviceProps;
    };
}
