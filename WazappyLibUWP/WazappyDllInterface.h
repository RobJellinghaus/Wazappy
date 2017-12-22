// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

// This header is the only header file which external clients of this DLL should include.
// It defines the only dllexported APIs, and all of the exported types, in the library.

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

		enum ContentType
		{
			ContentType_Tone,
			ContentType_File
		};

		// Types of Wazappy nodes.
		enum WazappyNodeType
		{
			// Nonexistent value (to catch uninitialized nodes).
			NodeType_None,

			// Input device which captures incoming audio.
			NodeType_CaptureDevice,

			// Output device which renders outgoing audio.
			NodeType_RenderDevice
		};

		// A handle to a Wazappy node. 
		// No reference counting or even tracking is done over this interface; it works purely at the raw pointer level.
		// On the Wazappy side, debug builds never delete nodes, only mark them as tombstoned, with contracts catching
		// post-mortem access.  This at least catches use-after-free situations in debug builds.
		// Would be good to have something better than void* here (e.g. something stronger than a totally weak pointer),
		// but not initially....
		struct WazappyNodeHandle
		{
		public:
			WazappyNodeType nodeType;
			void* pointer;

			WazappyNodeHandle(WazappyNodeType nodeType, void* pointer) : nodeType(nodeType), pointer(pointer)
			{
				Contract::Requires(nodeType > WazappyNodeType::NodeType_None);
				Contract::Requires(pointer != nullptr);
			}

			WazappyNodeHandle(const WazappyNodeHandle& other) : nodeType(other.nodeType), pointer(other.pointer) {}

			WazappyNodeHandle& operator=(const WazappyNodeHandle& other)
			{
				nodeType = other.nodeType;
				pointer = other.pointer;
			}

			// Default node handle: invalid for all use
			WazappyNodeHandle() : nodeType{}, pointer{} {}

			bool IsValid() const { return nodeType != WazappyNodeType::NodeType_None; }
		};

		// Top-level methods which affect the entire session.
		// The session is semantically a singleton; that is, there is no WazappyNodeHandle for a session,
		// and these methods require no context.
		class __declspec(dllexport) WASAPISession
		{
		public:
			static HRESULT WASAPISession_SetVolumeOnSession(UINT32 volume);

			// Begin initializing the audio device(s).
			static HRESULT WASAPISession_InitializeAudioDeviceAsync();

			// Becomes true once the session is initialized.
			static BOOL WASAPISession_IsInitialized();

			// Get a node handle for the default capture device.
			// Can be called before IsInitialized().
			static WazappyNodeHandle WASAPISession_GetDefaultCaptureDevice();

			// Get a node handle for the default render device.
			// Can be called before IsInitialized().
			static WazappyNodeHandle WASAPISession_GetDefaultRenderDevice();

			// Close the session.  Will forcibly free all related objects and release all devices
			// and handles.
			// Can only be called after IsInitialized().
			// IsInitialized() becomes false after this method is called.
			static HRESULT WASAPISession_Close();
		};

		// Type of function pointer taking a void* and returning an HRESULT.
		// This is used for all callbacks from Wazappy to Wazappy clients; typically the void*
		// is 
		typedef HRESULT(__stdcall *DeviceStateCallback)(void* target, DeviceState deviceState);

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
		// The pointer field of a WASAPIRenderDevice handle is a 
		class __declspec(dllexport) WASAPIRenderDevice
		{
		public:
			// Get the current device state of this render device.
			static DeviceState WASAPIRenderDevice_GetDeviceState(WazappyNodeHandle handle);

			// Register a device state callback.
			static HRESULT WASAPIRenderDevice_RegisterDeviceStateChangeCallback(WazappyNodeHandle handle, DeviceStateCallback callback, void* target);

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

			// Register a device state callback.
			static HRESULT WASAPIRenderDevice_RegisterDeviceStateChangeCallback(WazappyNodeHandle handle, DeviceStateCallback callback, void* target);

			static HRESULT WASAPICaptureDevice_SetProperties(WazappyNodeHandle handle, CAPTUREDEVICEPROPS props);
			static HRESULT WASAPICaptureDevice_InitializeAudioDeviceAsync(WazappyNodeHandle handle);
			static HRESULT WASAPICaptureDevice_StartCaptureAsync(WazappyNodeHandle handle);
			static HRESULT WASAPICaptureDevice_StopCaptureAsync(WazappyNodeHandle handle);
			static HRESULT WASAPICaptureDevice_FinishCaptureAsync(WazappyNodeHandle handle);

		};
	}
}
