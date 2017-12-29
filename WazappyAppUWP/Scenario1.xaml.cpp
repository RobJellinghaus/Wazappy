// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "Scenario1.xaml.h"

using namespace Wazappy;
using namespace WazappyApp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Media::Devices;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

Scenario1::Scenario1()
{
	InitializeComponent();
	m_DevicesList = safe_cast<ListBox^>(static_cast<IFrameworkElement^>(this)->FindName("DevicesList"));
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void Scenario1::OnNavigatedTo(NavigationEventArgs^ e)
{
	// A pointer back to the main page.  This is needed if you want to call methods in MainPage such
	// as NotifyUser()
	rootPage = MainPage::Current;
}

void Scenario1::ShowStatusMessage(Platform::String^ str, NotifyType messageType)
{
	rootPage->NotifyUser(str, messageType);
}

void Scenario1::Enumerate_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button^ b = safe_cast<Button^>(sender);
	if (b != nullptr)
	{
		m_DevicesList->Items->Clear();
		EnumerateAudioDevicesAsync();
	}
}

void Scenario1::EnumerateAudioDevicesAsync()
{
	// Get the string identifier of the audio renderer
	String^ AudioSelector = MediaDevice::GetAudioRenderSelector();

	// Add custom properties to the query
	auto PropertyList = ref new Platform::Collections::Vector<String^>();
	PropertyList->Append(PKEY_AudioEndpoint_Supports_EventDriven_Mode);

	// Setup the asynchronous callback
	Concurrency::task<DeviceInformationCollection^> enumOperation(DeviceInformation::FindAllAsync(AudioSelector, PropertyList));
	enumOperation.then([this](DeviceInformationCollection^ DeviceInfoCollection)
	{
		if ((DeviceInfoCollection == nullptr) || (DeviceInfoCollection->Size == 0))
		{
			this->ShowStatusMessage("No Devices Found.", NotifyType::ErrorMessage);
		}
		else
		{
			try
			{
				// Enumerate through the devices and the custom properties
				for (unsigned int i = 0; i < DeviceInfoCollection->Size; i++)
				{
					DeviceInformation^ deviceInfo = DeviceInfoCollection->GetAt(i);
					String^ DeviceInfoString = deviceInfo->Name;

					if (deviceInfo->Properties->Size > 0)
					{
						// Pull out the custom property
						Object^ DevicePropString = deviceInfo->Properties->Lookup(PKEY_AudioEndpoint_Supports_EventDriven_Mode);

						if (nullptr != DevicePropString)
						{
							DeviceInfoString = DeviceInfoString + " --> EventDriven(" + DevicePropString + ")";
						}
					}

					this->m_DevicesList->Items->Append(DeviceInfoString);
				}

				String^ strMsg = "Enumerated " + DeviceInfoCollection->Size + " Device(s).";
				this->ShowStatusMessage(strMsg, NotifyType::StatusMessage);
			}
			catch (Platform::Exception^ e)
			{
				this->ShowStatusMessage(e->Message, NotifyType::ErrorMessage);
			}
		}
	});
}


