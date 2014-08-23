//***************************************************************************
//* WaveForm class.
//* This class handles loading and processing of audio stream data
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995

#ifndef __WAVE__H
#define __WAVE__H

class WaveForm {
public:
	WaveForm();
	~WaveForm();

	enum Channel {	kLeftChannel  = 0,
					kRightChannel = 1,
					kMixChannels = 2 };

	// Initialize, open and read the sample data
	BOOL InitOpen(LPCTSTR lpszPathName);

	// Delete the waveform and free allocated memory
	void FreeSample();

	BOOL IsEmpty();
	int GetSample(long sampleNo, WaveForm::Channel channel);

	long GetNumSamples();	// Number of samples
	long GetBeginTime();	// Start time in milliseconds
	long GetEndTime();		// End time in milliseconds
	long GetBeginSample();	// First sample
	long GetEndSample();	// Last sample
	long GetSamplesPerSec();// Samples per second
	int  GetBitsPerSample();
	int  GetNumChannels();
	int  GetMaxValue();

	void DumpDebug();		// Debug Stuff


private:
	void SetNumSamples(long numSamples);
	void SetBeginTime(long startTime);
	void SetEndTime(long endTime);
	void SetBeginSample(long beginSample);
	void SetEndSample(long endSample);
	void SetMaxValue(int value);

	BOOL AllocSample(long lSize);

private:
	// DWORD pSampleData;
	char *pSampleData;
	DWORD lSampleDataSize;
	DWORD m_lSamplesPerSec;
	long m_lNumSamples;
    long m_lWaveBeginTime;
    long m_lWaveEndTime;
    long m_lWaveBeginSample;
    long m_lWaveEndSample;

	int m_nChannels;
	int m_nBitsPerSample;
	int m_nMaxValue;
};

#endif
