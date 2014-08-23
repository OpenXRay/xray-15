/**********************************************************************
 *<
	FILE: PainterTester.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __PAINTERTESTER__H
#define __PAINTERTESTER__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "utilapi.h"

//This contains all the canvas and painter interfaces
#include "IPainterInterface.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;



#define PAINTERTESTER_CLASS_ID	Class_ID(0x63589a7d, 0x303de56d)



// if a plugin wants to use the painter interface 
// it must first be of a IPainterCanvas.  This allows the connection
// to the painter

class PainterTester : public UtilityObj,public ReferenceTarget,  public IPainterCanvasInterface_V5 
	{
	public:

		HWND			hPanel;
		IUtil			*iu;
		Interface		*ip;
		ICustButton		*iPaintButton;

		Tab<IParamArray *> nodeList;
		Tab<IParamArray *> mirrorNodeList;
		IPainterInterface_V5 *pPainter;

		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		

		void DeleteThis() { }		
		//Constructor/Destructor

		PainterTester();
		~PainterTester();	

		//this is called when the user presses the paint button
		//which will toggle on/off the painter
		void PaintMode();
		void PaintOptions();
		void SetNodes();
		void CreateCylinder(BOOL isMirror, Point3 pos, Point3 normal,float rad, float height);
		void ResizeCylinders(int ct, float *radius, float *str);
		int radIndex, heightIndex;

	//ParamBlock2 & ReferenceTarget stuff
		void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev=NULL) {}		
		void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next=NULL) {}

		Class_ID ClassID() {return PAINTERTESTER_CLASS_ID;}		
		SClass_ID SuperClassID() { return UTILITY_CLASS_ID; }
		void GetClassName(TSTR& s) { s = GetString(IDS_CLASS_NAME); }


		int NumRefs() { return 0; }
		RefTargetHandle GetReference(int i) { return NULL; }
		void SetReference(int i, RefTargetHandle rtarg) {  }

		int	NumParamBlocks() { return 0; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return NULL; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return NULL; } // return id'd ParamBlock

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message) {return REF_SUCCEED;}

		void* GetInterface(ULONG id);

// Painter Canvas interfaces

		// This is called when the user starts begins to start a pen stroke
		BOOL  StartStroke();

		// This is called as the user sytrokes across the mesh or screen
		BOOL  PaintStroke(BOOL hit, IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node,
						  BOOL mirrorOn,
						  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
						  Point3 localMirrorPoint, Point3 localMirrorNormal);

		// This is called as the user ends a strokes 
		BOOL  EndStroke();
// This is called as the user ends a strokes when the users has it set to update on mouse up only
// the canvas gets a list of all points, normals etc instead of one at a time
		BOOL  EndStroke(int ct, BOOL *hit,IPoint2 *mousePos, 
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

		// This is called if the system wants to end a session
		BOOL  SystemEndPaintSession();
		void PainterDisplay(TimeValue t, ViewExp *vpt, int flags) {}

		BOOL regularSample;
		BOOL sampleInsideBrush;
		BOOL testPointGather;

		BOOL testCustomPoints;
		Tab<INode*> customNodes;
		LoadCustomNodes();

		Tab<INode*> nodes;

};


#endif // __PAINTERTESTER__H
