// Licensed under the MIT License.

#pragma once

#include "WazappyClient.h"
#include "DeviceStateChangedEvent.h"

Platform::Collections::Map<Wazappy::CallbackId, WazappyApp::DeviceStateChangedEvent^>^ s_callbackMap =
	ref new Platform::Collections::Map<Wazappy::CallbackId, WazappyApp::DeviceStateChangedEvent^>();

Wazappy::CallbackId s_nextId{};

std::mutex s_clientMutex{};

void WazappyApp::WazappyClient::DeviceStateChangedEventCallback(Wazappy::CallbackId id, Wazappy::DeviceState deviceState)
{
	std::lock_guard<std::mutex> methodGuard(s_clientMutex);
	Wazappy::Contract::Requires(s_callbackMap->HasKey(id));
	s_callbackMap->Lookup(id)->SetState(deviceState, S_OK, true);
}

Wazappy::CallbackId WazappyApp::WazappyClient::RegisterCallback(WazappyApp::DeviceStateChangedEvent^ deviceStateChangedEvent)
{
	std::lock_guard<std::mutex> methodGuard(s_clientMutex);
	Wazappy::CallbackId nextId = ++s_nextId;
	s_callbackMap->Insert(nextId, deviceStateChangedEvent);
	return nextId;
}

// Unregister the given callback.
// Is thread-safe (takes a global lock).
void WazappyApp::WazappyClient::UnregisterCallback(Wazappy::CallbackId id)
{
	std::lock_guard<std::mutex> methodGuard(s_clientMutex);
	Wazappy::Contract::Requires(s_callbackMap->HasKey(id));
	s_callbackMap->Remove(id);
}
