//***************************************************************************
//* RunTimeWave class.
//* This class handles real-time (recording) processing of audio stream data
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* January 1996
//*

#ifndef __RTWAVE__H
#define __RTWAVE__H

class RunTimeWave {
public:
	RunTimeWave();
	~RunTimeWave();

	void SetupRecWindow();
	int  StartRecording();
	void ContinueRecording(WAVEHDR* pHdr);
	void StopRecording();
	void RecordingStopped();
	void SetDevice(int nIndex);
	int GetDevice();
	void SetSample(int sample);
	int GetSample(int oversampling);
	void SetStopFlag(int status);
	int  GetStopFlag();

	BOOL IsRecording();
	WAVEINCAPS* m_pDevCaps;
	WAVEINCAPS *pCurCaps;
	WAVEFORMATEX fmt;

	int m_nNumDevices;
	HWND recWindow;
	HWAVEIN m_hWaveIn;
	int blockSize;

	WAVEHDR* pWaveHdr;
	char *pSampleData;
	DWORD lSampleDataSize;
	int m_nIndex;
	int stopFlag;
};

#endif
