
/****************************************************************************
 *
 *  AUDIO.C
 *
 *  Simple routines to play audio using an AVIStream to get data.
 *
 *  Uses global variables, so only one instance at a time.
 *  (Usually, there's only one sound card, so this isn't so bad.
 *
 **************************************************************************/

/**************************************************************************
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 *  Copyright (c) 1992, 1993  Microsoft Corporation.  All Rights Reserved.
 * 
 **************************************************************************/
#include "strbasic.h"

#ifdef WIN95STUFF

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <vfw.h>

#include "udmIA64.h"
// WIN64 Cleanup: Shuler
// Until the Unified Data Model is complete and
// available in MSVC++ 7.0, this header file is
// required.

#include "audio.h"
#include "muldived.h"

/*--------------------------------------------------------------+
| ****************** AUDIO PLAYING SUPPORT ******************** |
+--------------------------------------------------------------*/

static	HWAVEOUT	shWaveOut = 0;	/* Current MCI device ID */
static	LONG		slBegin;
static	LONG		slWaveBegin;
static	LONG		slCurrent;
static	LONG		slEnd;
static	LONG		slWaveEnd;
static	BOOL		sfLooping;
static	BOOL		sfPlaying = FALSE;

static LONG sTotal;
static LONG sPlayed;

#define MAX_AUDIO_BUFFERS	16
#define MIN_AUDIO_BUFFERS	2
#define AUDIO_BUFFER_SIZE	16384

static	WORD		swBuffers;	    // total # buffers
static	WORD		swBuffersOut;	    // buffers device has
static	WORD		swNextBuffer;	    // next buffer to fill
static	LPWAVEHDR	salpAudioBuf[MAX_AUDIO_BUFFERS];

static	PAVISTREAM	spavi;		    // stream we're playing
static	LONG		slSampleSize;	    // size of an audio sample

static	LONG		sdwBytesPerSec;
static	LONG		sdwSamplesPerSec;

//watje ranges at where the sound will be played
//these are percentages based ver the current time segment
static float startRange;
static float endRange;


/*---------------------------------------------------------------+
| aviaudioCloseDevice -- close the open audio device, if any.    | 
+---------------------------------------------------------------*/
void NEAR aviaudioCloseDevice(void)
	{
#ifndef INTERIM_64_BIT	// CCJ
    WORD	w;

    if (shWaveOut) {
		while (swBuffers > 0) {
		    --swBuffers;
		    waveOutUnprepareHeader(shWaveOut, salpAudioBuf[swBuffers],
					sizeof(WAVEHDR));
	    	GlobalFreePtr((LPSTR) salpAudioBuf[swBuffers]);
			}	
	
		w = waveOutClose(shWaveOut);

		shWaveOut = NULL;	
    	}
#endif	// INTERIM_64_BIT
	}

/*--------------------------------------------------------------+
| aviaudioOpenDevice -- get ready to play waveform data.	|
+--------------------------------------------------------------*/
BOOL FAR aviaudioOpenDevice(HWND hwnd, PAVISTREAM pavi)
	{
#ifndef INTERIM_64_BIT	// CCJ
    WORD		w;
    LPVOID		lpFormat;
    LONG		cbFormat;
    AVISTREAMINFO	strhdr;

    if (!pavi)		// no wave data to play
		return FALSE;
    
    if (shWaveOut)	// already something playing
		return TRUE;

    spavi = pavi;

    AVIStreamInfo(pavi, &strhdr, sizeof(strhdr));

    slSampleSize = (LONG) strhdr.dwSampleSize;
    if (slSampleSize <= 0 || slSampleSize > AUDIO_BUFFER_SIZE)
		return FALSE;
    
    AVIStreamFormatSize(pavi, 0, &cbFormat);

    lpFormat = GlobalAllocPtr(GHND, cbFormat);
    if (!lpFormat)
		return FALSE;

    AVIStreamReadFormat(pavi, 0, lpFormat, &cbFormat);

    sdwSamplesPerSec = ((LPWAVEFORMAT) lpFormat)->nSamplesPerSec;
    sdwBytesPerSec = ((LPWAVEFORMAT) lpFormat)->nAvgBytesPerSec;
    
    w = waveOutOpen(&shWaveOut, (UINT)WAVE_MAPPER, lpFormat,
			(DWORD_PTR) hwnd, 0L, CALLBACK_WINDOW);
		// WIN64 Cleanup: Shuler

    //
    // Maybe we failed because someone is playing sound already.
    // Shut any sound off, and try once more before giving up.
    //
    if (w) {
		sndPlaySound(NULL, 0);
		w = waveOutOpen(&shWaveOut, (UINT)WAVE_MAPPER, lpFormat,
			(DWORD_PTR)hwnd, 0L, CALLBACK_WINDOW);
			// WIN64 Cleanup: Shuler
    	}
		    
    if (w != 0) {
		/* Show error message here? */	
		return FALSE;
    	}
    
    for (swBuffers = 0; swBuffers < MAX_AUDIO_BUFFERS; swBuffers++) {
		if (!(salpAudioBuf[swBuffers] = 
			(LPWAVEHDR)GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, 
				(DWORD)(sizeof(WAVEHDR) + AUDIO_BUFFER_SIZE))))
		    break;
		
		salpAudioBuf[swBuffers]->dwFlags = WHDR_DONE;
		salpAudioBuf[swBuffers]->lpData = (LPSTR) salpAudioBuf[swBuffers] 
							    + sizeof(WAVEHDR);
		salpAudioBuf[swBuffers]->dwBufferLength = AUDIO_BUFFER_SIZE;
		if (!waveOutPrepareHeader(shWaveOut, salpAudioBuf[swBuffers], 
						sizeof(WAVEHDR)))
		    continue;
	
		GlobalFreePtr((LPSTR) salpAudioBuf[swBuffers]);
		break;
    	}
    
    if (swBuffers < MIN_AUDIO_BUFFERS) {
		aviaudioCloseDevice();
		return FALSE;
    	}

    swBuffersOut = 0;
    swNextBuffer = 0;

    sfPlaying = FALSE;
    
    return TRUE;
