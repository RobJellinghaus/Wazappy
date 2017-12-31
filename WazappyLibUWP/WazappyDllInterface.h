// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

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

		// Types of Wazappy nodes, corresponding to concrete subclasses.
		enum WazappyNodeType
		{
			// Nonexistent value (to catch uninitialized nodes).
			NodeType_None,

			// Input device which captures incoming audio.
			NodeType_CaptureDevice,

			// Output device which renders outgoing audio.
			NodeType_RenderDevice
		};

		// The ID of a WASAPI node object; avoids issues with marshaling object references.
		typedef int NodeId;

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
			NodeId nodeId;

			WazappyNodeHandle(WazappyNodeType nodeType, NodeId nodeId) : nodeType(nodeType), nodeId(nodeId)
			{
				Contract::Requires(nodeType > WazappyNodeType::NodeType_None);
				Contract::Requires(nodeId != (NodeId)0);
			}

			WazappyNodeHandle(const WazappyNodeHandle& other) : nodeType(other.nodeType), nodeId(other.nodeId) {}

			WazappyNodeHandle& operator=(const WazappyNodeHandle& other)
			{
				nodeType = other.nodeType;
				nodeId = other.nodeId;
				return *this;
			}

			// Default node handle: invalid for all use
			WazappyNodeHandle() : nodeType{}, nodeId{} {}

			bool IsValid() const { return nodeType != WazappyNodeType::NodeType_None; }
		};

		// Top-level methods which affect the entire session.
		// The session is semantically a singleton; that is, there is no WazappyNodeHandle for a session,
		// and these methods require no context.
		// TODO: make this actually be a thing, by figuring out how to support multiple devices, by refactoring device state into session.
		class __declspec(dllexport) WASAPISessionInterop
		{
		public:
			// Get a node handle for the default capture device.
			// Can be called before IsInitialized().
			static WazappyNodeHandle WASAPISession_GetDefaultCaptureDevice();

			// Get a node handle for the default render device.
			// Can be called before IsInitialized().
			static WazappyNodeHandle WASAPISession_GetDefaultRenderDevice();
		};

		// The ID of a callback object; avoids issues with marshaling function pointers.
		typedef int CallbackId;

		// Callback which updates a deviceState.  The CallbackId is passed in when registering.
		typedef HRESULT(__stdcall *DeviceStateCallback)(CallbackId target, DeviceState deviceState);

		/*
		class __declspec(dllexport) WASAPINodeInterop
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

		// Methods specific to Devices; all handles must be Devices.
		class __declspec(dllexport) WASAPIDeviceInterop
		{
		public:
			static HRESULT WASAPIDevice_SetVolumeOnSession(WazappyNodeHandle handle, UINT32 volume);

			// Begin initializing the audio device(s).
			// TODO: clarify the lifecycle of this, the exclusivity versus other devices, etc.
			static HRESULT WASAPIIDevice_InitializeAudioDeviceAsync(WazappyNodeHandle handle);

			// Becomes true once the session is initialized.
			static BOOL WASAPIIDevice_IsInitialized(WazappyNodeHandle handle);

			// Get the current device state of this device.
			static DeviceState WASAPIDevice_GetDeviceState(WazappyNodeHandle handle);

			// Register the device state callback hook, used for dispatching all callbacks.
			static HRESULT WASAPIDevice_RegisterDeviceStateChangeCallbackHook(DeviceStateCallback hook);

			// Register a particular callback on this node, by its ID.
			static HRESULT WASAPIDevice_RegisterDeviceStateChangeCallback(WazappyNodeHandle handle, CallbackId id);
			// Unregister a particular callback on this node, by its ID.
			static HRESULT WASAPIDevice_UnregisterDeviceStateChangeCallback(WazappyNodeHandle handle, CallbackId id);
		};

		// Methods specific to RenderDevices; all handles must be RenderDevices.
		class __declspec(dllexport) WASAPIRenderDeviceInterop
		{
		public:
			static HRESULT WASAPIRenderDevice_SetProperties(WazappyNodeHandle handle, DEVICEPROPS props);
			static HRESULT WASAPIRenderDevice_StartPlaybackAsync(WazappyNodeHandle handle);
			static HRESULT WASAPIRenderDevice_StopPlaybackAsync(WazappyNodeHandle handle);
			static HRESULT WASAPIRenderDevice_PausePlaybackAsync(WazappyNodeHandle handle);
		};

		// Methods specific to CaptureDevices; all handles must be CaptureDevices.
		class __declspec(dllexport) WASAPICaptureDeviceInterop
		{
		public:
			static HRESULT WASAPICaptureDevice_SetProperties(WazappyNodeHandle handle, CAPTUREDEVICEPROPS props);
			static HRESULT WASAPICaptureDevice_StartCaptureAsync(WazappyNodeHandle handle);
			static HRESULT WASAPICaptureDevice_StopCaptureAsync(WazappyNodeHandle handle);
			static HRESULT WASAPICaptureDevice_FinishCaptureAsync(WazappyNodeHandle handle);
		};
	}
}
