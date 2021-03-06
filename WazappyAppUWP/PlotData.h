// The Wazappy project implements a WASAPI-based sound engine for Windows UWP and desktop apps.
// https://github.com/RobJellinghaus/Wazappy
// Licensed under the MIT License.
// This file based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

#include "Windows.h"
#include "WazappyDllInterface.h"

#define MILLISECONDS_TO_VISUALIZE 20

namespace WazappyApp
{
	// Class for PlotDataReady events
	public ref class PlotDataReadyEventArgs sealed
	{
	internal:
		PlotDataReadyEventArgs(Platform::Array<int, 1>^ points, UINT32 size) :
			m_PointArray(points),
			m_Size(size)
		{};

		property Platform::Array<int, 1>^ Points
		{
			Platform::Array<int, 1>^ get() { return m_PointArray; }
		};

		property UINT32 Size
		{
			UINT32 get() { return m_Size; }
		};

	private:
		Platform::Array<int, 1>^     m_PointArray;
		UINT32 m_Size;
	};

	// PlotDataReady delegate
	public delegate void PlotDataReadyHandler(Platform::Object^ sender, PlotDataReadyEventArgs^ e);

	// PlotDataReady Event
	public ref class PlotDataReadyEvent sealed
	{
	public:
		PlotDataReadyEvent() {};

	internal:
		static void SendEvent(Object^ obj, Platform::Array<int, 1>^ points, UINT32 size)
		{
			PlotDataReadyEventArgs^ e = ref new PlotDataReadyEventArgs(points, size);
			PlotDataReady(obj, e);
		}

	public:
		static event PlotDataReadyHandler^    PlotDataReady;
	};
}