#else
	return FALSE;
#endif	// INTERIM_64_BIT
	}


/*--------------------------------------------------------------+
| aviaudioTime -						|
| Return the time in milliseconds corresponding to the		|
| currently playing audio sample, or -1 if no audio is playing.	|
|								|
| WARNING: Some sound cards are pretty inaccurate!		|
+--------------------------------------------------------------*/
LONG FAR aviaudioTime(void)
	{
#ifndef INTERIM_64_BIT	// CCJ
    MMTIME	mmtime;
    
    if (!sfPlaying)
	return -1;

    mmtime.wType = TIME_SAMPLES;
    
    waveOutGetPosition(shWaveOut, &mmtime, sizeof(mmtime));

    if (mmtime.wType == TIME_SAMPLES)
		return AVIStreamSampleToTime(spavi, slBegin)
			+ muldiv32(mmtime.u.sample, 1000, sdwSamplesPerSec);
    else if (mmtime.wType == TIME_BYTES)
		return AVIStreamSampleToTime(spavi, slBegin)
			+ muldiv32(mmtime.u.cb, 1000, sdwBytesPerSec);
    else
#endif	// INTERIM_64_BIT
		return -1;
	}


/*--------------------------------------------------------------+
| aviaudioiFillBuffers -					|
| Fill up any empty audio buffers and ship them out to the	|
| device.							|
+--------------------------------------------------------------*/
BOOL NEAR aviaudioiFillBuffers(void)
	{
#ifndef INTERIM_64_BIT	// CCJ
    LONG		lRead;
    WORD		w;
    LONG		lSamplesToPlay;
	int i, sti;
	float perStart,perEnd;
	float per;
    
    /* We're not playing, so do nothing. */
    if (!sfPlaying)
		return TRUE;
    
    while (swBuffersOut < swBuffers) {		
		if (sPlayed >= sTotal) {
			if (sfLooping) {
				/* Looping, so go to the beginning. */
				slCurrent = slBegin;
				sPlayed   = 0;
			} else {
				break;
				}
			}

		if (slCurrent >= slWaveEnd) {
	    	slCurrent = slWaveBegin;			
			}

		/* Figure out how much data should go in this buffer */
		lSamplesToPlay = slWaveEnd - slCurrent;
		if (lSamplesToPlay + sPlayed > sTotal) {
			lSamplesToPlay = sTotal - sPlayed;
			}

		if (lSamplesToPlay > AUDIO_BUFFER_SIZE / slSampleSize)
	    	lSamplesToPlay = AUDIO_BUFFER_SIZE / slSampleSize;


		AVIStreamRead(spavi, slCurrent, lSamplesToPlay,
			      salpAudioBuf[swNextBuffer]->lpData,
			      AUDIO_BUFFER_SIZE,
			      &salpAudioBuf[swNextBuffer]->dwBufferLength,
			      &lRead);
	
		if (lRead != lSamplesToPlay) {
		    return FALSE;
			}
		perStart = (float)sPlayed/(float)sTotal;
		perEnd = (float)(sPlayed+lRead)/(float)sTotal;
//watje if the current time segment of the cache falls outside the range then nuke
		if ( (perEnd < startRange) || (perStart > endRange) ) 
			{
			for (i=0; i < lRead; i++)
				salpAudioBuf[swNextBuffer]->lpData[i] = 0;
			}
//watje if the current end segment of the cache falls inside the range then nuke just part of the cache
		else if ( (perEnd >= startRange) && (perEnd <= endRange)  && (perStart < startRange)) 
			{
//need to just wipe out the first x number of bytes at the beginning
			per = (startRange-perStart)/(perEnd-perStart);
			sti =(int) (per * lRead);
			for (i=0; i < sti; i++)
				salpAudioBuf[swNextBuffer]->lpData[i] = 0;

			}
//watje if the current start segment of the cache falls inside the range then nuke just part of the cache
		else if ( (perStart >= startRange) && (perStart <= endRange)  && (perEnd > endRange)) 
			{
//need to just wipe out the first x number of bytes at the end
			per = (endRange-perStart)/(perEnd-perStart);
			sti =(int) ( per * lRead);
			for (i=sti; i < lRead; i++)
				salpAudioBuf[swNextBuffer]->lpData[i] = 0;

			}


		slCurrent += lRead;
		sPlayed   += lRead;

		w = waveOutWrite(shWaveOut, salpAudioBuf[swNextBuffer],sizeof(WAVEHDR));
	
		if (w != 0) {
	    	return FALSE;
			}
	
		++swBuffersOut;
		++swNextBuffer;
		if (swNextBuffer >= swBuffers) {
		    swNextBuffer = 0;
			}
   		}

    if (swBuffersOut == 0 && sPlayed >= sTotal)
		aviaudioStop();
    
    /* We've filled all of the buffers we can or want to. */
    return TRUE;
#else
	return FALSE;
#endif	// INTERIM_64_BIT
	}

