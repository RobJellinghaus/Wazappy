// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "Scenario2.xaml.h"
#include "WazappyClient.h"
#include <ppl.h>

using namespace concurrency;

using namespace Wazappy;
using namespace WazappyApp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Media;
using namespace Windows::Storage;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;


Scenario2::Scenario2() :
	m_IsMFLoaded(false),
	m_ContentStream(nullptr),
	m_StateChangedEvent(nullptr),
	m_renderer(),
	m_ContentType(ContentType::ContentType_Tone)
{
	InitializeComponent();

	// Initialize MF
	HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
	if (SUCCEEDED(hr))
	{
		m_IsMFLoaded = true;
	}
	else
	{
		ThrowIfFailed(hr);
	}

	m_CoreDispatcher = CoreWindow::GetForCurrentThread()->Dispatcher;

	// Get a string representing the Default Audio Render Device
	String^ deviceId = Windows::Media::Devices::MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default);

	auto PropertiesToRetrieve = ref new Platform::Collections::Vector<String^>();
	PropertiesToRetrieve->Append("System.Devices.AudioDevice.RawProcessingSupported");
	// read property store to see if the device supports a RAW processing mode
	create_task(Windows::Devices::Enumeration::DeviceInformation::CreateFromIdAsync(deviceId, PropertiesToRetrieve)).then([this](Windows::Devices::Enumeration::DeviceInformation^ deviceInformation)
	{
		auto obj = deviceInformation->Properties->Lookup("System.Devices.AudioDevice.RawProcessingSupported");
		this->m_deviceSupportsRawMode = false;
		if (obj)
		{
			this->m_deviceSupportsRawMode = obj->Equals(true);
		}

		if (this->m_deviceSupportsRawMode)
		{
			this->toggleRawAudio->IsEnabled = true;
			ShowStatusMessage("Raw Supported", NotifyType::StatusMessage);
		}
		else
		{
			this->toggleRawAudio->IsEnabled = false;
			ShowStatusMessage("Raw Not Supported", NotifyType::StatusMessage);
		}
	});

	// Register for Media Transport controls.  This is required to support background
	// audio scenarios.
	m_SystemMediaControls = SystemMediaTransportControls::GetForCurrentView();
	m_SystemMediaControlsButtonToken = m_SystemMediaControls->ButtonPressed += ref new TypedEventHandler<SystemMediaTransportControls^, SystemMediaTransportControlsButtonPressedEventArgs^>(this, &Scenario2::MediaButtonPressed);
	m_SystemMediaControls->IsPlayEnabled = true;
	m_SystemMediaControls->IsPauseEnabled = true;
	m_SystemMediaControls->IsStopEnabled = true;
}

Scenario2::~Scenario2()
{
	if (m_deviceStateChangeToken.Value != 0)
	{
		m_StateChangedEvent->StateChangedEvent -= m_deviceStateChangeToken;
		m_StateChangedEvent = nullptr;
		m_deviceStateChangeToken.Value = 0;
	}

	if (m_SystemMediaControls)
	{
		m_SystemMediaControls->ButtonPressed -= m_SystemMediaControlsButtonToken;
		m_SystemMediaControls->IsPlayEnabled = false;
		m_SystemMediaControls->IsPauseEnabled = false;
		m_SystemMediaControls->IsStopEnabled = false;
		m_SystemMediaControls->PlaybackStatus = MediaPlaybackStatus::Closed;
	}

	// Uninitialize MF
	if (m_IsMFLoaded)
	{
		MFShutdown();
		m_IsMFLoaded = false;
	}
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void Scenario2::OnNavigatedTo(NavigationEventArgs^ e)
{
	// A pointer back to the main page.  This is needed if you want to call methods in MainPage such
	// as NotifyUser()
	rootPage = MainPage::Current;
	UpdateContentUI(false);
}

/// <summary>
/// Invoked when about to leave this page.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void Scenario2::OnNavigatedFrom(NavigationEventArgs^ e)
{
	if (nullptr != m_StateChangedEvent)
	{
		DeviceState deviceState = m_StateChangedEvent->GetState();

		if (deviceState == DeviceState::Playing)
		{
			StopDevice();
		}
	}
}

#pragma region UI Related Code
void Scenario2::ShowStatusMessage(Platform::String^ str, NotifyType messageType)
{
	m_CoreDispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler(
		[this, str, messageType]()
	{
		rootPage->NotifyUser(str, messageType);
	}));
}

