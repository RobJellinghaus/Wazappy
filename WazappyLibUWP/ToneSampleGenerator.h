// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <limits.h>

template<typename T> T Convert(double Value);

namespace Wazappy
{
	class ToneSampleGenerator
	{
	public:
		ToneSampleGenerator();
		~ToneSampleGenerator();

		BOOL IsEOF() { return (m_SampleQueue == nullptr); };
		UINT32 GetBufferLength() { return (m_SampleQueue != nullptr ? m_SampleQueue->BufferSize : 0); };
		void Flush();

		HRESULT GenerateSampleBuffer(DWORD Frequency, UINT32 FramesPerPeriod, WAVEFORMATEX *wfx);
		HRESULT FillSampleBuffer(UINT32 BytesToRead, BYTE *Data);

	private:
		template <typename T>
		void GenerateSineSamples(BYTE *Buffer, size_t BufferLength, DWORD Frequency, WORD ChannelCount, DWORD SamplesPerSecond, double Amplitude, double *InitialTheta);

	private:
		RenderBuffer * m_SampleQueue;
		RenderBuffer **m_SampleQueueTail;
	};
}
