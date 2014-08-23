/**********************************************************************
 *<
	FILE:  sound.cpp

	DESCRIPTION:  Some sound related functions

	CREATED BY:  Rolf Berteig -- based on examples fro VFW SDK

	HISTORY: created 2 July 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "core.h"

#ifdef WIN95STUFF

#include <mmsystem.h>
#include <vfw.h>
#include "audio.h"
#include "muldived.h"
#include "sound.h"

#define GlobalSizePtr(lp)   GlobalSize(GlobalPtrHandle(lp))
typedef BYTE  *HPBYTE;
typedef SHORT  *HPINT;

static char fileName[256] = {'\0'};


FilteredWave::FilteredWave(PAVISTREAM pavi,int resolution)
	{
#ifndef INTERIM_64_BIT	// CCJ
	PCMWAVEFORMAT wf;
	LPVOID lpAudio=NULL;
	LONG l, lBytes;	
	DWORD *ptr, sum[2], pos, v;

	LONG s   = AVIStreamStartTime(pavi);
	LONG e   = AVIStreamEndTime(pavi);
	LONG len = e-s;

	LONG sstart = AVIStreamTimeToSample(pavi,s);
	LONG send   = AVIStreamTimeToSample(pavi,e);
	LONG slen   = send-sstart;

	HPBYTE bptr;
	HPINT iptr;
	int in, b, av[2];
	int j,k;
	DWORD i;

	sat    = NULL;
	satLen = 0;
	start  = (s*TIME_TICKSPERSEC)/1000;
	end    = (e*TIME_TICKSPERSEC)/1000;

	l = sizeof(wf);
    AVIStreamReadFormat(pavi,0,&wf,&l);
    if (!l)	{
        return;
		}
	if (wf.wf.wFormatTag != WAVE_FORMAT_PCM) {
		return;
		}

	lBytes = slen * wf.wf.nChannels * wf.wBitsPerSample/8;    
	lpAudio = GlobalAllocPtr(GHND,lBytes);    
    if (!lpAudio) {
    	return;
		}

    AVIStreamRead(pavi,sstart,slen,lpAudio,lBytes,NULL,&l);
    if (l != slen) {
        GlobalFreePtr(lpAudio);
        return;
		}
    
	satLen = (len*resolution)/(1000);
	numSum = slen/satLen;
	satLen = slen/numSum;

	sat = new DWORD[satLen * wf.wf.nChannels];
	if (!sat) {
		GlobalFreePtr(lpAudio);
		return;
		}
	channels = wf.wf.nChannels;
	ptr = sat;	
	pos = 0;
	
	//
	// First find the average value
	//
	av[0] = av[1] = 0;
	iptr = (HPINT)lpAudio;
	bptr = (HPBYTE)lpAudio;	

	for (i=0; i<(DWORD)slen; i++) {		
		if (wf.wBitsPerSample==8) {
			for (j=0;j<channels;j++) {
				av[j] += *bptr++ - 0x80;
				}	
		} else {
			for (j=0;j<channels;j++) {
				av[j] += *iptr++;				
				}
			}			
		}
	for (j=0;j<channels;j++) {
		av[j] /= slen;
		}
	
	// 
	// Now build the SAT moving the average to 0.
	//
	sum[0] = sum[1] = 0;
	iptr = (HPINT)lpAudio;
	bptr = (HPBYTE)lpAudio;		

	for (i=0; i<satLen; i++) {
		for (k=0; k<numSum; k++,pos++) {
			if (wf.wBitsPerSample==8) {
				for (j=0;j<channels;j++) {
					b = *bptr++  - av[j];
					if (b > 0x80) {
						v = (b - 0x80) * 256;						
					} else {
						v = (0x80 - b) * 256;						
						}					
					sum[j] += v;
					}	
			} else {
				for (j=0;j<channels;j++) {
					in = *iptr++ - av[j];
					if (in<0) in = -in;					
					sum[j] += in;					
					}
				}			
			}

		for (j=0;j<channels;j++) {
			*ptr++ = sum[j];
			}
		}
	
	// Find the max
	max[0] = max[1] = 0;
	for (i=1; i<satLen; i++) {
		for (j=0;j<channels;j++) {
			v = (sat[i*channels+j]-sat[(i-1)*channels+j])/numSum;
			if (v>max[j]) max[j] = v;
			}
		}

	GlobalFreePtr(lpAudio);	
#endif	// INTERIM_64_BIT
	}

FilteredWave::~FilteredWave()
	{
	if (sat) delete sat;
	sat = NULL;
	}

DWORD FilteredWave::Sample(TimeValue t0,TimeValue t1,int channel)
	{
	if (!sat || channel>=channels) return 0;
	int s,e,den;

	if (end<=start) return 0;
	if (t1-t0 > end-start) {
		t0 = start;
		t1 = end;
		}
	if (t0<start) {
		int d = (end-start) * ((start-t0)/(end-start) + 1);
		t0 += d;
		t1 += d; 
		}
	t0 = t0 % (end-start);
	t1 = t1 % (end-start);
	if (t1<t0) t1 = end-1;

	//s = ((t0-start) * satLen)/(end-start);
	//e = ((t1-start) * satLen)/(end-start);
	s = MulDiv32((t0-start), satLen, (end-start));
	e = MulDiv32((t1-start), satLen, (end-start));

	if (s==e) {
		if (e) s = e-1;
		else e = s+1;
		if (s<0 || e>=(int)satLen) return 0;
		}
	assert(s>=0);
	assert(e<(int)satLen);

	den = numSum * (e-s);
	if (!den) return 0;
	else return (sat[e*channels+channel]-sat[s*channels+channel])/den;
	}


BOOL GetSoundFileName(HWND hWnd,TSTR &name,TSTR &dir)
	{
#ifndef INTERIM_64_BIT	// CCJ
	OPENFILENAME ofn;	
	char filter[256];

	AVIBuildFilter(filter,sizeof(filter),FALSE);

	ofn.lStructSize       = sizeof(OPENFILENAME);
	ofn.hwndOwner         = hWnd;
	ofn.hInstance         = NULL;	
	ofn.lpstrTitle        = GetResString(IDS_RB_OPENSOUND);
	ofn.lpstrFilter       = filter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = fileName;
	ofn.nMaxFile          = sizeof(fileName);
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = 0;
	ofn.lpstrInitialDir   = dir;
	ofn.Flags             = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = NULL;
	ofn.lCustData         = 0;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;

	if (GetOpenFileNamePreview(&ofn)) {
		name = fileName;
		SplitFilename(name,&dir,NULL,NULL);
		return TRUE;
	} else {
		return FALSE;
		}
#else	// INTERIM_64_BIT
	return FALSE;
#endif	// INTERIM_64_BIT
	}

PAVISTREAM GetAudioStream(TSTR name,TCHAR *dir)
	{
	HRESULT		hr;
	PAVIFILE	pfile;
	PAVISTREAM	pstream = NULL;
	BOOL res = TRUE;

#ifndef INTERIM_64_BIT	// CCJ

	// RB 5/10/99: Reworked this a bit. Added the current scene dir as a possible search location.
	// Also now using SplitPathFile() instead of doing it by hand.

 	hr = AVIFileOpen(&pfile,name,OF_READ,NULL);
	if (hr) {
		TSTR fileName, tryName;
		SplitPathFile(name, NULL, &fileName);

		// Try the given directory (which is the sound dir)
		tryName = TSTR(dir) + TSTR(_T("\\")) + fileName;
		hr = AVIFileOpen(&pfile,tryName,OF_READ,NULL);
		
		if (hr) {
			// Try the scene directory
			TSTR sceneName = GetCOREInterface()->GetCurFilePath();
			TSTR scenePath;
			SplitPathFile(sceneName, &scenePath, NULL);
			tryName = scenePath + TSTR(_T("\\")) + fileName;
			hr = AVIFileOpen(&pfile,tryName,OF_READ,NULL);
			}

#if 0
		// Try the file in the given directory
		int i = name.Length()-1;
		while (i>0) {
			if (name[i]=='\\' ||
				name[i]==':' ||
				name[i]=='/') {
				i++;
				break;
				}	
			i--;
			}
		if (name.Length()-i>0) {
			TSTR newname = TSTR(dir) + TSTR(_T("\\")) + name.Substr(i,name.Length()-i);
			hr = AVIFileOpen(&pfile,newname,OF_READ,NULL);
			}
#endif
		}
	if (hr) return NULL;

	AVIFileGetStream(pfile,&pstream,streamtypeAUDIO,0);
	AVIFileRelease(pfile);

	if (!pstream) return NULL;

	// Verify it's PCM
	PCMWAVEFORMAT wf;
	LONG l = sizeof(wf);
    AVIStreamReadFormat(pstream,0,&wf,&l);
    if (!l)	{
        AVIStreamRelease(pstream);
		return NULL;
		}
	if (wf.wf.wFormatTag != WAVE_FORMAT_PCM) {
		AVIStreamRelease(pstream);
		return NULL;
		}

#endif	// INTERIM_64_BIT
	return pstream;
	}

BOOL OpenSoundFile(HWND hWnd,PAVISTREAM *pavi)
	{
#ifndef INTERIM_64_BIT	// CCJ
	OPENFILENAME ofn;
	char filter[256];

	AVIBuildFilter(filter,sizeof(filter),FALSE);

	ofn.lStructSize       = sizeof(OPENFILENAME);
	ofn.hwndOwner         = hWnd;
	ofn.hInstance         = NULL;	
	ofn.lpstrTitle        = GetResString(IDS_RB_OPENSOUND);
	ofn.lpstrFilter       = filter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = fileName;
	ofn.nMaxFile          = sizeof(fileName);
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = 0;
	ofn.lpstrInitialDir   = NULL;
	ofn.Flags             = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = NULL;
	ofn.lCustData         = 0;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;

	if (GetOpenFileNamePreview(&ofn)) {
		HRESULT		hr;
    	PAVIFILE	pfile;
		PAVISTREAM	pstream;
		BOOL res = TRUE;

   	 	hr = AVIFileOpen(&pfile,fileName,OF_READ,NULL);
    	if (hr) return FALSE;

		if (AVIFileGetStream(
				pfile,&pstream,streamtypeAUDIO,0) != AVIERR_OK) {			
			res = FALSE;
			goto done;
			}
		
		*pavi = pstream;

	done:
		AVIFileRelease(pfile);
		return res;
	} else {
		return FALSE;
		}
#else	// INTERIM_64_BIT
	return FALSE;
#endif	// INTERIM_64_BIT
	}
	    



/////////////////////////////////////////////////////////////////////
// Micorsoft's stuff....
//
//

/*----------------------------------------------------------------------------*\
|    PaintAudio()								|
|    Draw some samples of audio inside the given rectangle			|
\*----------------------------------------------------------------------------*/

