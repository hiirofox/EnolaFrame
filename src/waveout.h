#pragma once

#include <stdio.h>
#include <stdint.h>
#include <Windows.h>
#include <initguid.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <assert.h>


class WaveOut
{
private:
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	HRESULT hr;
	IMMDeviceEnumerator* enumerator = NULL;
	IMMDevice* renderer = NULL;
	IAudioClient* renderClient = NULL;
	IAudioRenderClient* renderService = NULL;
	WAVEFORMATEX* format = NULL;

	template <typename T>
	void _FillBuffer(const float* bufferL, const float* bufferR, int numSamples);
	template <>
	void _FillBuffer<float>(const float* bufferL, const float* bufferR, int numSamples);
	template <>
	void _FillBuffer<int16_t>(const float* bufferL, const float* bufferR, int numSamples);

public:
	WaveOut();
	~WaveOut();
	void Init();
	void Start();
	int GetSampleRate();
	void WaveOut::FillBuffer(const float* bufferL, const float* bufferR, int numSamples);
	void Stop();
	void Close();
};