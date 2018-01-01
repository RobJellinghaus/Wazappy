// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "WASAPISession.h"

using namespace Windows::System::Threading;
using namespace Wazappy;

NodeId WASAPISession::s_nextNodeId{};
std::mutex WASAPISession::s_mutex{};
std::map<NodeId, ComPtr<WASAPIDevice>> WASAPISession::s_deviceMap{};

void WASAPISession::RegisterDevice(const ComPtr<WASAPIDevice>& device)
{
	std::lock_guard<std::mutex> guard(s_mutex);
	s_deviceMap.emplace(device->GetNodeId(), device);
}

void WASAPISession::UnregisterDevice(NodeId nodeId)
{
	std::lock_guard<std::mutex> guard(s_mutex);
	Contract::Requires(s_deviceMap.find(nodeId) != s_deviceMap.end(), L"Device with given ID must exist");
	s_deviceMap.erase(nodeId);
}

WASAPIDevice* WASAPISession::GetDevice(NodeId nodeId)
{
	std::lock_guard<std::mutex> guard(s_mutex);
	const auto& found = s_deviceMap.find(nodeId);
	Contract::Requires(found != s_deviceMap.end(), L"Device with given ID must exist");
	return found->second.Get();
}

NodeId WASAPISession::GetNextNodeId()
{
	std::lock_guard<std::mutex> guard(s_mutex);
	return ++s_nextNodeId;
}