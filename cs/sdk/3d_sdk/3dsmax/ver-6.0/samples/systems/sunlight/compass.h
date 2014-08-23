/**********************************************************************
 *<
	FILE: helpers.h

	DESCRIPTION: Helper object header file

	CREATED BY: Tom Hudson

	HISTORY: Created 31 January 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#ifndef __COMPASS__H
#define __COMPASS__H

#include "Max.h"
#include "sunlight.h"

TCHAR *GetString(int id);

#define COMPASS_CLASS_ID Class_ID(0x69011e82, 0x5622b0d)


class CompassRoseObject: public HelperObject {
	public:			
		// Class vars
		static HWND hParams;
		static IObjParam *iObjParams;
		static int dlgShowAxis;
		static float dlgAxisLength;

		// Snap suspension flag (TRUE during creation only)
		BOOL suspendSnap;
					
		// Params
		BOOL showAxis;
		float axisLength;
 		int extDispFlags;

		//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );		
	
		CompassRoseObject();
		~CompassRoseObject();
		
		// From BaseObject
		TCHAR* GetObjectName() { return GetString(IDS_DB_COMPASS_OBJECT); }
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		void SetExtendedDisplay(int flags);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// From Object
		ObjectState Eval(TimeValue time);
		void InitNodeName(TSTR& s) { s = GetString(IDS_DB_COMPASS); }
		ObjectHandle ApplyTransform(Matrix3& matrix) {return this;}
		Interval ObjectValidity(TimeValue t) {return FOREVER;}
		int CanConvertToType(Class_ID obtype) {return FALSE;}
		Object* ConvertToType(TimeValue t, Class_ID obtype) {assert(0);return NULL;}		
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		int DoOwnSelectHilite()	{ return 1; }
		BOOL HasViewDependentBoundingBox() { return TRUE; }

		// Animatable methods
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return COMPASS_CLASS_ID; }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_DB_COMPASS_CLASS)); }
		int IsKeyable(){ return 0;}
		
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
	};				


extern ClassDesc* GetCompassRoseDesc();

#endif // __HELPERS__H