void PaintAudio(
		HDC hdc, PRECT prc, PAVISTREAM pavi, LONG lStart, LONG lLen)
	{
#ifndef INTERIM_64_BIT	// CCJ
    LPVOID lpAudio=NULL;
    PCMWAVEFORMAT wf;
    int i;
    int x,y;
    int w,h;
    BYTE b;
    HBRUSH hbr;
    RECT rc = *prc;
    LONG    lBytes;
    LONG    l, lLenOrig = lLen;
    LONG    lWaveBeginTime = AVIStreamStartTime(pavi);
    LONG    lWaveEndTime   = AVIStreamEndTime(pavi);

    //
    // We can't draw before the beginning of the stream - adjust
    //
    if (lStart < lWaveBeginTime) {
		lLen -= lWaveBeginTime - lStart;
		lStart = lWaveBeginTime;
		// right justify the legal samples in the rectangle - don't stretch
		rc.left = rc.right - (int)muldiv32(rc.right - rc.left, lLen, lLenOrig);
	    }

    //
    // We can't draw past the end of the stream
    //
    if (lStart + lLen > lWaveEndTime) {
		lLenOrig = lLen;
		lLen = max(0, lWaveEndTime - lStart);	// maybe nothing to draw!
		// left justify the legal samples in the rectangle - don't stretch
		rc.right = rc.left + (int)muldiv32(rc.right - rc.left, lLen, lLenOrig);
	    }

    // Now start working with samples, not time
    l = lStart;
    lStart = AVIStreamTimeToSample(pavi, lStart);
    lLen   = AVIStreamTimeToSample(pavi, l + lLen) - lStart;

    //
    // Get the format of the wave data
    //
    l = sizeof(wf);
    AVIStreamReadFormat(pavi, lStart, &wf, &l);
    if (!l)
        return;

    w = rc.right - rc.left;
    h = rc.bottom - rc.top;

    //
    // We were starting before the beginning or continuing past the end.
    // We're not painting in the whole original rect --- use a dark background
    //
    if (rc.left > prc->left) {
        SelectObject(hdc, GetStockObject(DKGRAY_BRUSH));
		PatBlt(hdc, prc->left, rc.top, rc.left - prc->left,
						rc.bottom - rc.top, PATCOPY);
    	}
    if (rc.right < prc->right) {
        SelectObject(hdc, GetStockObject(DKGRAY_BRUSH));
		PatBlt(hdc, rc.right, rc.top, prc->right - rc.right,
						rc.bottom - rc.top, PATCOPY);
    	}

#define BACKBRUSH  (GetSysColor(COLOR_BTNFACE))		// background
#define MONOBRUSH  (GetSysColor(COLOR_BTNSHADOW))	// for mono audio
#define LEFTBRUSH  (RGB(0,0,255))			// left channel
#define RIGHTBRUSH (RGB(0,255,0))			// right channel
#define HPOSBRUSH  (RGB(255,0,0))			// current position
    
    //
    // Paint the background
    //
    hbr = (HBRUSH)SelectObject(hdc, CreateSolidBrush(BACKBRUSH));
    PatBlt(hdc, rc.left, rc.top, w, h, PATCOPY);
    DeleteObject(SelectObject(hdc, hbr));

    //
    // !!! we can only paint PCM data right now.  Sorry!
    //
    if (wf.wf.wFormatTag != WAVE_FORMAT_PCM)
        return;

    //
    // How many bytes are we painting? Alloc some space for them
    //
    lBytes = lLen * wf.wf.nChannels * wf.wBitsPerSample / 8;
    if (!lpAudio)
        lpAudio = GlobalAllocPtr (GHND, lBytes);
    else if ((LONG)GlobalSizePtr(lpAudio) < lBytes)
        lpAudio = GlobalReAllocPtr(lpAudio, lBytes, GMEM_MOVEABLE);
    if (!lpAudio)
        return;

    //
    // Read in the wave data
    //
    AVIStreamRead(pavi, lStart, lLen, lpAudio, lBytes, NULL, &l);
    if (l != lLen)
        return;
    
#define MulDiv(a,b,c) (UINT)((DWORD)(UINT)(a) * (DWORD)(UINT)(b) / (UINT)(c))

    //
    // !!! Flickers less painting it NOW or LATER?
    // First show the current position as a bar
    //
    //hbr = (HBRUSH)SelectObject(hdc, CreateSolidBrush(HPOSBRUSH));
    //PatBlt(hdc, prc->right / 2, prc->top, 1, prc->bottom - prc->top, PATCOPY);
    //DeleteObject(SelectObject(hdc, hbr));

    //
    // Paint monochrome wave data
    //
    if (wf.wf.nChannels == 1) {

		//
		// Draw the x-axis
		//
        hbr = (HBRUSH)SelectObject(hdc, CreateSolidBrush(MONOBRUSH));
        y = rc.top + h/2;
        PatBlt(hdc, rc.left, y, w, 1, PATCOPY);
    
		//
		// 8 bit data is centred around 0x80
		//
        if (wf.wBitsPerSample == 8) {
            for (x=0; x<w; x++) {

				// which byte of audio data belongs at this pixel?
                b = *((HPBYTE)lpAudio + muldiv32(x, lLen, w));

                if (b > 0x80) {
                    i = y - MulDiv(b - 0x80, (h / 2), 128);
                    PatBlt(hdc, rc.left+x, i, 1, y-i, PATCOPY);
                	}
                else {
                    i = y + MulDiv(0x80 - b, (h / 2), 128);
                    PatBlt(hdc, rc.left + x, y, 1, i - y, PATCOPY);
                	}
            	}
        	}

		//
		// 16 bit data is centred around 0x00
		//
        else if (wf.wBitsPerSample == 16) {
            for (x=0; x<w; x++) {

				// which byte of audio data belongs at this pixel?
	            i = *((HPINT)lpAudio + muldiv32(x,lLen,w));

	            if (i > 0) {
	               i = y - (int) ((LONG)i * (h/2) / 32768);
	               PatBlt(hdc, rc.left+x, i, 1, y-i, PATCOPY);
	            	}
	            else {
	               i = (int) ((LONG)i * (h/2) / 32768);
	               PatBlt(hdc, rc.left+x, y, 1, -i, PATCOPY);
	            	}
	            }
        	}
        DeleteObject(SelectObject(hdc, hbr));
	    } // endif mono

    //
    // Draw stereo waveform data
    //
    else if (wf.wf.nChannels == 2) {
		//
		// 8 bit data is centred around 0x80
		//
        if (wf.wBitsPerSample == 8) {

            // Left channel
            hbr = (HBRUSH)SelectObject(hdc, CreateSolidBrush(LEFTBRUSH));
            y = rc.top + h/4;
            PatBlt(hdc, rc.left, y, w, 1, PATCOPY);

            for (x=0; x<w; x++) {
                b = *((HPBYTE)lpAudio + muldiv32(x,lLen,w) * 2);

                if (b > 0x80) {
                    i = y - MulDiv(b-0x80,(h/4),128);
                    PatBlt(hdc, rc.left+x, i, 1, y-i, PATCOPY);
                	}
                else {
                    i = y + MulDiv(0x80-b,(h/4),128);
                    PatBlt(hdc, rc.left+x, y, 1, i-y, PATCOPY);
                	}
            	}
            DeleteObject(SelectObject(hdc, hbr));
                
            // Right channel
            hbr = (HBRUSH)SelectObject(hdc, CreateSolidBrush(RIGHTBRUSH));
            y = rc.top + h * 3 / 4;
            PatBlt(hdc, rc.left, y, w, 1, PATCOPY);

            for (x=0; x<w; x++) {
                b = *((HPBYTE)lpAudio + muldiv32(x,lLen,w) * 2 + 1);

                if (b > 0x80) {
                    i = y - MulDiv(b-0x80,(h/4),128);
                    PatBlt(hdc, rc.left+x, i, 1, y-i, PATCOPY);
                	}
                else {
                    i = y + MulDiv(0x80-b,(h/4),128);
                    PatBlt(hdc, rc.left+x, y, 1, i-y, PATCOPY);
                	}
            	}
            DeleteObject(SelectObject(hdc, hbr));
        }

		//
		// 16 bit data is centred around 0x00
		//
        else if (wf.wBitsPerSample == 16) {

            // Left channel
            hbr = (HBRUSH)SelectObject(hdc, CreateSolidBrush(LEFTBRUSH));
            y = rc.top + h/4;
            PatBlt(hdc, rc.left, y, w, 1, PATCOPY);

            for (x=0; x<w; x++) {
                i = *((HPINT)lpAudio + muldiv32(x,lLen,w) * 2);
                if (i > 0) {
                    i = y - (int) ((LONG)i * (h/4) / 32768);
                    PatBlt(hdc, rc.left+x, i, 1, y-i, PATCOPY);
                	}
                else {
                    i = (int) ((LONG)i * (h/4) / 32768);
                    PatBlt(hdc, rc.left+x, y, 1, -i, PATCOPY);
                	}
            	}
            DeleteObject(SelectObject(hdc, hbr));

            // Right channel
            hbr = (HBRUSH)SelectObject(hdc, CreateSolidBrush(RIGHTBRUSH));
            y = rc.top + h * 3 / 4;
            PatBlt(hdc, rc.left, y, w, 1, PATCOPY);

            for (x=0; x<w; x++) {
                i = *((HPINT)lpAudio + muldiv32(x,lLen,w) * 2 + 1);
                if (i > 0) {
                	i = y - (int) ((LONG)i * (h/4) / 32768);
                	PatBlt(hdc, rc.left+x, i, 1, y-i, PATCOPY);
               	 	}
                else {
                	i = (int) ((LONG)i * (h/4) / 32768);
                	PatBlt(hdc, rc.left+x, y, 1, -i, PATCOPY);
                	}
            	}
            DeleteObject(SelectObject(hdc, hbr));
        	}
    	} // endif stereo

	if (lpAudio) {
		GlobalFreePtr(lpAudio);
    	lpAudio = NULL;
		}
#endif	// INTERIM_64_BIT
	}








#endif // WIN95STUFF


