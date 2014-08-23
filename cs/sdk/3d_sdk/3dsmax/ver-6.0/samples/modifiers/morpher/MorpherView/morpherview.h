/*===========================================================================*\
 | 
 |  FILE:	MorpherView.h
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

#ifndef __RFXUTIL__H
#define __RFXUTIL__H

#include "max.h"
#include "iparamm2.h"
#include "utilapi.h"

#include "resource.h"

// Morpher include file
#include "wm3.h"

#define	MRVUTIL_CLASSID		Class_ID(0x12877dcc, 0x78227462)


TCHAR *GetString(int id);
extern ClassDesc* GetMorpherViewDesc();



/*===========================================================================*\
 |	MorphViewUtil class defn
\*===========================================================================*/

class MorphViewUtil : public UtilityObj {
	public:

		IUtil *iu;
		Interface *ip;

		// Windows handle of our UI
		HWND hPanel;


		// For the morpher modifier pickmode
		ICustButton *pickBut,*bnode;
		INode *Wnode;

		// Our target morpher pointer
		MorphR3	*mp;


		//Constructor/Destructor
		MorphViewUtil();
		~MorphViewUtil();

		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		void LoadMorpherInfo( HWND hWnd );
		void LoadChannelInfo( HWND hWnd, int idx );
};

static MorphViewUtil theMVUtility;


/*===========================================================================*\
 |	Morpher modifier picker, similar to the one in the Morpher Material
\*===========================================================================*/

class GetMorphMod_MorpherView : 
		public PickModeCallback,
		public PickNodeCallback 
{
	public:				
		BOOL isPicking;
		MorphViewUtil *mvup;

		GetMorphMod_MorpherView() {
			mvup = NULL;
			isPicking=FALSE;
		}

		BOOL  HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL  Pick(IObjParam *ip,ViewExp *vpt);		
		BOOL  Filter(INode *node);
		BOOL  RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}

		void  EnterMode(IObjParam *ip);
		void  ExitMode(IObjParam *ip);		

		PickNodeCallback *GetFilter() {return this;}
};

static GetMorphMod_MorpherView theModPickmode_MorpherView;


/*===========================================================================*\
 |	Morpher target picker, same as the one used in the modifier
 |	Used to choose a node in the scene to use as a target
\*===========================================================================*/

class GetMorphTarget_MorpherView : 
		public PickModeCallback,
		public PickNodeCallback {
	public:				
		MorphR3 *mp;
		MorphViewUtil *mvup;
		int idx;

		BOOL isPicking;

		GetMorphTarget_MorpherView() {
			mp=NULL;
			isPicking=FALSE;
		}

		BOOL  HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL  Pick(IObjParam *ip,ViewExp *vpt);		
		BOOL  Filter(INode *node);
		BOOL  RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}

		void  EnterMode(IObjParam *ip);
		void  ExitMode(IObjParam *ip);		

		PickNodeCallback *GetFilter() {return this;}
		
	};

static GetMorphTarget_MorpherView theTargetPickMode_MorpherView;

#endif