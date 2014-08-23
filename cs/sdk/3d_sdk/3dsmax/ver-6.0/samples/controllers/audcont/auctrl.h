//***************************************************************************
//* Audio Amplitude Float Controller for 3D Studio MAX.
//* 
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*

#ifndef __AUCTRL__H
#define __AUCTRL__H

#include "Max.h"
#include "resource.h"
#include "wave.h"
#include "rtwave.h"

#define AUDIO_FLOAT_CONTROL_CNAME		GetString(IDS_CJ_FLOATCONTROL)
#define AUDIO_FLOAT_CONTROL_CLASS_ID1	0x2af81b7c
#define AUDIO_FLOAT_CONTROL_CLASS_ID2	0x5fbb7a86

#define AUDIO_POINT3_CONTROL_CNAME		GetString(IDS_CJ_POINT3CONTROL)
#define AUDIO_POINT3_CONTROL_CLASS_ID1	0x29964a18
#define AUDIO_POINT3_CONTROL_CLASS_ID2	0x3c3e77dc

#define AUDIO_POSITION_CONTROL_CNAME		GetString(IDS_CJ_POSITIONCONTROL)
#define AUDIO_POSITION_CONTROL_CLASS_ID1	0x4eef4cc2
#define AUDIO_POSITION_CONTROL_CLASS_ID2	0x5d975fac

#define AUDIO_ROTATION_CONTROL_CNAME		GetString(IDS_CJ_ROTATIONCONTROL)
#define AUDIO_ROTATION_CONTROL_CLASS_ID1	0x4e436d4b
#define AUDIO_ROTATION_CONTROL_CLASS_ID2	0x2f437701

#define AUDIO_SCALE_CONTROL_CNAME		GetString(IDS_CJ_SCALECONTROL)
#define AUDIO_SCALE_CONTROL_CLASS_ID1	0x38926eba
#define AUDIO_SCALE_CONTROL_CLASS_ID2	0x45c13f80

#define BASE_REFERENCE		0
#define TARGET_REFERENCE	1

extern ClassDesc* GetAudioFloatDesc();
extern ClassDesc* GetAudioPoint3Desc();
#ifndef NO_CONTROLLER_AUDIO_POSITION
extern ClassDesc* GetAudioPositionDesc();
#endif
#ifndef NO_CONTROLLER_AUDIO_ROTATION
extern ClassDesc* GetAudioRotationDesc();
#endif
#ifndef NO_CONTROLLER_AUDIO_SCALE
extern ClassDesc* GetAudioScaleDesc();
#endif

TCHAR *GetString(int id);

class AudioBaseControl : public StdControl {
	public:		
		int		type;	// Controller type
		Interval	range;

		// Directly mapped to user interface widgets
		int channel;
		int absolute;
		int numsamples;
		float threshold;
		int enableRuntime;
		TSTR szFilename;
		int quickdraw;

		// The wave objects
		WaveForm* wave;
		RunTimeWave* rtwave;
		
		// Used primary for the float controller, but also for internal use
		float min, max;

		AudioBaseControl();
		~AudioBaseControl();

		// Paint myself in TrackView
		int PaintTrack(ParamDimensionBase *dim,HDC hdc,Rect& rcTrack,Rect& rcPaint,float zoom,int scroll,DWORD flags);
		// Get a little more room in TrackView to paint the curve
		int GetTrackVSpace( int lineHeight );

		// Return the sample value at a specific time
		float SampleAtTime(TimeValue t, int ignore_oversampling, int painting);

		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 0;}		
		
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}

		BOOL IsAnimated() { return TRUE; }
		BOOL IsLeaf() {return TRUE;}
		// Control methods				
		void Copy(Control *from) {}
		
		Interval GetTimeRange(DWORD flags) {return range;}
		void Hold();
		void MapKeys(TimeMap *map,DWORD flags );

		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int etype);

		// Fixup the path to a referenced sound file.
		BOOL FixupFilename(TSTR &name,TCHAR *dir);

		void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags);
};


#endif

