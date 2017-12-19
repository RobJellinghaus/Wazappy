// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "MainPage.xaml.h"
#include "SampleConfiguration.h"

using namespace SDKSample;

Platform::Array<Scenario>^ MainPage::scenariosInner = ref new Platform::Array<Scenario>
{
    { "Device Enumeration", "SDKSample.WASAPIAudio.Scenario1" },
    { "Audio Rendering with Hardware Offload", "SDKSample.WASAPIAudio.Scenario2" },
    { "Audio Rendering with Low Latency", "SDKSample.WASAPIAudio.Scenario3" },
    { "PCM Audio Capture", "SDKSample.WASAPIAudio.Scenario4" }
};
