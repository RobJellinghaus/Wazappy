// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "WASAPIDevice.h"
#include "WASAPISession.h"

using namespace Windows::System::Threading;
using namespace Wazappy;

WASAPIDevice::WASAPIDevice() :
	m_nodeId(WASAPISession::GetNextNodeId()),
	m_BufferFrames(0),
	m_DeviceState(DeviceState::Uninitialized),
	m_AudioClient(nullptr),
	m_SampleReadyAsyncResult(nullptr)
{
	// Create events for sample ready or user stop
	m_SampleReadyEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
	if (nullptr == m_SampleReadyEvent)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	if (!InitializeCriticalSectionEx(&m_CritSec, 0, 0))
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

WASAPIDevice::~WASAPIDevice()
{
	SAFE_RELEASE(m_AudioClient);
	SAFE_RELEASE(m_SampleReadyAsyncResult);

	if (INVALID_HANDLE_VALUE != m_SampleReadyEvent)
	{
		CloseHandle(m_SampleReadyEvent);
		m_SampleReadyEvent = INVALID_HANDLE_VALUE;
	}

	DeleteCriticalSection(&m_CritSec);
}

//
//  InitializeAudioDeviceAsync()
//
//  Activates the default audio renderer on a asynchronous callback thread.  This needs
//  to be called from the main UI thread.
//
HRESULT WASAPIDevice::InitializeAudioDeviceAsync()
{
	IActivateAudioInterfaceAsyncOperation *asyncOp;
	HRESULT hr = S_OK;

	// Get a string representing the Default Audio Device Renderer
	m_DeviceIdString = GetDeviceId();

	// This call must be made on the main UI thread.  Async operation will call back to 
	// IActivateAudioInterfaceCompletionHandler::ActivateCompleted, which must be an agile interface implementation
	hr = ActivateAudioInterfaceAsync(m_DeviceIdString->Data(), __uuidof(IAudioClient3), nullptr, this, &asyncOp);
	if (FAILED(hr))
	{
		SetDeviceStateAndNotifyCallbacks(DeviceState::InError, true);
	}

	SAFE_RELEASE(asyncOp);

	return hr;
}

//
//  ActivateCompleted()
//
//  Callback implementation of ActivateAudioInterfaceAsync function.  This will be called on MTA thread
//  when results of the activation are available.
//
HRESULT WASAPIDevice::ActivateCompleted(IActivateAudioInterfaceAsyncOperation *operation)
{
	HRESULT hr = S_OK;
	HRESULT hrActivateResult = S_OK;
	IUnknown *punkAudioInterface = nullptr;

	if (GetDeviceState() != DeviceState::Uninitialized)
	{
		hr = E_NOT_VALID_STATE;
		goto exit;
	}

	// Check for a successful activation result
	hr = operation->GetActivateResult(&hrActivateResult, &punkAudioInterface);
	if (SUCCEEDED(hr) && SUCCEEDED(hrActivateResult))
	{
		// TODO: Note "false" here -- this is from original Windows sample logic... not sure still well motivated.
		SetDeviceStateAndNotifyCallbacks(DeviceState::Activated, false);

		// Get the pointer for the Audio Client
		punkAudioInterface->QueryInterface(IID_PPV_ARGS(&m_AudioClient));
		if (nullptr == m_AudioClient)
		{
			hr = E_FAIL;
			goto exit;
		}

		// Configure user defined properties
		hr = ConfigureDeviceInternal();
		if (FAILED(hr))
		{
			goto exit;
		}

		hr = m_AudioClient->InitializeSharedAudioStream(
			AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
			m_MinPeriodInFrames,
			m_MixFormat,
			nullptr);

		if (FAILED(hr))
		{
			goto exit;
		}

		// Get the maximum size of the AudioClient Buffer
		hr = m_AudioClient->GetBufferSize(&m_BufferFrames);
		if (FAILED(hr))
		{
			goto exit;
		}

		// Create Async callback for sample events
		hr = MFCreateAsyncResult(nullptr, &m_xSampleReady, nullptr, &m_SampleReadyAsyncResult);
		if (FAILED(hr))
		{
			goto exit;
		}

		// Sets the event handle that the system signals when an audio buffer is ready to be processed by the client
		hr = m_AudioClient->SetEventHandle(m_SampleReadyEvent);
		if (FAILED(hr))
		{
			goto exit;
		}

		// Now the subclass should execute, and should set the state to initialized once done.
		hr = ActivateCompletedInternal();
	}

exit:
	SAFE_RELEASE(punkAudioInterface);

	if (FAILED(hr))
	{
		SetDeviceStateAndNotifyCallbacks(DeviceState::InError, true);
		SAFE_RELEASE(m_AudioClient);
		SAFE_RELEASE(m_SampleReadyAsyncResult);
	}

	// Need to return S_OK
	return S_OK;
}

//
//  SetVolumeOnSession()
//
HRESULT WASAPIDevice::SetVolumeOnSession(UINT32 volume)
{
	if (volume > 100)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;
	ISimpleAudioVolume *SessionAudioVolume = nullptr;
	float ChannelVolume = 0.0;

	hr = m_AudioClient->GetService(__uuidof(ISimpleAudioVolume), reinterpret_cast<void**>(&SessionAudioVolume));
	if (FAILED(hr))
	{
		goto exit;
	}

	ChannelVolume = volume / (float)100.0;

	// Set the session volume on the endpoint
	hr = SessionAudioVolume->SetMasterVolume(ChannelVolume, nullptr);

exit:
	SAFE_RELEASE(SessionAudioVolume);
	return hr;
}

HRESULT WASAPIDevice::CreateWorkItemWaitingForSampleReadyEvent()
{
	return MFPutWaitingWorkItem(m_SampleReadyEvent, 0, m_SampleReadyAsyncResult, &m_SampleReadyKey);
}

HRESULT WASAPIDevice::CancelWorkItemWaitingForSampleReadyEvent()
{
	// Stop playback by cancelling Work Item
	// Cancel the queued work item (if any)
	if (0 != m_SampleReadyKey)
	{
		MFCancelWorkItem(m_SampleReadyKey);
		m_SampleReadyKey = 0;
	}
}

//
//  OnSampleReady()
//
//  Callback method when ready to fill sample buffer
//
HRESULT WASAPIDevice::OnSampleReady(IMFAsyncResult* pResult)
{
	HRESULT hr = S_OK;

	hr = OnAudioSampleRequested(false);

	if (SUCCEEDED(hr))
	{
		// Re-queue work item for next sample
		if (IsDeviceActive(GetDeviceState()))
		{
			hr = MFPutWaitingWorkItem(m_SampleReadyEvent, 0, m_SampleReadyAsyncResult, &m_SampleReadyKey);
		}
	}
	else
	{
		SetDeviceStateAndNotifyCallbacks(DeviceState::InError, true);
	}

	return hr;
}

DeviceStateCallback WASAPIDevice::s_deviceStateCallbackHook{};

std::map<NodeId, std::map<CallbackId, bool>> WASAPIDevice::s_deviceStateChangedCallbacks{};

std::mutex WASAPIDevice::s_deviceStateChangedCallbackMutex{};

void WASAPIDevice::RegisterDeviceStateCallbackHook(DeviceStateCallback hook)
{
	s_deviceStateCallbackHook = hook;
}

void WASAPIDevice::RegisterDeviceStateCallback(NodeId node, CallbackId callback)
{
	std::lock_guard<std::mutex> guard(s_deviceStateChangedCallbackMutex);
	auto& iter = s_deviceStateChangedCallbacks.find(node);
	if (iter != s_deviceStateChangedCallbacks.end())
	{
		auto& map = iter->second;
		Contract::Requires(map.find(callback) == map.end(), L"Must not post same callback on same node twice");
	}
	s_deviceStateChangedCallbacks[node].emplace(callback, true);
}

void WASAPIDevice::UnregisterDeviceStateCallback(NodeId node, CallbackId callback)
{
	std::lock_guard<std::mutex> guard(s_deviceStateChangedCallbackMutex);
	auto& iter = s_deviceStateChangedCallbacks.find(node);
	Contract::Requires(iter != s_deviceStateChangedCallbacks.end(), L"Some callback(s) must be registered on given node");
	auto& iter2 = iter->second.find(callback);
	Contract::Requires(iter2 != iter->second.end(), L"Given callback must be registered on given node");
	iter->second.erase(callback);
}

void WASAPIDevice::SetDeviceStateAndNotifyCallbacks(DeviceState newDeviceState, bool fireEvent)
{
	std::lock_guard<std::mutex> guard(s_deviceStateChangedCallbackMutex);
	if (fireEvent)
	{
		auto& iter = s_deviceStateChangedCallbacks.find(m_nodeId);
		for (auto& callback : iter->second)
		{
			s_deviceStateCallbackHook(callback.first, newDeviceState);
		}
	}
}