// Updates content controls based on selected option
void Scenario2::UpdateContentUI(Platform::Boolean disableAll)
{
	if ((nullptr != btnFilePicker) &&
		(nullptr != sliderFrequency))
	{
		if (disableAll)
		{
			btnFilePicker->IsEnabled = false;
			sliderFrequency->IsEnabled = false;
			radioFile->IsEnabled = false;
			radioTone->IsEnabled = false;
		}
		else
		{
			radioFile->IsEnabled = true;
			radioTone->IsEnabled = true;

			switch (m_ContentType)
			{
			case ContentType::ContentType_Tone:
				btnFilePicker->IsEnabled = false;
				sliderFrequency->IsEnabled = true;
				UpdateContentProps(sliderFrequency->Value.ToString() + " Hz");
				break;

			case ContentType::ContentType_File:
				btnFilePicker->IsEnabled = true;
				sliderFrequency->IsEnabled = false;
				break;

			default:
				break;
			}
		}
	}
}

// Updates transport controls based on current playstate
void Scenario2::UpdateMediaControlUI(DeviceState deviceState)
{
	switch (deviceState)
	{
	case DeviceState::Playing:
		btnPlay->IsEnabled = false;
		btnStop->IsEnabled = true;
		btnPlayPause->IsEnabled = true;
		btnPause->IsEnabled = true;
		break;

	case DeviceState::Stopped:
	case DeviceState::InError:
		btnPlay->IsEnabled = true;
		btnStop->IsEnabled = false;
		btnPlayPause->IsEnabled = true;
		btnPause->IsEnabled = false;

		UpdateContentUI(false);
		break;

	case DeviceState::Paused:
		btnPlay->IsEnabled = true;
		btnStop->IsEnabled = true;
		btnPlayPause->IsEnabled = true;
		btnPause->IsEnabled = false;
		break;

	case DeviceState::Starting:
	case DeviceState::Stopping:
		btnPlay->IsEnabled = false;
		btnStop->IsEnabled = false;
		btnPlayPause->IsEnabled = false;
		btnPause->IsEnabled = false;

		UpdateContentUI(true);
		break;
	}
}

// Updates textbox on UI thread
void Scenario2::UpdateContentProps(String^ str)
{
	String^ text = str;

	if (nullptr != txtContentProps)
	{
		// The event handler may be invoked on a background thread, so use the Dispatcher to invoke the UI-related code on the UI thread.
		txtContentProps->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler(
			[this, text]()
		{
			Windows::UI::Xaml::Media::SolidColorBrush ^brush;
			txtContentProps->Text = text;

			if (("" == text) && (m_ContentType == ContentType::ContentType_File))
			{
				brush = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(0xCC, 0xFF, 0x00, 0x00));
			}
			else
			{
				brush = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(0xFF, 0xFF, 0xFF, 0xFF));
			}

			txtContentProps->Background = brush;
		}));
	}
}

#pragma endregion

#pragma region UI Event Handlers
void Scenario2::sliderFrequency_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	Slider^ s = safe_cast<Slider^>(sender);
	if (s != nullptr)
	{
		UpdateContentProps(s->Value.ToString() + " Hz");
	}
}

void Scenario2::sliderVolume_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	Slider^ s = safe_cast<Slider^>(sender);
	if (s != nullptr)
	{
		OnSetVolume(static_cast<UINT32>(s->Value));
	}
}

void Scenario2::radioTone_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_ContentType = ContentType::ContentType_Tone;
	UpdateContentProps("");
	m_ContentStream = nullptr;
	UpdateContentUI(false);
}

void Scenario2::radioFile_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_ContentType = ContentType::ContentType_File;
	UpdateContentProps("");
	m_ContentStream = nullptr;
	UpdateContentUI(false);
}

void Scenario2::btnPlay_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	StartDevice();
}

void Scenario2::btnPause_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	PauseDevice();
}

void Scenario2::btnPlayPause_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	PlayPauseToggleDevice();
}

void Scenario2::btnStop_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	StopDevice();
}

void Scenario2::btnFilePicker_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	OnPickFileAsync();
}

#pragma endregion

