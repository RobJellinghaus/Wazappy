// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "WazappyDllInterface.h"
#include "WASAPICaptureDevice.h"
#include "WASAPIRenderDevice.h"
#include "WASAPISession.h"

using namespace Wazappy;

// Get a node handle for the default capture device.
// Can be called before IsInitialized().
WazappyNodeHandle WASAPISessionInterop::WASAPISession_GetDefaultCaptureDevice()
{
	ComPtr<WASAPICaptureDevice> captureDevice = Make<WASAPICaptureDevice>();
	Contract::Assert(captureDevice != nullptr);
	NodeId newDeviceId = WASAPISession::RegisterDevice(captureDevice);
	return WazappyNodeHandle(WazappyNodeType::NodeType_CaptureDevice, newDeviceId);
}

WazappyNodeHandle WASAPISessionInterop::WASAPISession_GetDefaultRenderDevice()
{
	ComPtr<WASAPIRenderDevice> device = Make<WASAPIRenderDevice>();
	Contract::Assert(device != nullptr);
	NodeId newDeviceId = WASAPISession::RegisterDevice(device);
	return WazappyNodeHandle(WazappyNodeType::NodeType_RenderDevice, newDeviceId);
}

template <typename TNode>
TNode* ResolveDevice(WazappyNodeHandle handle, WazappyNodeType expectedType)
{
	Contract::Requires(handle.nodeType == expectedType, L"Handle must be of expected type");
	return ResolveDevice<TNode>(handle);
}

template <typename TNode>
TNode* ResolveDevice(WazappyNodeHandle handle)
{
	const ComPtr<WASAPIDevice>& device = WASAPISession::GetDevice(handle.nodeId);
	return dynamic_cast<TNode*>(device.Get());
}

HRESULT WASAPIDeviceInterop::WASAPIDevice_SetVolumeOnSession(WazappyNodeHandle handle, UINT32 volume)
{
	WASAPIDevice* device = ResolveDevice<WASAPIDevice>(handle);
	return device->SetVolumeOnSession(volume);
}

HRESULT WASAPIDeviceInterop::WASAPIIDevice_InitializeAudioDeviceAsync(WazappyNodeHandle handle)
{
	WASAPIDevice* device = ResolveDevice<WASAPIDevice>(handle);
	return device->InitializeAudioDeviceAsync();
}

BOOL WASAPIDeviceInterop::WASAPIIDevice_IsInitialized(WazappyNodeHandle handle)
{
	WASAPIDevice* device = ResolveDevice<WASAPIDevice>(handle);
	return device->IsInitialized();
}

DeviceState WASAPIDeviceInterop::WASAPIDevice_GetDeviceState(WazappyNodeHandle handle)
{
	WASAPIDevice* device = ResolveDevice<WASAPIDevice>(handle);
	return device->GetDeviceState();
}

HRESULT WASAPIDeviceInterop::WASAPIDevice_RegisterDeviceStateChangeCallbackHook(DeviceStateCallback hook)
{
	WASAPIDevice::RegisterDeviceStateCallbackHook(hook);
	return S_OK;
}

HRESULT WASAPIDeviceInterop::WASAPIDevice_RegisterDeviceStateChangeCallback(WazappyNodeHandle handle, CallbackId id)
{
	WASAPIDevice::RegisterDeviceStateCallback(handle.nodeId, id);
	return S_OK;
}

HRESULT WASAPIDeviceInterop::WASAPIDevice_UnregisterDeviceStateChangeCallback(WazappyNodeHandle handle, CallbackId id)
{
	WASAPIDevice::UnregisterDeviceStateCallback(handle.nodeId, id);
	return S_OK;
}

HRESULT WASAPIRenderDeviceInterop::WASAPIRenderDevice_SetProperties(WazappyNodeHandle handle, DEVICEPROPS props)
{
	WASAPIRenderDevice* device = ResolveDevice<WASAPIRenderDevice>(handle, WazappyNodeType::NodeType_RenderDevice);
	return device->SetProperties(props);
}

HRESULT WASAPIRenderDeviceInterop::WASAPIRenderDevice_StartPlaybackAsync(WazappyNodeHandle handle)
{
	WASAPIRenderDevice* device = ResolveDevice<WASAPIRenderDevice>(handle, WazappyNodeType::NodeType_RenderDevice);
	return device->StartPlaybackAsync();
}

HRESULT WASAPIRenderDeviceInterop::WASAPIRenderDevice_StopPlaybackAsync(WazappyNodeHandle handle)
{
	WASAPIRenderDevice* device = ResolveDevice<WASAPIRenderDevice>(handle, WazappyNodeType::NodeType_RenderDevice);
	return device->StopPlaybackAsync();
}
		
HRESULT WASAPIRenderDeviceInterop::WASAPIRenderDevice_PausePlaybackAsync(WazappyNodeHandle handle)
{
	WASAPIRenderDevice* device = ResolveDevice<WASAPIRenderDevice>(handle, WazappyNodeType::NodeType_RenderDevice);
	return device->PausePlaybackAsync();
}

HRESULT WASAPICaptureDeviceInterop::WASAPICaptureDevice_SetProperties(WazappyNodeHandle handle, CAPTUREDEVICEPROPS props)
{
	WASAPICaptureDevice* device = ResolveDevice<WASAPICaptureDevice>(handle, WazappyNodeType::NodeType_CaptureDevice);
	return device->SetProperties(props);
}

HRESULT WASAPICaptureDeviceInterop::WASAPICaptureDevice_StartCaptureAsync(WazappyNodeHandle handle)
{
	WASAPICaptureDevice* device = ResolveDevice<WASAPICaptureDevice>(handle, WazappyNodeType::NodeType_CaptureDevice);
	return device->StartCaptureAsync();
}

HRESULT WASAPICaptureDeviceInterop::WASAPICaptureDevice_StopCaptureAsync(WazappyNodeHandle handle)
{
	WASAPICaptureDevice* device = ResolveDevice<WASAPICaptureDevice>(handle, WazappyNodeType::NodeType_CaptureDevice);
	return device->StopCaptureAsync();
}

HRESULT WASAPICaptureDeviceInterop::WASAPICaptureDevice_FinishCaptureAsync(WazappyNodeHandle handle)
{
	WASAPICaptureDevice* device = ResolveDevice<WASAPICaptureDevice>(handle, WazappyNodeType::NodeType_CaptureDevice);
	return device->FinishCaptureAsync();
}
