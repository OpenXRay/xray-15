/**********************************************************************
 *<
	FILE: PaintDeformTest.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __PAINTDEFORMTEST__H
#define __PAINTDEFORMTEST__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "meshadj.h"
#include "XTCObject.h"
//Need to include the painter in interface header
#include "IPainterInterface.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;


#define PAINTDEFORMTEST_CLASS_ID	Class_ID(0x6d1eb9d9, 0x6d7abbd5)




#define PBLOCK_REF	0


class PaintDefromModData : public LocalModData 
	{
	public:

//holds our offsets delta from the original 
		Tab<Point3> offsetList;

		LocalModData *Clone()
			{
			PaintDefromModData *pmd = new PaintDefromModData();
			pmd->offsetList = offsetList;
			return pmd;
			}


	};


class PainterNodeList
	{
	public:
//just some temp data
		INode *node;
		PaintDefromModData *pmd;
		Matrix3 tmToLocalSpace;
	};

//Need to sub class off of IPainterCanvasInterface_V5 so we get access to all the methods

class PaintDeformTest : public Modifier, public IPainterCanvasInterface_V5  
	{
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

		//From Modifier
		ChannelMask ChannelsUsed()  { return GEOM_CHANNEL|TOPO_CHANNEL; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return GEOM_CHANNEL; }
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
//		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);



		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return FALSE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

//		BOOL HasUVW();
//		void SetGenUVW(BOOL sw);


		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);


		Interval GetValidity(TimeValue t);

		// Automatic texture support
		
		// Loading/Saving
//		IOResult Load(ILoad *iload);
//		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return PAINTDEFORMTEST_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 1; }
		TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable* SubAnim(int i) { return pblock; }

		// TODO: Maintain the number or references here
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return pblock; }
		void SetReference(int i, RefTargetHandle rtarg) { pblock=(IParamBlock2*)rtarg; }




		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		

		IOResult SaveLocalData(ISave *isave, LocalModData *pld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

		//Constructor/Destructor

		PaintDeformTest();
		~PaintDeformTest();	

		void* GetInterface(ULONG id);

// These are the IPainterCanvasInterface_V5 you must instatiate
		// This is called when the user tart a pen stroke
		BOOL  StartStroke();

		//This is called as the user strokes across the mesh or screen with the mouse down
		BOOL  PaintStroke(
						  BOOL hit,
						  IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node,
						  BOOL mirrorOn,
						  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
						  Point3 localMirrorPoint, Point3 localMirrorNormal
						  ) ;

		// This is called as the user ends a strokes when the users has it set to always update
		BOOL  EndStroke();

		// This is called as the user ends a strokes when the users has it set to update on mouse up only
		// the canvas gets a list of all points, normals etc instead of one at a time
		//		int ct - the number of elements in the following arrays
		//  <...> see paintstroke() these are identical except they are arrays of values
		BOOL  EndStroke(int ct, BOOL *hit, IPoint2 *mousePos, 
						  Point3 *worldPoint, Point3 *worldNormal,
						  Point3 *localPoint, Point3 *localNormal,
						  Point3 *bary,  int *index,
						  BOOL *shift, BOOL *ctrl, BOOL *alt, 
						  float *radius, float *str,
						  float *pressure, INode **node,
						  BOOL mirrorOn,
						  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
						  Point3 *localMirrorPoint, Point3 *localMirrorNormal);

		// This is called as the user cancels a stroke by right clicking
		BOOL  CancelStroke();	

		//This is called when the painter want to end a paint session for some reason.
		BOOL  SystemEndPaintSession();
		void PainterDisplay(TimeValue t, ViewExp *vpt, int flags) {}

//just some function to handle UI creatation and destruction
		void InitUI(HWND hWnd);
		void DestroyUI(HWND hWnd);

//Some helper functions to handle painter UI
		void Paint();
		void PaintOptions();


	private:

		PaintDefromModData *GetPMD(INode *pNode);

		ICustButton		*iPaintButton;
		IPainterInterface_V5 *pPainter;
		Tab<PainterNodeList> painterNodeList;
		int lagRate, lagCount;

};


class PaintDeformTestDlgProc : public ParamMap2UserDlgProc {
	public:
		PaintDeformTest *mod;
		PaintDeformTestDlgProc(PaintDeformTest *m) {mod = m;}
		BOOL DlgProc(TimeValue t,IParamMap2* map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this;}
	};


class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	  int count;
	};


class PaintDeformTestClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new PaintDeformTest(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return PAINTDEFORMTEST_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("PaintDeformTest"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



class PointRestore : public RestoreObj {
	public:

		PaintDefromModData *pmd;

		Tab<Point3> uPointList;
		Tab<Point3> rPointList;
		BOOL update;
		PaintDeformTest *mod;

		PointRestore(PaintDeformTest *mod,PaintDefromModData *c) 
			{
			this->mod = mod;
			pmd = c;
			uPointList = pmd->offsetList;
			update = FALSE;
			}   		
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				rPointList = pmd->offsetList;
				}

			pmd->offsetList = uPointList;
			if (update)
				mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

			}
		void Redo()
			{
//watje 9-7-99  198721 
			pmd->offsetList = rPointList;

			if (update)
				mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			update = TRUE;
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_PAINT))); }
	};



#endif // __PAINTDEFORMTEST__H
