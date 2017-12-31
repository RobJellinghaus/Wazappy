// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "WASAPIRenderDevice.h"

using namespace Windows::System::Threading;
using namespace Wazappy;

//
//  WASAPIRenderer()
//
WASAPIRenderDevice::WASAPIRenderDevice() :
    m_AudioRenderClient( nullptr ),
    m_ToneSource( nullptr ),
    m_MFSource( nullptr )
{
}

//
//  ~WASAPIRenderer()
//
WASAPIRenderDevice::~WASAPIRenderDevice()
{
    SAFE_RELEASE( m_AudioRenderClient );
    SAFE_RELEASE( m_MFSource );
    SAFE_DELETE( m_ToneSource );
}

// Get the default render device's ID.
Platform::String^ WASAPIRenderDevice::GetDeviceId()
{
	// Get a string representing the Default Audio Device Renderer
	return MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default);
}

// Perform device-type-specific activation logic.
HRESULT WASAPIRenderDevice::ActivateCompletedInternal()
{
    // Get the render client
    HRESULT hr = m_AudioClient->GetService( __uuidof(IAudioRenderClient), (void**) &m_AudioRenderClient );
    if (FAILED( hr ))
    {
        goto exit;
    }

    // Everything succeeded
    m_DeviceState = DeviceState::Initialized;

exit:
    return hr;
}

//  Sets various properties that the user defines in the scenario
HRESULT WASAPIRenderDevice::SetProperties( DEVICEPROPS props )
{
    m_DeviceProps = props;
    return S_OK;
}

bool WASAPIRenderDevice::IsDeviceActive(DeviceState deviceState)
{
	return deviceState == DeviceState::Playing;
}

//  Get the time in seconds between passes of the audio device
UINT32 WASAPIRenderDevice::GetBufferFramesPerPeriod()
{
    REFERENCE_TIME defaultDevicePeriod = 0;
    REFERENCE_TIME minimumDevicePeriod = 0;

    if (m_DeviceProps.IsHWOffload)
    {
        return m_BufferFrames;
    }

    // Get the audio device period
    HRESULT hr = m_AudioClient->GetDevicePeriod( &defaultDevicePeriod, &minimumDevicePeriod);
    if (FAILED( hr ))
    {
        return 0;
    }

    double devicePeriodInSeconds;
    
    if (m_DeviceProps.IsLowLatency)
    {
        devicePeriodInSeconds = minimumDevicePeriod / (10000.0*1000.0);
    }
    else
    {
        devicePeriodInSeconds = defaultDevicePeriod / (10000.0*1000.0);
    }
    

    return static_cast<UINT32>( m_MixFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5 );
}

//
//  ConfigureDeviceInternal()
//
//  Sets additional playback parameters and opts into hardware offload
//
HRESULT WASAPIRenderDevice::ConfigureDeviceInternal()
{
    if (m_DeviceState != DeviceState::Activated)
    {
        return E_NOT_VALID_STATE;
    }

    HRESULT hr = S_OK;

    // Opt into HW Offloading.  If the endpoint does not support offload it will return AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE
    AudioClientProperties audioProps = {0};
    audioProps.cbSize = sizeof( AudioClientProperties );
    audioProps.bIsOffload = m_DeviceProps.IsHWOffload;
    audioProps.eCategory = AudioCategory_Media;

    if (m_DeviceProps.IsRawChosen && m_DeviceProps.IsRawSupported)
    {
        audioProps.Options = AUDCLNT_STREAMOPTIONS_RAW;
    }

    hr = m_AudioClient->SetClientProperties( &audioProps );
    if (FAILED( hr ))
    {
        return hr;
    }

    // This sample opens the device is shared mode so we need to find the supported WAVEFORMATEX mix format
    hr = m_AudioClient->GetMixFormat( &m_MixFormat );
    if (FAILED( hr ))
    {
        return hr;
    }

   // The wfx parameter below is optional (Its needed only for MATCH_FORMAT clients). Otherwise, wfx will be assumed 
   // to be the current engine format based on the processing mode for this stream
   hr = m_AudioClient->GetSharedModeEnginePeriod(m_MixFormat, &m_DefaultPeriodInFrames, &m_FundamentalPeriodInFrames, &m_MinPeriodInFrames, &m_MaxPeriodInFrames);
   if (FAILED( hr ))
   {
      return hr;
   }

    // Verify the user defined value for hardware buffer
    hr = ValidateBufferValue();

    return hr;
}

