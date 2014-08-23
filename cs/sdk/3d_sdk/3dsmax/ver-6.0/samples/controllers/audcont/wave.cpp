//***************************************************************************
//* WaveForm class.
//* This class handles loading and processing of audio stream data
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*

#include "windows.h"
#include "vfw.h"
#include "wave.h"

#include <stdio.h>

WaveForm::WaveForm()
{
	pSampleData = 0;
	lSampleDataSize = 0;
}

WaveForm::~WaveForm()
{
	FreeSample();
}

BOOL WaveForm::IsEmpty()
{
	return (lSampleDataSize == 0);
}

BOOL WaveForm::InitOpen(LPCTSTR lpszPathName)
{
	PAVISTREAM stream;
	AVISTREAMINFO streamInfo;
	PCMWAVEFORMAT wf;
	long l;

	AVIFileInit();  // Initialize the AVI library

	if (AVIStreamOpenFromFile(&stream, lpszPathName, streamtypeAUDIO, 0, OF_READ, NULL)) {
		AVIFileExit();
		return FALSE;
	}

	if (AVIStreamInfo(stream, &streamInfo, sizeof(streamInfo))) {
		AVIFileExit();
		return FALSE;
	}

	SetNumSamples(streamInfo.dwLength);
	SetBeginTime(AVIStreamStartTime(stream));
	SetEndTime(AVIStreamEndTime(stream));

    SetBeginSample(AVIStreamTimeToSample(stream, GetBeginTime()));
    SetEndSample(AVIStreamTimeToSample(stream, GetEndTime()));

    l = sizeof(wf);
    AVIStreamReadFormat(stream, GetBeginTime(), &wf, &l);
    if (!l) {
		AVIFileExit();
        return FALSE;
	}

    if (wf.wf.wFormatTag != WAVE_FORMAT_PCM) {
		AVIFileExit();
        return FALSE;
	}

	if (!AllocSample((GetEndSample() - GetBeginSample())
			* wf.wf.nChannels * wf.wBitsPerSample / 8)) {
		AVIFileExit();
		return FALSE;
	}

	m_nChannels = wf.wf.nChannels;
	m_nBitsPerSample = wf.wBitsPerSample;
	m_lSamplesPerSec = wf.wf.nSamplesPerSec;

    AVIStreamRead(stream, GetBeginSample(), GetEndSample() - GetBeginSample(), (void*)pSampleData, lSampleDataSize, NULL, &l);
	AVIFileExit();

	long erv = lSampleDataSize;
	if (m_nChannels == 2)
		erv = erv / 2;
	if (m_nBitsPerSample == 16)
		erv = erv / 2;
	
	if (l != erv)
		return FALSE;

	int tMax = -65535;
	int sample;
	long i;

	for (i = 0; i< (GetEndSample() - GetBeginSample()); i++) {
		sample = abs(GetSample(i, kLeftChannel));
		tMax = sample > tMax ? sample : tMax;
	}

	if (m_nChannels == 2) {
		for (i = 0; i< (GetEndSample() - GetBeginSample()); i++) {
			sample = GetSample(i, kRightChannel);
			tMax = sample > tMax ? sample : tMax;
		}
	}

	SetMaxValue(tMax);

	return TRUE;
}

void WaveForm::DumpDebug()
{
	char tbuf[80];
	int sample;
	long i;


	sprintf(tbuf, "Max value: %d\n", GetMaxValue());
	OutputDebugString(tbuf);

	for (i = 0; i< (GetEndSample() - GetBeginSample()); i++) {
		sample = abs(GetSample(i, kLeftChannel));
		sprintf(tbuf, "Sample[%05ld]: %d\n", i, sample);
		OutputDebugString(tbuf);
	}
}

void WaveForm::SetNumSamples(long numSamples)
{
	m_lNumSamples = numSamples;
}

void WaveForm::SetBeginTime(long beginTime)
{
	m_lWaveBeginTime = beginTime;
}

void WaveForm::SetEndTime(long endTime)
{
	m_lWaveEndTime = endTime;
}

void WaveForm::SetBeginSample(long beginSample)
{
	m_lWaveBeginSample = beginSample;
}

void WaveForm::SetMaxValue(int value)
{
	m_nMaxValue = value;
}

void WaveForm::SetEndSample(long endSample)
{
	m_lWaveEndSample = endSample;
}

long WaveForm::GetNumSamples()
{
	return m_lNumSamples;
}

long WaveForm::GetBeginTime()
{
	return m_lWaveBeginTime;
}

long WaveForm::GetEndTime()
{
	return m_lWaveEndTime;
}

long WaveForm::GetBeginSample()
{
	return m_lWaveBeginSample;
}

long WaveForm::GetEndSample()
{
	return m_lWaveEndSample;
}

long WaveForm::GetSamplesPerSec()
{
	return m_lSamplesPerSec;
}

int  WaveForm::GetBitsPerSample()
{
	return m_nBitsPerSample;
}

int  WaveForm::GetNumChannels()
{
	return m_nChannels;
}

int  WaveForm::GetMaxValue()
{
	return m_nMaxValue;
}

BOOL WaveForm::AllocSample(long lSize)
{
	if (pSampleData)
		FreeSample();

	pSampleData = new char[lSize];

	lSampleDataSize = pSampleData ? lSize : 0;

	return pSampleData ? TRUE : FALSE;
}

void WaveForm::FreeSample()
{
	if (pSampleData) {
		delete [] pSampleData;

		pSampleData = 0;
		lSampleDataSize = 0;
	}
}

// Macros to normalize 8 bits sound data around -127 to +128
// No need to normalize 16 bit sound
#define AUDIO_NORM8(x) (x<=0?x+128:x-127)

int WaveForm::GetSample(long sampleNo, WaveForm::Channel channel)
{
	char *sample8;
	short *sample16;
	int sample;
	long lSize;

	if (IsEmpty()) // Do we have a wave file loaded?
		return 0;

	// Get two pointers of different datasize to point to the audio stream.
	sample8 = (char *)pSampleData;
	sample16 = (short *)pSampleData;

	// Test for out of range samples
	// Adjust for two channels and stereo recording
	lSize = GetNumChannels() > 1 ? 2 : 1;
	lSize = lSize * (GetBitsPerSample() == 8 ? 1 : 2);

	if (sampleNo >= (long)lSampleDataSize/lSize || sampleNo < 0)
		return 0;

	if (GetNumChannels() == 2) {
		if (channel == Channel::kLeftChannel)
			sampleNo = sampleNo * 2;
		else if (channel == Channel::kRightChannel)
			sampleNo = sampleNo * 2 + 1;
		else if (channel == Channel::kMixChannels)
			sampleNo = sampleNo * 2; // Get left channel first
	}

	if (GetBitsPerSample() == 8) {
		sample = AUDIO_NORM8(sample8[sampleNo]); // -127 -> +128
		if (channel == Channel::kMixChannels && GetNumChannels() == 2) {
			sample += AUDIO_NORM8(sample8[sampleNo+1]);
			sample = sample /2;
		}
	}
	else {
		sample = sample16[sampleNo];
		if (channel == Channel::kMixChannels && GetNumChannels() == 2) {
			sample += sample16[sampleNo+1];
			sample = sample /2;
		}
	}

	return sample;
}