// Event callback from WASAPI renderer for changes in device state
void Scenario2::OnDeviceStateChange(Object^ sender, DeviceStateChangedEventArgs^ e)
{
	// Update Control Buttons
	m_CoreDispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler(
		[this, e]()
	{
		UpdateMediaControlUI(e->State);
	}));

	// Handle state specific messages
	switch (e->State)
	{
	case DeviceState::Initialized:
		StartDevice();
		m_SystemMediaControls->PlaybackStatus = MediaPlaybackStatus::Closed;
		break;

	case DeviceState::Playing:
		ShowStatusMessage("Playback Started", NotifyType::StatusMessage);
		m_SystemMediaControls->PlaybackStatus = MediaPlaybackStatus::Playing;
		break;

	case DeviceState::Paused:
		ShowStatusMessage("Playback Paused", NotifyType::StatusMessage);
		m_SystemMediaControls->PlaybackStatus = MediaPlaybackStatus::Paused;
		break;

	case DeviceState::Stopped:
		m_renderer = WazappyNodeHandle();

		if (m_deviceStateChangeToken.Value != 0)
		{
			m_StateChangedEvent->StateChangedEvent -= m_deviceStateChangeToken;
			m_StateChangedEvent = nullptr;
			m_deviceStateChangeToken.Value = 0;
		}

		ShowStatusMessage("Playback Stopped", NotifyType::StatusMessage);
		m_SystemMediaControls->PlaybackStatus = MediaPlaybackStatus::Stopped;
		break;

	case DeviceState::InError:
		HRESULT hr = e->hr;

		if (m_deviceStateChangeToken.Value != 0)
		{
			m_StateChangedEvent->StateChangedEvent -= m_deviceStateChangeToken;
			m_StateChangedEvent = nullptr;
			m_deviceStateChangeToken.Value = 0;
		}

		// null out the renderer handle
		m_renderer = WazappyNodeHandle();

		m_SystemMediaControls->PlaybackStatus = MediaPlaybackStatus::Closed;

		wchar_t hrVal[11];
		swprintf_s(hrVal, 11, L"0x%08x\0", hr);
		String^ strHRVal = ref new String(hrVal);

		String^ strMessage = "";

		// Specifically handle a couple of known errors
		switch (hr)
		{
		case AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE:
			strMessage = "ERROR: Endpoint Does Not Support HW Offload (" + strHRVal + ")";
			ShowStatusMessage(strMessage, NotifyType::ErrorMessage);
			break;

		case AUDCLNT_E_RESOURCES_INVALIDATED:
			strMessage = "ERROR: Endpoint Lost Access To Resources (" + strHRVal + ")";
			ShowStatusMessage(strMessage, NotifyType::ErrorMessage);
			break;

		default:
			strMessage = "ERROR: " + strHRVal + " has occurred.";
			ShowStatusMessage(strMessage, NotifyType::ErrorMessage);
		}
	}
}

//
//  OnPickFileAsync()
//
//  File chooser for MF Source playback.  Retrieves a pointer to IRandomAccessStream
//
void Scenario2::OnPickFileAsync()
{
	Pickers::FileOpenPicker^ filePicker = ref new Pickers::FileOpenPicker();
	filePicker->ViewMode = Pickers::PickerViewMode::List;
	filePicker->SuggestedStartLocation = Pickers::PickerLocationId::MusicLibrary;
	filePicker->FileTypeFilter->Append(".wav");
	filePicker->FileTypeFilter->Append(".mp3");
	filePicker->FileTypeFilter->Append(".wma");

	concurrency::create_task(filePicker->PickSingleFileAsync()).then(
		[this](Windows::Storage::StorageFile^ file)
	{
		if (nullptr != file)
		{
			// Open the stream
			concurrency::create_task(file->OpenAsync(FileAccessMode::Read)).then(
				[this, file](IRandomAccessStream^ stream)
			{
				if (stream != nullptr)
				{
					m_ContentStream = stream;
					UpdateContentProps(file->Path);
				}
			});
		}
	});
}

//
//  OnSetVolume()
//
//  Updates the session volume
//
void Scenario2::OnSetVolume(UINT32 volume)
{
	if (m_renderer.IsValid() && WASAPIDeviceInterop::WASAPIDevice_IsInitialized(m_renderer))
	{
		// Updates the Session volume on the AudioClient
		WASAPIDeviceInterop::WASAPIDevice_SetVolumeOnSession(m_renderer, volume);
	}
}

