// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

#include "Common.h"
#include "Contract.h"
#include <mfapi.h>
#include <AudioClient.h>
#include <mmdeviceapi.h>

#include "DeviceState.h"

namespace Wazappy
{
	extern "C"
	{
		// User Configurable Arguments for Scenario
		struct DEVICEPROPS
		{
			BOOL IsHWOffload;
			BOOL IsTonePlayback;
			BOOL IsBackground;
			BOOL IsRawSupported;
			BOOL IsRawChosen;
			BOOL IsLowLatency;
			REFERENCE_TIME hnsBufferDuration;
			DWORD Frequency;
		};

		// User Configurable Arguments for Scenario
		struct CAPTUREDEVICEPROPS
		{
			BOOL IsLowLatency;
		};
	}

	public enum class ContentType
	{
		ContentType_Tone,
		ContentType_File
	};

	// Types of Wazappy nodes.
	public enum class WazappyNodeType
	{
		// Nonexistent value (to catch uninitialized nodes).
		NodeType_None,

		// Input device which captures incoming audio.
		NodeType_CaptureDevice,

		// Output device which renders outgoing audio.
		NodeType_RenderDevice
	};

	extern "C"
	{
		// A handle to a Wazappy node. 
		struct WazappyNodeHandle
		{
		public:
			const WazappyNodeType nodeType;
			const size_t id;

			WazappyNodeHandle(WazappyNodeType nodeType, size_t id) : nodeType(nodeType), id(id)
			{
				Contract::Requires(nodeType > WazappyNodeType::NodeType_None);
				Contract::Requires(id > 0);
			}

			WazappyNodeHandle(const WazappyNodeHandle& other) : nodeType(other.nodeType), id(other.id) {}

			// Default node handle: invalid for all use
			WazappyNodeHandle() : nodeType{}, id{} {}

			bool IsValid() const { return nodeType != WazappyNodeType::NodeType_None; }
		};

		// Top-level methods which affect the entire session.
		class __declspec(dllexport) WASAPISession
		{
		public:
			static HRESULT WASAPISession_SetVolumeOnSession(UINT32 volume);

			// Begin initializing the audio device(s).
			static HRESULT WASAPISession_InitializeAudioDeviceAsync();

			// Becomes true once the session is initialized.
			static BOOL WASAPISession_IsInitialized();

			// Get a node handle for the default capture device
			static WazappyNodeHandle WASAPISession_GetDefaultCaptureDevice();
			// Get a node handle for the default render device.
			static WazappyNodeHandle WASAPISession_GetDefaultRenderDevice();
		};

		/*
		class __declspec(dllexport) WASAPINode
		{
		public:
			// Add an incoming conection from the given upstream node.
			// The upstream node must not already be an incoming connection, even transitively.
			static HRESULT AddIncomingConnection(WazappyNodeHandle upstreamNode);

			// Remove an incoming connection from the given upstream node.
			// The upstream node must be a (direct) incoming connection.
			static HRESULT RemoveIncomingConnection(WazappyNodeHandle upstreamNode);
		};
		*/

		// Methods specific to RenderDevices; all handles must be RenderDevices.
		class __declspec(dllexport) WASAPIRenderDevice
		{
		public:
			static DeviceState WASAPIRenderDevice_GetDeviceState(WazappyNodeHandle handle);

			static HRESULT WASAPIRenderDevice_SetProperties(WazappyNodeHandle handle, DEVICEPROPS props);
			static HRESULT WASAPIRenderDevice_StartPlaybackAsync(WazappyNodeHandle handle);
			static HRESULT WASAPIRenderDevice_StopPlaybackAsync(WazappyNodeHandle handle);
			static HRESULT WASAPIRenderDevice_PausePlaybackAsync(WazappyNodeHandle handle);
		};

		// Methods specific to CaptureDevices; all handles must be CaptureDevices.
		class __declspec(dllexport) WASAPICaptureDevice
		{
		public:
			static DeviceState WASAPICaptureDevice_GetDeviceState(WazappyNodeHandle handle);

			static HRESULT WASAPICaptureDevice_SetProperties(WazappyNodeHandle handle, CAPTUREDEVICEPROPS props);
			static HRESULT WASAPICaptureDevice_InitializeAudioDeviceAsync(WazappyNodeHandle handle);
			static HRESULT WASAPICaptureDevice_StartCaptureAsync(WazappyNodeHandle handle);
			static HRESULT WASAPICaptureDevice_StopCaptureAsync(WazappyNodeHandle handle);
			static HRESULT WASAPICaptureDevice_FinishCaptureAsync(WazappyNodeHandle handle);

		};
	}
}
