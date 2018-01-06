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
		
		bool IsInitialized() { return m_DeviceState >= DeviceState::Initialized; }

		virtual Platform::String^ GetDeviceId() = 0;
		virtual HRESULT ConfigureDeviceInternal() = 0;

		HRESULT SetVolumeOnSession(UINT32 volume);

		NodeId GetNodeId() { return m_nodeId; }
		DeviceState GetDeviceState() { return m_DeviceState; }

		METHODASYNCCALLBACK(WASAPIDevice, SampleReady, OnSampleReady);

		// IActivateAudioInterfaceCompletionHandler
		STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation *operation);

		// Subtypes override this method to perform additional logic on activation.
		virtual HRESULT ActivateCompletedInternal() = 0;

	public:
		// Register a hook for all device state change callbacks.
		static void RegisterDeviceStateCallbackHook(DeviceStateCallback hook);

		// Register a particular callback for a particular node.
		static void RegisterDeviceStateCallback(NodeId node, CallbackId callback);

		// Unregister a particular callback for a particular node.
		static void UnregisterDeviceStateCallback(NodeId node, CallbackId callback);

	private:
		HRESULT OnSampleReady(IMFAsyncResult* pResult);

		// An audio sample is requested by the device.
		virtual HRESULT OnAudioSampleRequested(Platform::Boolean IsSilence = false) = 0;

		// Returns true if the device is currently active (e.g. if sample-ready work items should continue to be queued).
		virtual bool IsDeviceActive(DeviceState deviceState) = 0;

	protected:
		// Update the device state, calling any callbacks.
		// This calls the callback hook for each callback registered on this device; the callback hook is called from the caller thread.
		// TODO: make the callback hook use work items for invoking the actual callbacks on a worker thread, to isolate the audio graph
		// from the client.
		void SetDeviceStateAndNotifyCallbacks(DeviceState newState, bool fireEvent);

		// Create a work item waiting for the sample ready event.
		HRESULT CreateWorkItemWaitingForSampleReadyEvent();

		// Cancel the work item (if any) which was waiting for the sample ready event.
		HRESULT CancelWorkItemWaitingForSampleReadyEvent();

	private:
		// The single callback for all DeviceState-changed events.
		static DeviceStateCallback s_deviceStateCallbackHook;

		// Map from nodes to DeviceStateChanged callbacks for each node.
		static std::map<NodeId, std::map<CallbackId, bool>> s_deviceStateChangedCallbacks;
		
		// Mutex for operating over callbacks.
		static std::mutex s_deviceStateChangedCallbackMutex;

	protected:
		virtual ~WASAPIDevice();

		IAudioClient3 *m_AudioClient;

		UINT32 m_BufferFrames;

		WAVEFORMATEX *m_MixFormat;

		UINT32 m_DefaultPeriodInFrames;
		UINT32 m_FundamentalPeriodInFrames;
		UINT32 m_MaxPeriodInFrames;
		UINT32 m_MinPeriodInFrames;

	private:
		const NodeId m_nodeId;
		Platform::String^ m_DeviceIdString;

		HANDLE m_SampleReadyEvent;
		MFWORKITEM_KEY m_SampleReadyKey;
		IMFAsyncResult *m_SampleReadyAsyncResult;

		CRITICAL_SECTION m_CritSec;

		DeviceState m_DeviceState;
	};
}

