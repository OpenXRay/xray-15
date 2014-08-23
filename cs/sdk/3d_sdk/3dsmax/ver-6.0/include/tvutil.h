/**********************************************************************
 *<
	FILE: tvutil.h

	DESCRIPTION: Track view utility plug-in class

	CREATED BY:	Rolf Berteig

	HISTORY: 12/18/96
    03/11/02 -- Adam Felt.  Expanded the interfaces.  
	
	WARNING: Plugin compatibility is not preserved between R4.x and R5 
	for TrackviewUtility plugins.  R4.x TrackViewUtility plugins need 
	to be recompiled before running on R5.x and later.  R5 and later 
	plugins will not run properly on previous versions.


 *>	Copyright (c) 1996-2002, All Rights Reserved.
 **********************************************************************/

#ifndef __TVUTIL_H__
#define __TVUTIL_H__

class TrackViewUtility;
#include "ref.h"

// The five track view major modes
#define TVMODE_EDITKEYS			0
#define TVMODE_EDITTIME			1
#define TVMODE_EDITRANGES		2
#define TVMODE_POSRANGES		3
#define TVMODE_EDITFCURVE		4

// This is an interface that is given to track view utilities
// that allows them to access the track view they were launched from.
class ITVUtility : public InterfaceServer {
	public:
		virtual int GetNumTracks()=0;
		virtual Animatable *GetAnim(int i)=0;
		virtual Animatable *GetClient(int i)=0;
		virtual int GetSubNum(int i)=0;
		virtual TSTR GetTrackName(int i)=0;
		virtual BOOL IsSelected(int i)=0;
		virtual void SetSelect(int i,BOOL sel)=0;
		virtual HWND GetTrackViewHWnd()=0;
		virtual int GetMajorMode()=0;
		virtual Interval GetTimeSelection()=0;
		virtual BOOL SubTreeMode()=0;
		virtual Animatable *GetTVRoot()=0;

		// This must be called when a track view utility is closing
		// so that it can be unregistered from notifications
		virtual void TVUtilClosing(TrackViewUtility *util)=0;

		// Access to the trackview interface.  R5 and later only
		virtual ITreeView* GetTVInterface()=0;
	};

// Results for TrackViewUtility::FilterAnim
#define FILTER_SUBTREE	0	// Filter out this anim and it's subtree
#define FILTER_NONE		1 	// Don't filter anything
#define FILTER_ANIM		-1	// Filter out this anim, but include it's subtree
#define FILTER_NODE		-2	// Filter out this node and subAnims, but include it's children


// This is the base class for track view utilities. Plug-ins will
// derive their classes from this class.
class TrackViewUtility : public InterfaceServer {
	public:

		virtual void DeleteThis()=0;		
		virtual void BeginEditParams(Interface *ip,ITVUtility *iu) {}
		virtual void EndEditParams(Interface *ip,ITVUtility *iu) {}

		virtual void TrackSelectionChanged() {}
		virtual void NodeSelectionChanged() {}
		virtual void KeySelectionChanged() {}
		virtual void TimeSelectionChanged() {}
		virtual void MajorModeChanged() {}
		virtual void TrackListChanged() {}
		
		// available in R5 and later only
		virtual int  FilterAnim(Animatable* anim, Animatable* client, int subNum) {return FILTER_NONE;}

		// used to identify registered utilities
		virtual void		GetClassName(TSTR& s)	{ s = TSTR(_T("")); }  
		virtual Class_ID	ClassID()				{ return Class_ID(0,0); }
		// end of R5 additions
	};



#endif //__TVUTIL_H__
