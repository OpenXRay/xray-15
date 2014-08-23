//***************************************************************************
//* RunTimeWave class.
//* This class handles real-time (recording) processing of audio stream data
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* January 1996
//*

#include "auctrl.h"
#include "windows.h"
#include "vfw.h"
#include "rtwave.h"

extern HINSTANCE hInstance;
#define AUCTRL_DUMMY_WINDOW "AUCTRLDUMMYWINDOW"

RunTimeWave::RunTimeWave()
{
	m_pDevCaps = NULL;
	pCurCaps = NULL;
	m_hWaveIn = NULL;
	blockSize = 512;
 	pSampleData = new char[blockSize];
    fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.wBitsPerSample = 8;
    fmt.nChannels = 1;
    fmt.nSamplesPerSec = 11025;
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec;
    fmt.nBlockAlign = 1;

	m_nNumDevices = waveInGetNumDevs();
	if (m_nNumDevices > 0) {
		m_pDevCaps = new WAVEINCAPS[m_nNumDevices];
		for (int i = 0; i< m_nNumDevices; i++) {
			waveInGetDevCaps(i, &m_pDevCaps[i], sizeof(WAVEINCAPS));
			m_pDevCaps[i].wMid = i;
		}
	}

	SetupRecWindow();

	recWindow = CreateWindow(
		_T(AUCTRL_DUMMY_WINDOW), NULL,
		//WS_POPUP | WS_VISIBLE,
		WS_POPUP,
		//0,0,320,200, 
		0,0,0,0,
		NULL,
		NULL,
		hInstance,
		NULL );
}

RunTimeWave::~RunTimeWave()
{
	if (IsRecording()) {
		StopRecording();
		RecordingStopped();
	}

	if (recWindow) DestroyWindow(recWindow);	

	delete [] pSampleData;

	if (m_pDevCaps)
		delete [] m_pDevCaps;

}

static LRESULT CALLBACK RecWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	WAVEHDR* pWave;
	RunTimeWave* rtw;

	switch (msg) {
		case WIM_DATA:
			pWave = (WAVEHDR*)lParam;
			rtw = (RunTimeWave *)pWave->dwUser;
			if (rtw) {
				memcpy(rtw->pSampleData, pWave->lpData, rtw->blockSize);
				if (!rtw->GetStopFlag()) {
					rtw->ContinueRecording(pWave);
				}
				else {
					rtw->RecordingStopped();
				}
			}
			break;
		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
	}
	return 0;
}

void RunTimeWave::SetupRecWindow()
{
	WNDCLASS  wc;

	wc.style         = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.lpfnWndProc   = RecWndProc;
    wc.lpszClassName = _T(AUCTRL_DUMMY_WINDOW);

	RegisterClass(&wc);	
}

int RunTimeWave::StartRecording()
{
	SetStopFlag(FALSE);

	MMRESULT mmr;
    mmr = waveInOpen(&m_hWaveIn,
                     pCurCaps->wMid,
                     &fmt, 
                     (DWORD_PTR)(recWindow), 
                     0, 
                     CALLBACK_WINDOW);
    if (mmr != 0) {
		return mmr;
	}


    WAVEHDR* phdr = (WAVEHDR*)malloc(sizeof(WAVEHDR));
	pWaveHdr = phdr;
    // fill out the wave header
    memset(phdr, 0, sizeof(WAVEHDR));
    phdr->lpData = (char *)malloc(blockSize);
    phdr->dwBufferLength = blockSize;
    phdr->dwUser = (DWORD_PTR)(void*)this;  // so we can find the object 

    // Prepare the header
    mmr = waveInPrepareHeader(m_hWaveIn, phdr, sizeof(WAVEHDR));
    if (mmr != 0) {
        return mmr;
    }

    // Send it to the driver
    mmr = waveInAddBuffer(m_hWaveIn, phdr, sizeof(WAVEHDR));
    if (mmr != 0) {
        return mmr;
    }

    // Start the recording
    mmr = waveInStart(m_hWaveIn);
    if (mmr != 0) {
        return mmr;
    }

	return 0;
}

void RunTimeWave::ContinueRecording(WAVEHDR* pHdr)
{
    // Send another block to the driver
    // Allocate a header
    WAVEHDR* phdrNew = (WAVEHDR*)malloc(sizeof(WAVEHDR));
	pWaveHdr = phdrNew;

    // fill out the wave header
    memset(phdrNew, 0, sizeof(WAVEHDR));
    phdrNew->lpData = (char *)malloc(blockSize);
    phdrNew->dwBufferLength = blockSize;
    phdrNew->dwUser = (DWORD_PTR)(void*)this;  // so we can find the object 

    // Prepare the header
    MMRESULT mmr = waveInPrepareHeader(m_hWaveIn, phdrNew, sizeof(WAVEHDR));
    if (mmr != 0) return ;

    // Send it to the driver
    mmr = waveInAddBuffer(m_hWaveIn, phdrNew, sizeof(WAVEHDR));
    if (mmr != 0) return ;

    // Now handle the block that was completed
    // Unprepare the header
    mmr = waveInUnprepareHeader(m_hWaveIn, pHdr, sizeof(WAVEHDR));
    if (mmr != 0) return ;

    // free the previous block + header
	free(pHdr->lpData);
    free(pHdr);
}


void RunTimeWave::StopRecording()
{
	SetStopFlag(TRUE);
}

void RunTimeWave::RecordingStopped()
{
	waveInReset(m_hWaveIn);
	waveInClose(m_hWaveIn);
	free(pWaveHdr->lpData);
	free(pWaveHdr);
	m_hWaveIn = NULL;
}

void RunTimeWave::SetDevice(int nIndex)
{
	pCurCaps = &m_pDevCaps[nIndex];
	m_nIndex = nIndex;
}

int RunTimeWave::GetDevice()
{
	return m_nIndex;
}

BOOL RunTimeWave::IsRecording()
{
	return (m_hWaveIn && !GetStopFlag()) ? TRUE : FALSE;
}

void RunTimeWave::SetSample(int sample)
{
	//m_nSample = sample;
}

// Macros to normalize 8 bits sound data around -127 to +128
#define AUDIO_NORM8(x) (x<=0?x+128:x-127)

int RunTimeWave::GetSample(int oversampling)
{
	int sample = 0;
	int i;

	if (oversampling > blockSize)
		oversampling = blockSize;

	for (i=0; i < oversampling; i++)
		sample += abs(AUDIO_NORM8(pSampleData[i]));

	if (i != 0)
		sample = sample / i;

	return sample;
}

void RunTimeWave::SetStopFlag(int status)
{
	stopFlag = status;
}

int RunTimeWave::GetStopFlag()
{
	return stopFlag;
}