//
//  InitializeDevice()
//
//  Sets up a new instance of the WASAPI renderer
//
void Scenario2::InitializeDevice()
{
	HRESULT hr = S_OK;

	if (!m_renderer.IsValid())
	{
		// Create a new WASAPI instance
		m_renderer = WASAPISessionInterop::WASAPISession_GetDefaultRenderDevice();

		if (!m_renderer.IsValid())
		{
			OnDeviceStateChange(this, ref new DeviceStateChangedEventArgs(DeviceState::InError, E_OUTOFMEMORY));
			return;
		}

		// Get a pointer to the device event interface
		// TODO: make this keep the DeviceStateEvent on this side and wire up the callback functor
		m_StateChangedEvent = ref new DeviceStateChangedEvent();

		if (nullptr == m_StateChangedEvent)
		{
			OnDeviceStateChange(this, ref new DeviceStateChangedEventArgs(DeviceState::InError, E_FAIL));
			return;
		}

		// Register for events
		m_deviceStateChangeToken = m_StateChangedEvent->StateChangedEvent += ref new DeviceStateChangedHandler(this, &Scenario2::OnDeviceStateChange);

		// Configure user based properties
		DEVICEPROPS props;
		int BufferSize = 0;
		swscanf_s(txtHWBuffer->Text->Data(), L"%d", &BufferSize);

		switch (m_ContentType)
		{
		case ContentType::ContentType_Tone:
			props.IsTonePlayback = true;
			props.Frequency = static_cast<DWORD>(sliderFrequency->Value);
			break;

		case ContentType::ContentType_File:
			props.IsTonePlayback = false;
			// props.ContentStream = m_ContentStream;
			break;
		}

		props.IsLowLatency = false;
		props.IsHWOffload = static_cast<Platform::Boolean>(toggleHWOffload->IsOn);
		props.IsBackground = static_cast<Platform::Boolean>(toggleBackgroundAudio->IsOn);
		props.IsRawChosen = static_cast<Platform::Boolean>(toggleRawAudio->IsOn);
		props.IsRawSupported = m_deviceSupportsRawMode;
		props.hnsBufferDuration = static_cast<REFERENCE_TIME>(BufferSize);

		WASAPIRenderDeviceInterop::WASAPIRenderDevice_SetProperties(m_renderer, props);

		// Selects the Default Audio Device
		WASAPIDeviceInterop::WASAPIDevice_InitializeAudioDeviceAsync(m_renderer);

		// Register the callback hook, and then the callback
		WASAPIDeviceInterop::WASAPIDevice_RegisterDeviceStateChangeCallbackHook(
			WazappyClient::DeviceStateChangedEventCallback);
		// Get a callback ID for our state change event
		m_deviceStateChangeCallbackId = WazappyClient::RegisterCallback(m_StateChangedEvent);
		// Register our callback on the render device
		WASAPIDeviceInterop::WASAPIDevice_RegisterDeviceStateChangeCallback(m_renderer, m_deviceStateChangeCallbackId);
	}
}

//
//  StartDevice()
//
void Scenario2::StartDevice()
{
	if (!m_renderer.IsValid())
	{
		// Call from main UI thread
		InitializeDevice();
	}
	else
	{
		// Starts a work item to begin playback, likely in the paused state
		WASAPIRenderDeviceInterop::WASAPIRenderDevice_StartPlaybackAsync(m_renderer);
	}
}

//
//  StopDevice()
//
void Scenario2::StopDevice()
{
	if (m_renderer.IsValid())
	{
		// Set the event to stop playback
		WASAPIRenderDeviceInterop::WASAPIRenderDevice_StopPlaybackAsync(m_renderer);
	}
}

//
//  PauseDevice()
//
void Scenario2::PauseDevice()
{
	if (m_renderer.IsValid())
	{
		DeviceState deviceState = m_StateChangedEvent->GetState();

		if (deviceState == DeviceState::Playing)
		{
			// Starts a work item to pause playback
			WASAPIRenderDeviceInterop::WASAPIRenderDevice_PausePlaybackAsync(m_renderer);
		}
	}
}

//
//  PlayPauseToggleDevice()
//
void Scenario2::PlayPauseToggleDevice()
{
	if (m_renderer.IsValid())
	{
		DeviceState deviceState = m_StateChangedEvent->GetState();

		if (deviceState == DeviceState::Playing)
		{
			// Starts a work item to pause playback
			WASAPIRenderDeviceInterop::WASAPIRenderDevice_PausePlaybackAsync(m_renderer);
		}
		else if (deviceState == DeviceState::Paused)
		{
			// Starts a work item to start playback
			WASAPIRenderDeviceInterop::WASAPIRenderDevice_StartPlaybackAsync(m_renderer);
		}
	}
	else
	{
		StartDevice();
	}
}

//
//   MediaButtonPressed
//
void Scenario2::MediaButtonPressed(SystemMediaTransportControls^ sender, SystemMediaTransportControlsButtonPressedEventArgs^ e)
{
	m_CoreDispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler(
		[this, e]()
	{
		switch (e->Button)
		{
		case SystemMediaTransportControlsButton::Play:
			StartDevice();
			break;

		case SystemMediaTransportControlsButton::Pause:
			PauseDevice();
			break;

		case SystemMediaTransportControlsButton::Stop:
			StopDevice();
			break;

		default:
			break;
		}
	}));
}
