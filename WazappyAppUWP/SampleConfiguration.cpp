// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "MainPage.xaml.h"
#include "SampleConfiguration.h"

using namespace WazappyApp;

Platform::Array<Scenario>^ MainPage::scenariosInner = ref new Platform::Array<Scenario>
{
    { "Device Enumeration", "WazappyApp.Scenario1" },
    { "Audio Rendering with Hardware Offload", "WazappyApp.Scenario2" },
    // { "Audio Rendering with Low Latency", "Wazappy.Scenario3" },
    // { "PCM Audio Capture", "Wazappy.Scenario4" }
};
