//************************************************************************** 
//* SaveVerification.cpp -	A callback mechanism to validate that scene
//*							files are saved correctly.
//*
//* This plug-in works by subscribing to a 'post save' callback, and as soon
//* as a file is saved, the file will be reopened and validated. If the
//* validation fails,it will prompt the user to resave the file.
//* The validation is simply verifying that the file is a valid OLE structured
//* storage file and that the steams required by max is present - as such the
//* format is validated, but not the actual contents.
//* 
//* Christer Janson
//* Discreet, A division of Autodesk, Inc.
//*
//* October 28, 2002, CCJ - Initial coding.
//*
//* Copyright (c) 2002, All Rights Reserved. 
//***************************************************************************

#include "SaveVerification.h"

SaveVerificationGUP::SaveVerificationGUP()
{
}

SaveVerificationGUP::~SaveVerificationGUP()
{
}

DWORD SaveVerificationGUP::Start()
{
	// Register a post save callback that gets called when the user saves the file.
	RegisterNotification(PostSaveCallback, this, NOTIFY_FILE_POST_SAVE);

	return GUPRESULT_KEEP;
}

void SaveVerificationGUP::Stop()
{
	// unregister the callback.
	UnRegisterNotification(PostSaveCallback, this, NOTIFY_FILE_POST_SAVE);
}

DWORD SaveVerificationGUP::Control(DWORD parameter)
{
	return 0;
}

void SaveVerificationGUP::DeleteThis()
{
	delete this;
}

// This method gets called when the user saves a file.
void SaveVerificationGUP::PostSaveCallback(void* param, NotifyInfo* info)
{
	SaveVerificationGUP* pThis = (SaveVerificationGUP*)param;
	pThis->ValidateFile((TCHAR*)info->callParam);
}	

void SaveVerificationGUP::ValidateFile(TSTR filename)
{
	if (filename.isNull()) filename = Max()->GetCurFilePath();
	if (!filename.isNull())
	{
		// Validate that the file is a valid STG file and that the proper streams are present.
		if (!ValidateStreams(filename))
		{
			TSTR str = TSTR("File ") + filename + TSTR(" is possibly corrupt.\nWould you like to try to resave?");

			// The file was possibly corrupt! Prompt the user to resave.
			if (MessageBox(MaxWnd(), str, "Attention", MB_YESNO | MB_ICONERROR) == IDYES)
			{
				Max()->FileSaveAs();
			}
		}
	}
}

bool SaveVerificationGUP::ValidateStreams(TSTR filename)
{
	bool		validFile = true;
	HRESULT		res;
	IStorage*	pIStorage = NULL;

	// This opens the file as an OLE structured storage file.
	res = StgOpenStorage (WStr(filename), 0, STGM_DIRECT|STGM_READ|STGM_SHARE_DENY_WRITE, NULL, 0, &pIStorage);
	if (res == S_OK)
	{
		// Verify that the Scene stream is present.
		if (!HasStream(pIStorage, "Scene"))
			validFile = false;

		// Verify that the Config stream is present.
		if (!HasStream(pIStorage, "Config"))
			validFile = false;

		// Verify that the ClassData stream is present.
		if (!HasStream(pIStorage, "ClassData"))
			validFile = false;

		// Verify that the DllDirectory stream is present.
		if (!HasStream(pIStorage, "DllDirectory"))
			validFile = false;

		// It's twelve o'clock and all is well...
		pIStorage->Release();
		pIStorage = NULL;
	}
	else
	{
		validFile = false;
	}

	return validFile;
}

bool SaveVerificationGUP::HasStream(IStorage* storage, TSTR stream)
{
	bool		hasStream;
	IStream*	pIStream = NULL;

	// Verify that the given stream is present.
	HRESULT res = storage->OpenStream (WStr(stream), NULL,  STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pIStream);
	if (res == S_OK)  
	{
		pIStream->Release();
		pIStream = NULL;
		hasStream = true;
	}
	else {
		hasStream = false;
	}

	return hasStream;
}
