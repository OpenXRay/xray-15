/**********************************************************************
 *<
	FILE:  sound.h

	DESCRIPTION:  Some sound related functions

	CREATED BY:  Rolf Berteig -- based on examples fro VFW SDK

	HISTORY: created 2 July 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __SOUND_H__
#define __SOUND_H__

class FilteredWave {
	private:
		DWORD *sat;
		DWORD satLen, max[2];
		int channels, numSum;
		TimeValue start, end;

	public:
		FilteredWave(PAVISTREAM pavi,int resolution);
		~FilteredWave();

		int Channels() {return channels;}
		DWORD Sample(TimeValue t0,TimeValue t1,int channel=0);  // range is 0 - 0x8000
		DWORD Max(int channel) {return max[channel];}
		TimeValue Start() {return start;}		
		TimeValue End() {return end;}
	};



CoreExport BOOL GetSoundFileName(HWND hWnd,TSTR &name,TSTR &dir);

PAVISTREAM GetAudioStream(TSTR name,TCHAR *dir);

BOOL OpenSoundFile(HWND hWnd,PAVISTREAM *pavi);

#endif // __SOUND_H__

