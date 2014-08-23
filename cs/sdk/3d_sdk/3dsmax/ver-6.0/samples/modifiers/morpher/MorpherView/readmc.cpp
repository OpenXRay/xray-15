/*===========================================================================*\
 | 
 |  FILE:	ReadMC.cpp
 |			MorpherView Utility - demonstrates use of MorpherAPI access
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 4-4-99
 | 
\*===========================================================================*/

/*===========================================================================*\
 |	This file shows how to extract data from the morphChannel class
 |	And present it in meaningful form. As well as drive the various MC functions
\*===========================================================================*/


#include "MorpherView.h"



/*===========================================================================*\
 |	Load the channels and their names out of a chosen Morpher Modifier
\*===========================================================================*/

void MorphViewUtil::LoadMorpherInfo( HWND hWnd )
{
	HWND mcList = GetDlgItem(hWnd,IDC_MCLIST);
	SendMessage(mcList,CB_RESETCONTENT,0,0);

	for(int i=0;i<100;i++)
	{
		char s[255];
		sprintf(s,"%i - %s",i+1,mp->chanBank[i].mName);
		SendMessage(mcList,CB_ADDSTRING,0,(LPARAM)s);
	}
	SendMessage(mcList,CB_SETCURSEL ,(WPARAM)0,0);
	LoadChannelInfo(hWnd,0);
}

/*===========================================================================*\
 |	Load the data out of a morph channel and display it
\*===========================================================================*/

void MorphViewUtil::LoadChannelInfo( HWND hWnd, int idx )
{
	// Channel name (mName)
	SetWindowText(GetDlgItem(hWnd,IDC_MNAME),mp->chanBank[idx].mName);

	// The two point counts, nPts and nmPts
	char s[255];
	sprintf(s,"%i",mp->chanBank[idx].mNumPoints);
	SetWindowText(GetDlgItem(hWnd,IDC_NPTS),s);
	sprintf(s,"%i",mp->chanBank[idx].mPoints.size());
	SetWindowText(GetDlgItem(hWnd,IDC_NMPTS),s);

	// The various states of the channels
	SetCheckBox(hWnd,IDC_MMODDED,mp->chanBank[idx].mModded);
	SetCheckBox(hWnd,IDC_MACTIVE,mp->chanBank[idx].mActive);
	SetCheckBox(hWnd,IDC_MINVALID,!mp->chanBank[idx].mInvalid);
	SetCheckBox(hWnd,IDC_MACTIVEOVERRIDE,mp->chanBank[idx].mActiveOverride);
	SetCheckBox(hWnd,IDC_MCONNECTION,(mp->chanBank[idx].mConnection!=NULL));

	// The value of the channel - ie, its weighted percentage
	float tmpVal;
	mp->chanBank[idx].cblock->GetValue(0, ip->GetTime(), tmpVal, FOREVER);
	sprintf(s,"%.1f%%",tmpVal);
	SetWindowText(GetDlgItem(hWnd,IDC_VALUE),s);
}