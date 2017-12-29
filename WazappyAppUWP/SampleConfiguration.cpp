// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "MainPage.xaml.h"
#include "SampleConfiguration.h"

using namespace WazappyApp;

Platform::Array<Scenario>^ MainPage::scenariosInner = ref new Platform::Array<Scenario>
{
    { "Device Enumeration", "Wazappy.Scenario1" },
    { "Audio Rendering with Hardware Offload", "Wazappy.Scenario2" },
    { "Audio Rendering with Low Latency", "Wazappy.Scenario3" },
    { "PCM Audio Capture", "Wazappy.Scenario4" }
};