/*--------------------------------------------------------------+
| aviaudioPlay -- Play audio, starting at a given frame		|
|								|
+--------------------------------------------------------------*/
//watje added ranges  where the soundis to be played
//these are percentages of the current time segment
BOOL FAR aviaudioPlay(
	HWND hwnd, PAVISTREAM pavi, 
	LONG lCur, LONG lStart, LONG lEnd, 
	BOOL fWait, BOOL repeat, float lrstart, float lrend)
	{
#ifndef INTERIM_64_BIT	// CCJ
    LONG aviStart = AVIStreamStartTime(pavi);
    LONG aviEnd   = AVIStreamEndTime(pavi);
	LONG aviLen   = aviEnd-aviStart;
	
    if (!aviaudioOpenDevice(hwnd,pavi)) return FALSE;    

	if (!sfPlaying) {
		waveOutPause(shWaveOut);
		}
	
	if (lStart < aviStart) {
		LONG d = (aviStart-lStart)/aviLen + 1;
		lStart += d * aviLen;
		lEnd   += d * aviLen;
		lCur   += d * aviLen;
		}
	if (lStart > aviEnd) {
		LONG d = (lStart-aviEnd)/aviLen + 1;
		lStart -= d * aviLen;
		lEnd   -= d * aviLen;
		lCur   -= d * aviLen;
		}
	
	//sTotal  = ((lEnd-lStart)*sdwSamplesPerSec)/1000;
	//sPlayed = ((lCur-lStart)*sdwSamplesPerSec)/1000;
	sTotal  = MulDiv32((lEnd-lStart), sdwSamplesPerSec, 1000);
	sPlayed = MulDiv32((lCur-lStart), sdwSamplesPerSec, 1000);
	
	slBegin     = AVIStreamTimeToSample(pavi,lStart%aviLen);
	slCurrent   = AVIStreamTimeToSample(pavi,lCur%aviLen);
	slEnd       = AVIStreamTimeToSample(pavi,lEnd%aviLen);
	slWaveBegin = AVIStreamStart(pavi);
	slWaveEnd   = AVIStreamEnd(pavi);

//watje
	startRange = lrstart;
	endRange = lrend;


    sfLooping = repeat;

	if (!sfPlaying) {
    	sfPlaying = TRUE;
    	aviaudioiFillBuffers();		
		}

    //
    // Now unpause the audio and away it goes!
    //
    waveOutRestart(shWaveOut);
    
    //
    // Caller wants us not to return until play is finished
    //
    if (fWait) {
		while (swBuffersOut > 0) {
		    MSG msg;
			while (PeekMessage(&msg,hwnd,0,0,TRUE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				}
			}
    	}
    
#endif	// INTERIM_64_BIT
    return TRUE;
	}

/*--------------------------------------------------------------+
| aviaudioMessage -- handle wave messages received by		|
| window controlling audio playback.  When audio buffers are	|
| done, this routine calls aviaudioiFillBuffers to fill them	|
| up again.							|
+--------------------------------------------------------------*/
void FAR aviaudioMessage(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam)
	{
#ifndef INTERIM_64_BIT	// CCJ
    if (msg == MM_WOM_DONE) {
		--swBuffersOut;
		aviaudioiFillBuffers();
    	}
#endif	// INTERIM_64_BIT
	}


/*--------------------------------------------------------------+
| aviaudioStop -- stop playing, close the device.		|
+--------------------------------------------------------------*/
void FAR aviaudioStop(void)
	{
#ifndef INTERIM_64_BIT	// CCJ
    WORD	w;

    if (shWaveOut != 0) {
		w = waveOutReset(shWaveOut);
		sfPlaying = FALSE;	
		aviaudioCloseDevice();
    	}    
#endif	// INTERIM_64_BIT
	}

#endif // WIN95STUFF