//
//  ValidateBufferValue()
//
//  Verifies the user specified buffer value for hardware offload
//
HRESULT WASAPIRenderDevice::ValidateBufferValue()
{
    HRESULT hr = S_OK;

    if (!m_DeviceProps.IsHWOffload)
    {
        // If we aren't using HW Offload, set this to 0 to use the default value
        m_DeviceProps.hnsBufferDuration = 0;
        return hr;
    }

    REFERENCE_TIME hnsMinBufferDuration;
    REFERENCE_TIME hnsMaxBufferDuration;

    hr = m_AudioClient->GetBufferSizeLimits( m_MixFormat, true, &hnsMinBufferDuration, &hnsMaxBufferDuration );
    if (SUCCEEDED( hr ))
    {
        if (m_DeviceProps.hnsBufferDuration < hnsMinBufferDuration)
        {
            // using MINIMUM size instead
            m_DeviceProps.hnsBufferDuration = hnsMinBufferDuration;
        }
        else if (m_DeviceProps.hnsBufferDuration > hnsMaxBufferDuration)
        {
            // using MAXIMUM size instead
            m_DeviceProps.hnsBufferDuration = hnsMaxBufferDuration;
        }
    }

    return hr;
}

//
//  ConfigureSource()
//
//  Configures tone or file playback
//
HRESULT WASAPIRenderDevice::ConfigureSource()
{
    HRESULT hr = S_OK;
    UINT32 FramesPerPeriod = GetBufferFramesPerPeriod();

    if (m_DeviceProps.IsTonePlayback)
    {
        // Generate the sine wave sample buffer
        m_ToneSource = new ToneSampleGenerator();
        if (m_ToneSource)
        {
            hr = m_ToneSource->GenerateSampleBuffer( m_DeviceProps.Frequency, FramesPerPeriod, m_MixFormat );
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        m_MFSource = new MFSampleGenerator();
        if (m_MFSource)
        {
            hr = m_MFSource->Initialize( /*B4CR: m_DeviceProps.ContentStream*/nullptr, FramesPerPeriod, m_MixFormat );
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

//
//  StartPlaybackAsync()
//
//  Starts asynchronous playback on a separate thread via MF Work Item
//
HRESULT WASAPIRenderDevice::StartPlaybackAsync()
{
    HRESULT hr = S_OK;

    // We should be stopped if the user stopped playback, or we should be
    // initialzied if this is the first time through getting ready to playback.
    if ( (m_DeviceState == DeviceState::Stopped) ||
         (m_DeviceState == DeviceState::Initialized) )
    {
        // Setup either ToneGeneration or File Playback
        hr = ConfigureSource();
        if (FAILED( hr ))
        {
            m_DeviceState = DeviceState::InError;
            return hr;
        }

        m_DeviceState = DeviceState::Starting;
        return MFPutWorkItem2( MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0, &m_xStartPlayback, nullptr );
    }
    else if (m_DeviceState == DeviceState::Paused)
    {
        return MFPutWorkItem2( MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0, &m_xStartPlayback, nullptr );
    }

    // Otherwise something else happened
    return E_FAIL;
}

//
//  OnStartPlayback()
//
//  Callback method to start playback
//
HRESULT WASAPIRenderDevice::OnStartPlayback( IMFAsyncResult* pResult )
{
    HRESULT hr = S_OK;

    // Pre-Roll the buffer with silence
    hr = OnAudioSampleRequested( true );
    if (FAILED( hr ))
    {
        goto exit;
    }

    // For MF Source playback, need to start the source reader
    if (!m_DeviceProps.IsTonePlayback)
    {
        hr = m_MFSource->StartSource();
        if (FAILED( hr ))
        {
            goto exit;
        }
    }

    // Actually start the playback
    hr = m_AudioClient->Start();
    if (SUCCEEDED( hr ))
    {
        m_DeviceState = DeviceState::Playing;
        hr = MFPutWaitingWorkItem( m_SampleReadyEvent, 0, m_SampleReadyAsyncResult, &m_SampleReadyKey );
    }

exit:
    if (FAILED( hr ))
    {
        m_DeviceState = DeviceState::InError;
    }

    return S_OK;
}

//
//  StopPlaybackAsync()
//
//  Stop playback asynchronously via MF Work Item
//
HRESULT WASAPIRenderDevice::StopPlaybackAsync()
{
    if ( (m_DeviceState != DeviceState::Playing) &&
         (m_DeviceState != DeviceState::Paused) &&
         (m_DeviceState != DeviceState::InError) )
    {
        return E_NOT_VALID_STATE;
    }

    m_DeviceState = DeviceState::Stopping;

    return MFPutWorkItem2( MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0, &m_xStopPlayback, nullptr );
}

//
//  OnStopPlayback()
//
//  Callback method to stop playback
//
HRESULT WASAPIRenderDevice::OnStopPlayback( IMFAsyncResult* pResult )
{
    // Stop playback by cancelling Work Item
    // Cancel the queued work item (if any)
    if (0 != m_SampleReadyKey)
    {
        MFCancelWorkItem( m_SampleReadyKey );
        m_SampleReadyKey = 0;
    }

    // Flush anything left in buffer with silence
    OnAudioSampleRequested( true );

    m_AudioClient->Stop();
    SAFE_RELEASE( m_SampleReadyAsyncResult );

    if (m_DeviceProps.IsTonePlayback)
    {
        // Flush remaining buffers
        m_ToneSource->Flush();
    }
    else
    {
        // Stop Source and Flush remaining buffers
        m_MFSource->StopSource();
        m_MFSource->Shutdown();
    }

    m_DeviceState = DeviceState::Stopped;
    return S_OK;
}

//
//  PausePlaybackAsync()
//
//  Pause playback asynchronously via MF Work Item
//
HRESULT WASAPIRenderDevice::PausePlaybackAsync()
{
    if ( (m_DeviceState !=  DeviceState::Playing) &&
         (m_DeviceState != DeviceState::InError) )
    {
        return E_NOT_VALID_STATE;
    }

    // Change state to stop automatic queueing of samples
    m_DeviceState = DeviceState::Pausing;
    return MFPutWorkItem2( MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0, &m_xPausePlayback, nullptr );

}

//
//  OnPausePlayback()
//
//  Callback method to pause playback
//
HRESULT WASAPIRenderDevice::OnPausePlayback( IMFAsyncResult* pResult )
{
    m_AudioClient->Stop();
    m_DeviceState = DeviceState::Paused;
    return S_OK;
}

//
//  OnAudioSampleRequested()
//
//  Called when audio device fires m_SampleReadyEvent
//
HRESULT WASAPIRenderDevice::OnAudioSampleRequested( Platform::Boolean IsSilence )
{
    HRESULT hr = S_OK;
    UINT32 PaddingFrames = 0;
    UINT32 FramesAvailable = 0;

    EnterCriticalSection( &m_CritSec );

    // Get padding in existing buffer
    hr = m_AudioClient->GetCurrentPadding( &PaddingFrames );
    if (FAILED( hr ))
    {
        goto exit;
    }

    // Audio frames available in buffer
    if (m_DeviceProps.IsHWOffload)
    {
        // In HW mode, GetCurrentPadding returns the number of available frames in the 
        // buffer, so we can just use that directly
        FramesAvailable = PaddingFrames;
    }
    else
    {
        // In non-HW shared mode, GetCurrentPadding represents the number of queued frames
        // so we can subtract that from the overall number of frames we have
        FramesAvailable = m_BufferFrames - PaddingFrames;
    }

    // Only continue if we have buffer to write data
    if (FramesAvailable > 0)
    {
        if (IsSilence)
        {
            BYTE *Data;

            // Fill the buffer with silence
            hr = m_AudioRenderClient->GetBuffer( FramesAvailable, &Data );
            if (FAILED( hr ))
            {
                goto exit;
            }

            hr = m_AudioRenderClient->ReleaseBuffer( FramesAvailable, AUDCLNT_BUFFERFLAGS_SILENT );
            goto exit;
        }

        // Even if we cancel a work item, this may still fire due to the async
        // nature of things.  There should be a queued work item already to handle
        // the process of stopping or stopped
        if (m_DeviceState == DeviceState::Playing)
        {
            // Fill the buffer with a playback sample
            if (m_DeviceProps.IsTonePlayback)
            {
                hr = GetToneSample( FramesAvailable );
            }
            else
            {
                hr = GetMFSample( FramesAvailable );
            }
        }
    }

exit:
    LeaveCriticalSection( &m_CritSec );

    if (AUDCLNT_E_RESOURCES_INVALIDATED == hr)
    {
        m_DeviceState = DeviceState::Uninitialized;
        SAFE_RELEASE( m_AudioClient );
        SAFE_RELEASE( m_AudioRenderClient );
        SAFE_RELEASE( m_SampleReadyAsyncResult );

        hr = InitializeAudioDeviceAsync();
    }

    return hr;
}

//
//  GetToneSample()
//
//  Fills buffer with a tone sample
//
HRESULT WASAPIRenderDevice::GetToneSample( UINT32 FramesAvailable )
{
    HRESULT hr = S_OK;
    BYTE *Data;

    // Post-Roll Silence
    if (m_ToneSource->IsEOF())
    {
        hr = m_AudioRenderClient->GetBuffer( FramesAvailable, &Data );
        if (SUCCEEDED( hr ))
        {
            // Ignore the return
            hr = m_AudioRenderClient->ReleaseBuffer( FramesAvailable, AUDCLNT_BUFFERFLAGS_SILENT );
        }

        StopPlaybackAsync();
    }
    else if (m_ToneSource->GetBufferLength() <= ( FramesAvailable * m_MixFormat->nBlockAlign ))
    {
        UINT32 ActualFramesToRead = m_ToneSource->GetBufferLength() / m_MixFormat->nBlockAlign;
        UINT32 ActualBytesToRead = ActualFramesToRead * m_MixFormat->nBlockAlign;

        hr = m_AudioRenderClient->GetBuffer( ActualFramesToRead, &Data );
        if (SUCCEEDED( hr ))
        {
            hr = m_ToneSource->FillSampleBuffer( ActualBytesToRead, Data );
            if (SUCCEEDED( hr ))
            {
                hr = m_AudioRenderClient->ReleaseBuffer( ActualFramesToRead, 0 );
            }
        }
    }

    return hr;
}

//
//  GetMFSample()
//
//  Fills buffer with a MF sample
//
HRESULT WASAPIRenderDevice::GetMFSample( UINT32 FramesAvailable )
{
    HRESULT hr = S_OK;
    BYTE *Data = nullptr;

    // Post-Roll Silence
    if (m_MFSource->IsEOF())
    {
        hr = m_AudioRenderClient->GetBuffer( FramesAvailable, &Data );
        if (SUCCEEDED( hr ))
        {
            // Ignore the return
            hr = m_AudioRenderClient->ReleaseBuffer( FramesAvailable, AUDCLNT_BUFFERFLAGS_SILENT );
        }

        StopPlaybackAsync();
    }
    else
    {
        UINT32 ActualBytesRead = 0;
        UINT32 ActualBytesToRead = FramesAvailable * m_MixFormat->nBlockAlign;

        hr = m_AudioRenderClient->GetBuffer( FramesAvailable, &Data );
        if (SUCCEEDED( hr ))
        {
            hr = m_MFSource->FillSampleBuffer( ActualBytesToRead, Data, &ActualBytesRead );
            if (hr == S_FALSE)
            {
                // Hit EOS
                hr = m_AudioRenderClient->ReleaseBuffer( FramesAvailable, AUDCLNT_BUFFERFLAGS_SILENT );
                StopPlaybackAsync();
            }
            else if (SUCCEEDED( hr ))
            {
                // This can happen if we are pre-rolling so just insert silence
                if (0 == ActualBytesRead)
                {
                    hr = m_AudioRenderClient->ReleaseBuffer( FramesAvailable, AUDCLNT_BUFFERFLAGS_SILENT );
                }
                else
                {
                    hr = m_AudioRenderClient->ReleaseBuffer( ( ActualBytesRead / m_MixFormat->nBlockAlign ), 0 );
                }
            }
        }
    }

    return hr;
}

