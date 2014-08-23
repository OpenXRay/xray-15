//***************************************************************************
//* Audio Amplitude Controller for 3D Studio MAX.
//*	Base class for all "point 3" based controllers
//* 
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*

#include "auctrl.h"
#include "aup3base.h"
#include "aup3dlg.h"


extern HINSTANCE hInstance;
extern BOOL GetSoundFileName(HWND hWnd,TSTR &name,TSTR &dir); // From MAX

AudioP3Control::AudioP3Control() 
{
	pDlg = NULL;
	range = Interval(GetAnimStart(),GetAnimEnd());
	channel = 2;
	absolute = 0;
	min = 0.0f;
	max = 1.0f;
	numsamples = 1;
	threshold = 0.0f;
	enableRuntime = 0;
	quickdraw = 0;
} 

AudioP3Control::~AudioP3Control()
{
}

#define MIN_CHUNK			0x0100
#define MAX_CHUNK			0x0101
#define ABSOLUTE_CHUNK		0x0103
#define FILENAME_CHUNK		0x0104
#define NUMSAMPLES_CHUNK	0x0105
#define CHANNEL_CHUNK		0x0106
#define RANGE_CHUNK			0x0107
#define BASE_CHUNK			0x0108
#define TARGET_CHUNK		0x0109
#define THRESHOLD_CHUNK		0x010A
#define RUNTIME_CHUNK		0x010B
#define QUICKDRAW_CHUNK		0x010C

// Save the controller data
IOResult AudioP3Control::Save(ISave *isave)
{
	ULONG nb;

	Control::Save(isave);	// Handle ORT's

	isave->BeginChunk(BASE_CHUNK);
	isave->Write(&basePoint,sizeof(basePoint),&nb);
	isave->EndChunk();

	isave->BeginChunk(TARGET_CHUNK);
	isave->Write(&targetPoint,sizeof(targetPoint),&nb);
	isave->EndChunk();

	isave->BeginChunk(ABSOLUTE_CHUNK);
	isave->Write(&absolute,sizeof(absolute),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHANNEL_CHUNK);
	isave->Write(&channel,sizeof(channel),&nb);
	isave->EndChunk();

	isave->BeginChunk(FILENAME_CHUNK);
	isave->WriteWString((TCHAR*)szFilename);
	isave->EndChunk();

	isave->BeginChunk(NUMSAMPLES_CHUNK);
	isave->Write(&numsamples,sizeof(numsamples),&nb);
	isave->EndChunk();

	isave->BeginChunk(RANGE_CHUNK);
	isave->Write(&range,sizeof(range),&nb);
	isave->EndChunk();

	isave->BeginChunk(THRESHOLD_CHUNK);
	isave->Write(&threshold,sizeof(threshold),&nb);
	isave->EndChunk();

	isave->BeginChunk(RUNTIME_CHUNK);
	isave->Write(&enableRuntime,sizeof(enableRuntime),&nb);
	isave->EndChunk();

	isave->BeginChunk(QUICKDRAW_CHUNK);
	isave->Write(&quickdraw,sizeof(quickdraw),&nb);
	isave->EndChunk();

	return IO_OK;
}

// Load the controller data
IOResult AudioP3Control::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res = IO_OK;

	Control::Load(iload);	// Handle ORT's

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case BASE_CHUNK:
				res=iload->Read(&basePoint,sizeof(basePoint),&nb);
				break;

			case TARGET_CHUNK:
				res=iload->Read(&targetPoint,sizeof(targetPoint),&nb);
				break;

			case ABSOLUTE_CHUNK:
				res=iload->Read(&absolute,sizeof(absolute),&nb);
				break;

			case CHANNEL_CHUNK:
				res=iload->Read(&channel,sizeof(channel),&nb);
				break;

			case FILENAME_CHUNK: {
				wchar_t *buf = NULL;
				res=iload->ReadWStringChunk(&buf);
				szFilename = buf;
				if (!szFilename.isNull()) {
					if (FixupFilename(szFilename, iload->GetDir(APP_SOUND_DIR))) {
						// Initialize the WaveForm and load the audio stream
						wave->InitOpen(szFilename);
					}
				}
				break;
				}
			case NUMSAMPLES_CHUNK:
				res=iload->Read(&numsamples,sizeof(numsamples),&nb);
				break;

			case RANGE_CHUNK:
				res=iload->Read(&range,sizeof(range),&nb);
				break;

			case THRESHOLD_CHUNK:
				res=iload->Read(&threshold,sizeof(threshold),&nb);
				break;

			case RUNTIME_CHUNK:
				res=iload->Read(&enableRuntime,sizeof(enableRuntime),&nb);
				break;
			case QUICKDRAW_CHUNK:
				res=iload->Read(&quickdraw,sizeof(quickdraw),&nb);
				break;

		}

		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	return IO_OK;
}
