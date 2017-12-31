// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "WazappyDllInterface.h"

// Get a node handle for the default capture device.
// Can be called before IsInitialized().
Wazappy::WazappyNodeHandle Wazappy::WASAPISessionInterop::WASAPISession_GetDefaultCaptureDevice()
{
}

Wazappy::WazappyNodeHandle Wazappy::WASAPISessionInterop::WASAPISession_GetDefaultRenderDevice()
{
}

HRESULT Wazappy::WASAPIDeviceInterop::WASAPIDevice_SetVolumeOnSession(UINT32 volume)
{
}

HRESULT Wazappy::WASAPIDeviceInterop::WASAPIIDevice_InitializeAudioDeviceAsync()
{
}

BOOL Wazappy::WASAPIDeviceInterop::WASAPIIDevice_IsInitialized()
{
}

Wazappy::DeviceState Wazappy::WASAPIDeviceInterop::WASAPIDevice_GetDeviceState(Wazappy::WazappyNodeHandle handle)
{
}

HRESULT Wazappy::WASAPIDeviceInterop::WASAPIDevice_RegisterDeviceStateChangeCallbackHook(Wazappy::WazappyNodeHandle handle, DeviceStateCallback hook)
{
}

HRESULT Wazappy::WASAPIDeviceInterop::WASAPIDevice_RegisterDeviceStateChangeCallback(Wazappy::WazappyNodeHandle handle, CallbackId id)
{
}

HRESULT Wazappy::WASAPIDeviceInterop::WASAPIDevice_UnregisterDeviceStateChangeCallback(Wazappy::WazappyNodeHandle handle, CallbackId id)
{
}

HRESULT Wazappy::WASAPIRenderDeviceInterop::WASAPIRenderDevice_SetProperties(Wazappy::WazappyNodeHandle handle, DEVICEPROPS props)
{
}

HRESULT Wazappy::WASAPIRenderDeviceInterop::WASAPIRenderDevice_StartPlaybackAsync(Wazappy::WazappyNodeHandle handle)
{
}

HRESULT Wazappy::WASAPIRenderDeviceInterop::WASAPIRenderDevice_StopPlaybackAsync(Wazappy::WazappyNodeHandle handle)
{
}
		
HRESULT Wazappy::WASAPIRenderDeviceInterop::WASAPIRenderDevice_PausePlaybackAsync(Wazappy::WazappyNodeHandle handle)
{
}

HRESULT Wazappy::WASAPICaptureDeviceInterop::WASAPICaptureDevice_SetProperties(Wazappy::WazappyNodeHandle handle, CAPTUREDEVICEPROPS props)
{
}

HRESULT Wazappy::WASAPICaptureDeviceInterop::WASAPICaptureDevice_StartCaptureAsync(Wazappy::WazappyNodeHandle handle)
{
}

HRESULT Wazappy::WASAPICaptureDeviceInterop::WASAPICaptureDevice_StopCaptureAsync(Wazappy::WazappyNodeHandle handle)
{
}

HRESULT Wazappy::WASAPICaptureDeviceInterop::WASAPICaptureDevice_FinishCaptureAsync(Wazappy::WazappyNodeHandle handle)
{
}
