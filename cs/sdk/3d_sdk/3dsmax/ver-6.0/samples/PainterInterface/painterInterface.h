/**********************************************************************
 *<
	FILE: painterInterface.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __PAINTERINTERFACE__H
#define __PAINTERINTERFACE__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "icurvctl.h"

#include "BoundsTree.h"
#include "PointGatherer.h"

#include "iPainterInterface.h"

#include "utilapi.h"

#include "MAXScrpt\MAXScrpt.h"
#include "MAXScrpt\Listener.h"
#include "MAXScrpt\MAXObj.h"
#include "imacroscript.h"

//these are includes from the wintab sdk to handle pressure sensitive tablets
#include "msgpack.h"
#include <wintab.h>
#define PACKETDATA	( PK_BUTTONS | PK_NORMAL_PRESSURE)
#define PACKETMODE	PK_BUTTONS
#include <pktdef.h>


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define kPaintInterfaceContext 0x2881722a //<random number> makes your actions contexual

// Flags
#define CID_PAINT 	0x4f298c8b


#define ScriptPrint (the_listener->edit_stream->printf)


enum { painterinterface_param,painterinterface_defaults };
enum { painterinterface_nodelist,
       painterinterface_drawring,painterinterface_drawnormal,painterinterface_drawtrace,
       painterinterface_minsize,painterinterface_maxsize,
       painterinterface_minstr,painterinterface_maxstr,

	   painterinterface_additivemode,
	   painterinterface_falloffgraph,

	   painterinterface_pressureenable,
	   painterinterface_pressureaffects,
	   painterinterface_updateonmouseup,

	   painterinterface_quaddepth,

	   painterinterface_predefinedstrenable,
	   painterinterface_predefinedstrgraph,

	   painterinterface_predefinedsizeenable,
	   painterinterface_predefinedsizegraph,

	   painterinterface_mirrorenable,
	   painterinterface_mirroraxis,
	   painterinterface_mirroroffset,
	   painterinterface_mirrorgizmosize,

	   painterinterface_enablepointgather,

	   painterinterface_buildvnormal,

	   painterinterface_lagrate,
	   
	   
	   painterinterface_normalscale,
	   painterinterface_markerenable,
	   painterinterface_marker,


	   painterinterface_offmeshhittype,
	   painterinterface_offmeshhitzdepth,
	   painterinterface_offmeshhitpos,




	};



class PaintMode;

typedef BOOL (CALLBACK* WTPACKET)(HCTX, int, LPVOID);
typedef HCTX (CALLBACK* WTOPEN)(HWND, LPLOGCONTEXTA, BOOL);
typedef UINT (CALLBACK* WTINFO) (UINT, UINT, LPVOID);




	





class PainterInterface :  public ReferenceTarget, public IPainterInterface_V5, public ViewportDisplayCallback

	{
	public:	
		


		ISpinnerControl *spinTreeDepth;
		ISpinnerControl *spinLagRate;

		ISpinnerControl *spinMinStr;
		ISpinnerControl *spinMaxStr;

		ISpinnerControl *spinMinSize;
		ISpinnerControl *spinMaxSize;

		ISpinnerControl *spinMirrorOffset;
		ISpinnerControl *spinMirrorGizmoSize;

		ISpinnerControl *spinNormalScale, *spinMarker;


	    IParamBlock2 *pblock;
		static PaintMode *paintMode;

		void* GetInterface(ULONG id);

//ref targ methods
		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i)
				{
				if (i == 0) return pblock;
				else return NULL;
				}
		IParamBlock2* GetParamBlockByID(BlockID id)
				{
				if (pblock->ID() == id) return pblock;
				 else return  NULL; 
				 }

		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i)
			{
			if (i==0)
				{
				return (RefTargetHandle)pblock;
				}
			return NULL;
			}

		void SetReference(int i, RefTargetHandle rtarg)
			{
			if (i==0)
				{
				pblock = (IParamBlock2*)rtarg;
				}
			}



		int NumSubs() {return 1;}
		Animatable* SubAnim(int i) { return GetReference(i);}
 

		TSTR SubAnimName(int i)	{return _T("");	}

		int SubNumToRefNum(int subNum) {return -1;}

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message)	
			{
			return REF_SUCCEED;
			}		

		void DeleteThis() 
			{ 
			for (int i = 0; i < customPoints.Count(); i++)
				delete [] customPoints[i];
			}		

		
		
		
		//Constructor/Destructor

		PainterInterface();
		~PainterInterface();

//ViewportDisplayCallback methods
		void Display(TimeValue t, ViewExp *vpt, int flags);
		void GetViewportRect(TimeValue t, ViewExp *vpt, Rect *rect);
		BOOL Foreground() { return TRUE; }

//Painter Interface	methods

		void* GetPaintInterface(ULONG id);


		BOOL  InitializeCallback(ReferenceTarget *canvas);


		BOOL SetCallbackCategory(TSTR CategoryString);

		BOOL  InitializeNodes(int flags, Tab<INode*> &nodeList);

		BOOL  UpdateMeshes(BOOL updatePointGather);

		BOOL  InPaintMode();

		BOOL  StartPaintSession();
		BOOL  EndPaintSession();
		BOOL  BringUpOptions();

		BOOL  SystemEndPaintSession();

		BOOL  BringDownOptions();


		int *RetrieveTimeList(int &ct);
		IPoint2 *RetrieveMouseHitList(int &ct);

		BOOL TestHit(IPoint2 mousePos,
						  Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal
						  );


		BOOL RandomHit(Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  float &strFromFalloff, INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal,
						  int tabIndex);

		BOOL RandomHitAlongStroke(Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  float &strFromFalloff, INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal,
						  int tabIndex);

		BOOL ClearStroke();
		BOOL AddToStroke(IPoint2 mousePos, BOOL rebuildPointGatherData,BOOL updateViewPort);

		int GetStrokeCount();
		float *GetStrokeStr();
		float *GetStrokeRadius();
		Point3 *GetStrokePointWorld();
		Point3 *GetStrokeNormalWorld();
		
		Point3 *GetStrokePointWorldMirror();
		Point3 *GetStrokeNormalWorldMirror();

		float *GetStrokePressure();

		Point3 *GetStrokePointLocal();
		Point3 *GetStrokeNormalLocal();
		Point3 *GetStrokePointLocalMirror();
		Point3 *GetStrokeNormalLocalMirror();

		IPoint2 *GetStrokeMousePos();
		BOOL *GetStrokeHitList();

		Point3 *GetStrokeBary();
		int *GetStrokeIndex();

		BOOL *GetStrokeShift();
		BOOL *GetStrokeCtrl();
		BOOL *GetStrokeAlt();

		INode* *GetStrokeNode();
		int *GetStrokeTime();



		float GetStrFromPoint(Point3 point);

		float *GetPredefineStrStrokeData(int &ct);
		float *GetPredefineSizeStrokeData(int &ct);

		BOOL  GetEnablePointGather();
		void  SetEnablePointGather(BOOL enable);
		
		BOOL LoadCustomPointGather(int ct, Point3 *points, INode *node);

		float *RetrievePointGatherWeights(INode *node, int &ct);
		float *RetrievePointGatherStr(INode *node, int &ct);
		int *RetrievePointGatherIsMirror(INode *node, int &ct);
		Point3 *RetrievePointGatherPoints(INode *node, int &ct);
		BitArray *PointGatherHitVerts();

		BOOL  GetBuildNormalData();
		void  SetBuildNormalData(BOOL enable);

		Point3 *RetrievePointGatherNormals(INode *node, int &ct);
		float *RetrievePointGatherU(INode *node, int &ct);


//functions to access the mirror data
		BOOL  GetMirrorEnable();
		void  SetMirrorEnable(BOOL enable);

		int   GetMirrorAxis();
		void  SetMirrorAxis(int axis);

		Point3 GetMirrorPlaneCenter();

		float GetMirrorOffset();
		void  SetMirrorOffset(float offset);

		float GetMirrorGizmoSize();
		void  SetMirrorGizmoSize(float size);
		Point3 mirrorCenter;


		int GetTreeDepth();
		void SetTreeDepth(int depth);

		BOOL GetUpdateOnMouseUp();
		void SetUpdateOnMouseUp(BOOL update);

		int GetLagRate();
		void SetLagRate(int rate);



		float GetMinStr();
		void  SetMinStr(float str);

		float GetMaxStr();
		void  SetMaxStr(float str);

		float GetMinSize();
		void  SetMinSize(float str);

		float GetMaxSize();
		void  SetMaxSize(float str);

		BOOL  GetDrawRing();
		void  SetDrawRing(BOOL draw);

		BOOL  GetDrawNormal();
		void  SetDrawNormal(BOOL draw);

		BOOL  GetDrawTrace();
		void  SetDrawTrace(BOOL draw);

		BOOL  GetAdditiveMode();
		void  SetAdditiveMode(BOOL enable);

		BOOL  GetPressureEnable();
		void  SetPressureEnable(BOOL enable);

		BOOL  GetPressureAffects();
		void  SetPressureAffects(int affect);

		BOOL  GetPredefinedStrEnable();
		void  SetPredefinedStrEnable(BOOL enable);

		BOOL  GetPredefinedSizeEnable();
		void  SetPredefinedSizeEnable(BOOL enable);

		ICurve *GetPredefineSizeStrokeGraph();
		ICurve *GetPredefineStrStrokeGraph();


		ICurve *GetFalloffGraph();


		float GetNormalScale();
		void SetNormalScale(float scale);

		BOOL GetMarkerEnable();
		void SetMarkerEnable(BOOL on);
		float GetMarker();
		void SetMarker(float pos);



		int GetOffMeshHitType();
		void SetOffMeshHitType(int type);

		float GetOffMeshHitZDepth();
		void SetOffMeshHitZDepth(float depth);

		Point3 GetOffMeshHitPos();
		void SetOffMeshHitPos(Point3 pos);


		BOOL GetIsHit(int tabindex);

//Generic access tools
		BOOL  GetHitOnCP(IPoint2 mousePos, Point3 &worldHit, Point3 &worldNormal);
		BOOL  HitTest(IPoint2 mousePos, int &mousePoint, int flags, BOOL rebuildPointGatherData = TRUE);
		BOOL  ResizeStr(IPoint2 mousePos, int &mousePoint);
		BOOL  ResizeRadius(IPoint2 mousePos, int &mousePoint);

		void WMCommand(int attrib,int attribHight);
		void InitDialog(HWND hWnd);
		void UpdateControls();


		void ZeroTempList();
		void AppendTempList(BOOL hit, IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node);

		void ZeroTempMirrorList();
		void AppendTempMirrorList( Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal);


		int  GetHitCount();
		void GetHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld,
						 float &radius, float &str, int index);

		void GetHitFaceData(Point3 &bary, int &index, INode *node, int tabindex);

		void GetHitPressureData(BOOL &shift, BOOL &ctrl, BOOL &alt, float &pressure , int tabindex);

		void GetMirrorHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld, int tabindex);

		BitArray *GetPointGatherHitVerts(INode *node);


		WTPACKET PaintWTPacket;
		WTOPEN PaintWTOpen;
		WTINFO PaintWTInfo;

		HINSTANCE hWinTabDLL;

		float fpressure;
		void InitTablet(BOOL init);

		void GetSizeAndStr(float &size, float &str);
		void GetSizeAndStr(int ct, float *size, float *str);

		void RunCommand(TSTR command, TSTR category);
		void RunMacro(TSTR macro);
		void RunFunction(Value* fn);

		void RedrawNodes();



		void BringNodesToFront();
		BOOL inStrChange, inRadiusChange;

		void ScriptFunctions(Value* startStroke,Value* paintStroke, Value* endStroke,Value* cancelStroke,Value* systemEnd);

		void StartPaintMode();
		void EndPaintMode();
		BOOL IsCanvas_5_1()
			{
			if (canvas_5_1==NULL) return FALSE;
				else return TRUE;
			};

		void BuildParamBlock();

	private:

   		Value* fnStartStrokeHandler;
   		Value* fnPaintStrokeHandler;
   		Value* fnEndStrokeHandler;
   		Value* fnCancelStrokeHandler;
   		Value* fnSystemEndHandler;

		BOOL hitState;
		BOOL inPaintMode;
		IPainterCanvasInterface_V5 *canvas;
		IPainterCanvasInterface_V5_1 *canvas_5_1;

		BoundsTree meshSearchTree;


		void DrawCrossSection(Point3 a, Point3 align, float length,float strenght, GraphicsWindow *gw);

		Point3 localSpaceHit, localSpaceNormal;
		Point3 worldSpaceHit, worldSpaceNormal;
		Point3 worldSpaceHitMirror, worldSpaceNormalMirror;

		Matrix3 viewTm; 
		BOOL drawRing, drawNormal, drawTrace;
		Tab<Point3> traceWorldSpace;
		Tab<Point3> traceWorldSpaceMirror;

		HWND painterOptionsWindow;
		HWND painterTabletPressure;

		BOOL updateOnMouseUp;
		Tab<IPoint2> mousePosList;
		Tab<BOOL> hitList;
		Tab<Point3> worldPointList;
		Tab<Point3> worldNormalList;
		Tab<Point3> localPointList;
		Tab<Point3> localNormalList;
		Tab<Point3> baryList;
		Tab<int> indexList;
		Tab<BOOL> shiftList;
		Tab<BOOL> ctrlList;
		Tab<BOOL> altList;
		Tab<float> radiusList;
		Tab<float> strList;
		Tab<float> pressureList;
		Tab<INode*> nodeList;
		
		int startTime;
		Tab<int> timeList;

		Tab<Point3> worldPointMirrorList;
		Tab<Point3> worldNormalMirrorList;
		Tab<Point3> localPointMirrorList;
		Tab<Point3> localNormalMirrorList;

		void MirrorPoint(Point3 &point, Point3 &norm);


		float minSize, maxSize;
		float minStr, maxStr;
		BOOL additiveMode;

		void CreateCurveControl();
		ICurve *pCurve;
		ICurveCtl *curveCtl;

		BOOL predefinedStrEnable;
		ICurve *pPredefinedStrCurve;
		ICurveCtl *curvePredefinedStrCtl;

		BOOL predefinedSizeEnable;
		ICurve *pPredefinedSizeCurve;
		ICurveCtl *curvePredefinedSizeCtl;

		BOOL tabletExists;
		BOOL pressureEnable;

		int pressureAffects;

		BOOL mirrorEnable;
		int  mirrorAxis;
		float mirrorOffset;
		float mirrorGizmoSize;

		               		
		void LoadWinTabDLL();
		void UnLoadWinTabDLL();

		void CreateTabletWindow();
		void DestroyTabletWindow();


//		BOOL scriptCallback;
//		TSTR macroCallbackCategory;

		void ApplyPreDeterminedGraphs();

		Point3 ringColor;
		Point3 ringPressedColor;
		Point3 normalColor;
		Point3 normalPressedColor;
		Point3 traceColor;
		Point3 gizmoColor;
		void UpdateColors();

		BOOL inStroke;
		BOOL colorInitialized;

		BOOL enablePointGather;
		PointGatherer pointGather;
		
		Tab<int> customPointsCount;
		Tab<Point3*> customPoints;

		BOOL systemEndPaintSession;



		ICustToolbar *iFalloffToolBar;	
		void SetCurveShape(ICurve *curve, int type);

		int mouseFreeMoveCount;
		void RebuildPointGatherData(BOOL forceRebuild = FALSE);		
		static BOOL loadedDLL;
		
		
};


class PaintMouseProc : public MouseCallBack {
	public:


		PainterInterface *painterInterface;
		IObjParam *ip;
		
		PaintMouseProc(PainterInterface *m,IObjParam *i) {painterInterface=m;ip=i;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);

		
	};

//fix this i think i need to enumerate the nodes in my list
class PainterFGObject : public ChangeFGObject
	{
	BOOL IsValid();
	void Invalidate();
	void Validate();
	void callback(TimeValue t, IScene *scene);

	};

class PaintMode : public CommandMode {
	public:
		ChangeFGObject fgProc;
		PaintMouseProc proc;
		IObjParam *ip;
		PainterInterface *painterInterface;

		PaintMode(PainterInterface *m,IObjParam *i) 
			: fgProc(m), proc(m,i) {ip=i;painterInterface=m;}

		int Class() {return MOVE_COMMAND;}		
		int ID() {return CID_PAINT;}
		MouseCallBack *MouseProc(int *numPoints) {*numPoints=2;return &proc;}
		ChangeForegroundCallback *ChangeFGProc() 
//								{return CHANGE_FG_SELECTED;}
								{ return &fgProc;}
		BOOL ChangeFG(CommandMode *oldMode) 
//								{return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED);} 
								{return oldMode->ChangeFGProc() != &fgProc;}
		void EnterMode();
		void ExitMode();
	};


extern PainterInterface thePainterInterface;

class PainterInterfaceClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { 
						thePainterInterface.BuildParamBlock();
						return &thePainterInterface; 
						}
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID		ClassID() { return PAINTERINTERFACE_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("thePainterInterface"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};




enum {
		painterfn_initializenodes,

		painterfn_numbernodes,
		painterfn_getnode,

		painterfn_inpaintmode,


		painterfn_updatemeshes,
		painterfn_startpaintsession,painterfn_endpaintsession,
		painterfn_paintoptions,
		painterfn_paintmacro,

		painterfn_paintgetishit,
		painterfn_paintgethitpointdata,
		painterfn_paintgethitfacedata,
		painterfn_paintgethitpressuredata,
		painterfn_paintgethitmousepos,

		painterfn_paintgethittime,
		painterfn_paintgethitdist,
		painterfn_paintgethitvec,


		painterfn_paintgetmirrorhitpointdata,

		painterfn_paintgethitcount,


		painterfn_paintgetrandomhit,
		painterfn_paintgetrandomhitalongstroke,
		painterfn_paintgettesthit,

		painterfn_paintclearstroke,
		painterfn_paintaddtostroke,


		painterfn_paintgetcustomhitpointdata,
		painterfn_paintgetcustomhitfacedata,
		painterfn_paintgetcustomhitpressuredata,


		painterfn_paintloadcustompointgather,
		painterfn_paintgetpointgatherhitverts,
		painterfn_paintgetpointgatherhitpoint,
		painterfn_paintgetpointgatherhitstr,
		painterfn_paintgetpointgatherhitweight,
		painterfn_paintgetpointgatherhitnormal,

		painterfn_paintpointinside,


		painterfn_getmirrorcenter,

		painterfn_getupdateonmouseup,
		painterfn_setupdateonmouseup,

		painterfn_getlagrate,
		painterfn_setlagrate,

		painterfn_gettreedepth,
		painterfn_settreedepth,

		painterfn_getminstr,
		painterfn_setminstr,
		painterfn_getmaxstr,
		painterfn_setmaxstr,

		painterfn_getminsize,
		painterfn_setminsize,
		painterfn_getmaxsize,
		painterfn_setmaxsize,

		painterfn_getadditivemode,
		painterfn_setadditivemode,


		painterfn_getdrawring,painterfn_setdrawring,
		painterfn_getdrawnormal,painterfn_setdrawnormal,
		painterfn_getdrawtrace,painterfn_setdrawtrace,

		painterfn_getenablepressure,painterfn_setenablepressure,

		painterfn_getpressureaffects,painterfn_setpressureaffects,

		painterfn_getprefinedstrenable,painterfn_setprefinedstrenable,
		painterfn_getprefinedsizeenable,painterfn_setprefinedsizeenable,

		painterfn_getmirrorenable,painterfn_setmirrorenable,
		painterfn_getmirroraxis,painterfn_setmirroraxis,
		painterfn_getmirroroffset,painterfn_setmirroroffset,
		painterfn_getmirrorgizmosize,painterfn_setmirrorgizmosize,

		painterfn_getpointgatherenable,painterfn_setpointgatherenable,
		painterfn_getbuildnormals,painterfn_setbuildnormals,

		painterfn_getnormalscale,
		painterfn_setnormalscale,

		painterfn_getmarker,
		painterfn_setmarker,
		painterfn_getmarkerenable,
		painterfn_setmarkerenable,

		painterfn_getoffmeshhittype,
		painterfn_setoffmeshhittype,

		painterfn_getoffmeshhitzdepth,
		painterfn_setoffmeshhitzdepth,

		painterfn_getoffmeshhitpos,
		painterfn_setoffmeshhitpos,

		painterfn_scriptfunctions,
		painterfn_undostart,
		painterfn_undoaccept,
		painterfn_undocancel,


	};



class PainterInterfaceActions : public FPStaticInterface {
	public:

		virtual void fnInitializeNodes(int flags,Tab<INode*> *nodelist) =0;

		virtual int  fnGetNumberNodes() =0;
		virtual INode* fnGetNode(int index) =0;

		virtual void fnUpdateMeshes(BOOL updatePointGather) = 0;
		virtual void fnStartPaintSession() =0;
		virtual void fnEndPaintSession() =0;

		virtual BOOL fnInPaintMode() =0;

		virtual void fnPaintOptions() =0;

		virtual void fnLoadPaintMacro(TCHAR *macroString,TCHAR *macroCategory) =0;


		virtual BOOL fnGetIsHit(int tabindex)=0;
		
		virtual void fnGetHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
								  Point3 &hitPointWorld, Point3 &hitNormalWorld,
								  float &radius, float &str, int tabindex)=0;

		virtual void fnGetHitFaceData(Point3 &bary, int &index, INode *node, int tabindex)=0;

		virtual void fnGetHitPressureData(BOOL &shift, BOOL &ctrl, BOOL &alt, float &pressure, int tabindex )=0;

		virtual void fnGetMirrorHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld, int tabindex) = 0;


		virtual Point2* fnGetHitMousePos(int tabindex)=0;



		virtual int fnGetHitTime(int tabindex)=0;
		virtual float fnGetHitDist(int tabindex)=0;
		virtual Point3 *fnGetHitVec(int tabindex)=0;

		virtual BOOL fnGetHitCount()=0; 


		virtual BOOL fnGetRandomHitOnPoint(int tabIndex)=0;
		virtual BOOL fnGetRandomHitAlongStroke(int tabIndex)=0;
		virtual BOOL fnGetTestHit(Point2 p)=0;

		virtual void fnClearStroke() = 0;
		virtual void fnAddToStroke(Point2 mousePos,BOOL rebuildGather, BOOL updateViewPort) = 0;


		
		virtual void fnGetCustomHitPointData(
								  Point3 &hitPointWorld, Point3 &hitNormalWorld,
								  Point3 &hitPointLocal, Point3 &hitNormalLocal,
								   float &str)=0;	
		virtual void fnGetCustomHitFaceData(Point3 &bary, int &index, INode *node)=0;

//		virtual void fnGetCustomHitPressureData(BOOL &shift, BOOL &ctrl, BOOL &alt, float &pressure)=0;

		virtual void fnGetCustomMirrorHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld) = 0;

		virtual BitArray* fnGetPointGatherHitVerts(INode *node)=0;


		virtual void fnLoadCustomPointGather(INode *node, Tab<Point3*> *pointList)=0;

		virtual float fnGetPointGatherHitWeight(INode *node, int index)=0;
		virtual float fnGetPointGatherHitStr(INode *node, int index)=0;
		virtual Point3 *fnGetPointGatherHitPoint(INode *node, int index)=0;
		virtual Point3 *fnGetPointGatherHitNormal(INode *node, int index)=0;
		
		virtual BOOL fnPointInside(Point2 checkPoint,Tab<Point2*> *pointList)=0; 

		virtual Point3 *fnGetMirrorCenter()=0; 

		virtual BOOL GetUpdateOnMouseUp()=0; 
		virtual void SetUpdateOnMouseUp(BOOL update)=0; 

		virtual int GetLagRate()=0; 
		virtual void SetLagRate(int rate)=0; 


		virtual int GetTreeDepth()=0; 
		virtual void SetTreeDepth(int depth)=0; 

		virtual float GetMinStr()=0; 
		virtual void SetMinStr(float str)=0; 
		virtual float GetMaxStr()=0; 
		virtual void SetMaxStr(float str)=0; 

		virtual float GetMinSize()=0; 
		virtual void SetMinSize(float size)=0; 
		virtual float GetMaxSize()=0; 
		virtual void SetMaxSize(float size)=0; 

		virtual BOOL GetAdditiveMode()=0; 
		virtual void SetAdditiveMode(BOOL enable)=0; 


		virtual BOOL  GetDrawRing()=0;
		virtual void  SetDrawRing(BOOL draw)=0;

		virtual BOOL  GetDrawNormal()=0;
		virtual void  SetDrawNormal(BOOL draw)=0;

		virtual BOOL  GetDrawTrace()=0;
		virtual void  SetDrawTrace(BOOL draw)=0;

		virtual BOOL  GetEnablePressure()=0;
		virtual void  SetEnablePressure(BOOL enable)=0;

		virtual int GetPressureAffects()=0; 
		virtual void SetPressureAffects(int affects)=0; 

		virtual BOOL GetPreDefinedStrEnable()=0; 
		virtual void SetPreDefinedStrEnable(BOOL enable)=0; 

		virtual BOOL GetPreDefinedSizeEnable()=0; 
		virtual void SetPreDefinedSizeEnable(BOOL enable)=0; 

		virtual BOOL GetMirrorEnable()=0; 
		virtual void SetMirrorEnable(BOOL enable)=0; 

		virtual int GetMirrorAxis()=0; 
		virtual void SetMirrorAxis(int axis)=0; 

		virtual float GetMirrorOffset()=0; 
		virtual void SetMirrorOffset(float offset)=0; 

		virtual BOOL  GetBuildNormalData() = 0;
		virtual void  SetBuildNormalData(BOOL enable) = 0;

		virtual BOOL  GetEnablePointGather() = 0;
		virtual void  SetEnablePointGather(BOOL enable) = 0;


		virtual float  fnGetNormalScale() = 0;
		virtual void  fnSetNormalScale(float scale) = 0;

		virtual float  fnGetMarker() = 0;
		virtual void  fnSetMarker(float mlen) = 0;
		virtual BOOL  fnGetMarkerEnable() = 0;
		virtual void  fnSetMarkerEnable(BOOL enable) = 0;



		virtual int fnGetOffMeshHitType()=0;
		virtual void fnSetOffMeshHitType(int type)=0;

		virtual float fnGetOffMeshHitZDepth()=0;
		virtual void fnSetOffMeshHitZDepth(float depth)=0;

		virtual Point3* fnGetOffMeshHitPos()=0;
		virtual void fnSetOffMeshHitPos(Point3 pos)=0;

		virtual void fnScriptFunctions(Value* startStroke,Value* paintStroke,Value* endStroke,Value* cancelStroke,Value* systemEnd)=0;

		virtual void fnUndoStart()=0;
		virtual void fnUndoAccept()=0;
		virtual void fnUndoCancel()=0;


};


class PainterInterfaceActionsIMP:public PainterInterfaceActions{

public:

	DECLARE_DESCRIPTOR(PainterInterfaceActionsIMP) //Needed for static interfaces

	BEGIN_FUNCTION_MAP

		VFN_2(painterfn_initializenodes,fnInitializeNodes,TYPE_INT,TYPE_INODE_TAB);

		FN_0(painterfn_numbernodes,TYPE_INT, fnGetNumberNodes);
		FN_1(painterfn_getnode,TYPE_INODE, fnGetNode,TYPE_INT);


		VFN_1(painterfn_updatemeshes,fnUpdateMeshes,TYPE_BOOL);
		VFN_0(painterfn_startpaintsession,fnStartPaintSession);
		VFN_0(painterfn_endpaintsession,fnEndPaintSession);
		VFN_0(painterfn_paintoptions,fnPaintOptions);
		VFN_2(painterfn_paintmacro,fnLoadPaintMacro,TYPE_STRING,TYPE_STRING);

		FN_0(painterfn_inpaintmode,TYPE_BOOL, fnInPaintMode);


		FN_1(painterfn_paintgetishit,TYPE_BOOL, fnGetIsHit,TYPE_INT);

		VFN_7(painterfn_paintgethitpointdata,fnGetHitPointData,TYPE_POINT3_BR,TYPE_POINT3_BR,TYPE_POINT3_BR,TYPE_POINT3_BR,TYPE_FLOAT_BR,TYPE_FLOAT_BR,TYPE_INT);
		VFN_4(painterfn_paintgethitfacedata,fnGetHitFaceData,TYPE_POINT3_BR,TYPE_INT_BR,TYPE_INODE,TYPE_INT);
		VFN_5(painterfn_paintgethitpressuredata,fnGetHitPressureData,TYPE_BOOL_BR,TYPE_BOOL_BR,TYPE_BOOL_BR,TYPE_FLOAT_BR,TYPE_INT);

		VFN_5(painterfn_paintgetmirrorhitpointdata,fnGetMirrorHitPointData,TYPE_POINT3_BR,TYPE_POINT3_BR,TYPE_POINT3_BR,TYPE_POINT3_BR,TYPE_INT);

		FN_0(painterfn_paintgethitcount,TYPE_INT,fnGetHitCount);

		FN_1(painterfn_paintgetrandomhit,TYPE_BOOL,fnGetRandomHitOnPoint,TYPE_INT);
		FN_1(painterfn_paintgetrandomhitalongstroke,TYPE_BOOL,fnGetRandomHitAlongStroke,TYPE_INT);
		FN_1(painterfn_paintgettesthit,TYPE_BOOL,fnGetTestHit,TYPE_POINT2);

		VFN_0(painterfn_paintclearstroke,fnClearStroke);
		VFN_3(painterfn_paintaddtostroke,fnAddToStroke,TYPE_POINT2,TYPE_BOOL,TYPE_BOOL);




		VFN_5(painterfn_paintgetcustomhitpointdata,fnGetCustomHitPointData,TYPE_POINT3_BR,TYPE_POINT3_BR,TYPE_POINT3_BR,TYPE_POINT3_BR,TYPE_FLOAT_BR);
		VFN_3(painterfn_paintgetcustomhitfacedata,fnGetCustomHitFaceData,TYPE_POINT3_BR,TYPE_INT_BR,TYPE_INODE);
//		VFN_4(painterfn_paintgetcustomhitpressuredata,fnGetCustomHitPressureData,TYPE_BOOL_BR,TYPE_BOOL_BR,TYPE_BOOL_BR,TYPE_FLOAT_BR);

		


		FN_1(painterfn_paintgethitmousepos,TYPE_POINT2,fnGetHitMousePos,TYPE_INT);
		FN_1(painterfn_paintgethittime,TYPE_INT,fnGetHitTime,TYPE_INT);
		FN_1(painterfn_paintgethitdist,TYPE_FLOAT,fnGetHitDist,TYPE_INT);
		FN_1(painterfn_paintgethitvec,TYPE_POINT3,fnGetHitVec,TYPE_INT);



		VFN_2(painterfn_paintloadcustompointgather,fnLoadCustomPointGather,TYPE_INODE,TYPE_POINT3_TAB);

		FN_1(painterfn_paintgetpointgatherhitverts,TYPE_BITARRAY, fnGetPointGatherHitVerts,TYPE_INODE);
		FN_2(painterfn_paintgetpointgatherhitweight,TYPE_FLOAT, fnGetPointGatherHitWeight,TYPE_INODE, TYPE_INT);
		FN_2(painterfn_paintgetpointgatherhitstr,TYPE_FLOAT, fnGetPointGatherHitStr,TYPE_INODE, TYPE_INT);
		FN_2(painterfn_paintgetpointgatherhitpoint,TYPE_POINT3, fnGetPointGatherHitPoint,TYPE_INODE, TYPE_INT);
		FN_2(painterfn_paintgetpointgatherhitnormal,TYPE_POINT3, fnGetPointGatherHitNormal,TYPE_INODE, TYPE_INT);



		FN_2(painterfn_paintpointinside,TYPE_BOOL,fnPointInside,TYPE_POINT2,TYPE_POINT2_TAB);


		FN_0(painterfn_getmirrorcenter,TYPE_POINT3,fnGetMirrorCenter);

		
		PROP_FNS(painterfn_getupdateonmouseup, GetUpdateOnMouseUp, painterfn_setupdateonmouseup, SetUpdateOnMouseUp,TYPE_BOOL);
		PROP_FNS(painterfn_gettreedepth, GetTreeDepth, painterfn_settreedepth, SetTreeDepth,TYPE_INT);

		PROP_FNS(painterfn_getlagrate, GetLagRate, painterfn_setlagrate, SetLagRate,TYPE_INT);


		PROP_FNS(painterfn_getminstr, GetMinStr, painterfn_setminstr, SetMinStr,TYPE_FLOAT);
		PROP_FNS(painterfn_getmaxstr, GetMaxStr, painterfn_setmaxstr, SetMaxStr,TYPE_FLOAT);

		PROP_FNS(painterfn_getminsize, GetMinSize, painterfn_setminsize, SetMinSize,TYPE_FLOAT);
		PROP_FNS(painterfn_getmaxsize, GetMaxSize, painterfn_setmaxsize, SetMaxSize,TYPE_FLOAT);

		PROP_FNS(painterfn_getadditivemode, GetAdditiveMode, painterfn_setadditivemode, SetAdditiveMode,TYPE_BOOL);

		PROP_FNS(painterfn_getdrawring, GetDrawRing, painterfn_setdrawring, SetDrawRing,TYPE_BOOL);
		PROP_FNS(painterfn_getdrawnormal, GetDrawNormal, painterfn_setdrawnormal, SetDrawNormal,TYPE_BOOL);
		PROP_FNS(painterfn_getdrawtrace, GetDrawTrace, painterfn_setdrawtrace, SetDrawTrace,TYPE_BOOL);

		PROP_FNS(painterfn_getenablepressure, GetEnablePressure, painterfn_setenablepressure,SetEnablePressure,TYPE_BOOL);

		PROP_FNS(painterfn_getpressureaffects, GetPressureAffects, painterfn_setpressureaffects,SetPressureAffects,TYPE_INT);


		PROP_FNS(painterfn_getprefinedstrenable, GetPreDefinedStrEnable, painterfn_setprefinedstrenable,SetPreDefinedStrEnable,TYPE_BOOL);
		PROP_FNS(painterfn_getprefinedsizeenable, GetPreDefinedSizeEnable, painterfn_setprefinedsizeenable,SetPreDefinedSizeEnable,TYPE_BOOL);



		PROP_FNS(painterfn_getmirrorenable, GetMirrorEnable, painterfn_setmirrorenable,SetMirrorEnable,TYPE_BOOL);
		PROP_FNS(painterfn_getmirroraxis, GetMirrorAxis, painterfn_setmirroraxis,SetMirrorAxis,TYPE_INT);
		PROP_FNS(painterfn_getmirroroffset, GetMirrorOffset, painterfn_setmirroroffset,SetMirrorOffset,TYPE_FLOAT);
		PROP_FNS(painterfn_getmirrorgizmosize, GetMirrorGizmoSize, painterfn_setmirrorgizmosize,SetMirrorGizmoSize,TYPE_FLOAT);

		PROP_FNS(painterfn_getpointgatherenable, GetEnablePointGather, painterfn_setpointgatherenable,SetEnablePointGather,TYPE_BOOL);
		PROP_FNS(painterfn_getbuildnormals, GetBuildNormalData, painterfn_setbuildnormals,SetBuildNormalData,TYPE_BOOL);

		PROP_FNS(painterfn_getnormalscale, fnGetNormalScale, painterfn_setnormalscale,fnSetNormalScale,TYPE_FLOAT);
		PROP_FNS(painterfn_getmarker, fnGetMarker, painterfn_setmarker,fnSetMarker,TYPE_FLOAT);
		PROP_FNS(painterfn_getmarkerenable, fnGetMarkerEnable, painterfn_setmarkerenable,fnSetMarkerEnable,TYPE_BOOL);

		PROP_FNS(painterfn_getoffmeshhittype, fnGetOffMeshHitType, painterfn_setoffmeshhittype,fnSetOffMeshHitType,TYPE_INT);
		PROP_FNS(painterfn_getoffmeshhitpos, fnGetOffMeshHitPos, painterfn_setoffmeshhitpos,fnSetOffMeshHitPos,TYPE_POINT3);
		PROP_FNS(painterfn_getoffmeshhitzdepth, fnGetOffMeshHitZDepth, painterfn_setoffmeshhitzdepth,fnSetOffMeshHitZDepth,TYPE_FLOAT);


		VFN_5(painterfn_scriptfunctions,fnScriptFunctions,TYPE_VALUE,TYPE_VALUE,TYPE_VALUE,TYPE_VALUE,TYPE_VALUE);

		VFN_0(painterfn_undostart,fnUndoStart);
		VFN_0(painterfn_undoaccept,fnUndoAccept);
		VFN_0(painterfn_undocancel,fnUndoCancel);


	END_FUNCTION_MAP

	
	void fnInitializeNodes(int flags,Tab<INode*> *nodelist) ;

	int  fnGetNumberNodes();
	INode* fnGetNode(int index);

	void fnUpdateMeshes(BOOL updatePointGather);
	void fnStartPaintSession() ;
	void fnEndPaintSession() ;
	void fnPaintOptions() ;

	BOOL fnInPaintMode();


	void fnLoadPaintMacro(TCHAR *macroString,TCHAR *macroCategory);


	BOOL fnGetIsHit(int tabindex);

	void fnGetHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
								  Point3 &hitPointWorld, Point3 &hitNormalWorld,
								  float &radius, float &str, int tabindex);

	void fnGetHitFaceData(Point3 &bary, int &index, INode *node, int tabindex);

	void fnGetHitPressureData(BOOL &shift, BOOL &ctrl, BOOL &alt, float &pressure , int tabindex);

	void fnGetMirrorHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld, int tabindex);

	Point2 *fnGetHitMousePos(int tabindex);

	int fnGetHitTime(int tabindex);
	float fnGetHitDist(int tabindex);
	Point3 *fnGetHitVec(int tabindex);

	int fnGetHitCount(); 


	BOOL fnGetRandomHitOnPoint(int tabIndex);
	BOOL fnGetRandomHitAlongStroke(int tabIndex);
	BOOL fnGetTestHit(Point2 p);

	void fnClearStroke() ;
	void fnAddToStroke(Point2 mousePos,BOOL rebuildGatherData, BOOL updateViewPort);


	void fnGetCustomHitPointData(
								  Point3 &hitPointLocal, Point3 &hitNormalLocal,
								  Point3 &hitPointWorld, Point3 &hitNormalWorld,
								   float &str);
	void fnGetCustomHitFaceData(Point3 &bary, int &index, INode *node);

//	void fnGetCustomHitPressureData(BOOL &shift, BOOL &ctrl, BOOL &alt, float &pressure);

	void fnGetCustomMirrorHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld);

	BOOL fnPointInside(Point2 checkPoint,Tab<Point2*> *pointList); 

	Point3 *fnGetMirrorCenter(); 


	BOOL GetUpdateOnMouseUp(); 
	void SetUpdateOnMouseUp(BOOL update); 


	int GetLagRate(); 
	void SetLagRate(int rate); 


	int GetTreeDepth(); 
	void SetTreeDepth(int depth); 

	float GetMinStr(); 
	void SetMinStr(float str); 
	float GetMaxStr(); 
	void SetMaxStr(float str); 

	float GetMinSize(); 
	void SetMinSize(float size); 
	float GetMaxSize(); 
	void SetMaxSize(float size); 

	BOOL GetAdditiveMode(); 
	void SetAdditiveMode(BOOL enable); 

	BOOL  GetDrawRing();
	void  SetDrawRing(BOOL draw);

	BOOL  GetDrawNormal();
	void  SetDrawNormal(BOOL draw);

	BOOL  GetDrawTrace();
	void  SetDrawTrace(BOOL draw);


	BOOL  GetEnablePressure();
	void  SetEnablePressure(BOOL enable);

	int GetPressureAffects(); 
	void SetPressureAffects(int affects); 


	BOOL GetPreDefinedStrEnable(); 
	void SetPreDefinedStrEnable(BOOL enable); 

	BOOL GetPreDefinedSizeEnable(); 
	void SetPreDefinedSizeEnable(BOOL enable); 

	BOOL GetMirrorEnable(); 
	void SetMirrorEnable(BOOL enable); 

	int GetMirrorAxis(); 
	void SetMirrorAxis(int axis); 

	float GetMirrorOffset(); 
	void SetMirrorOffset(float offset); 

	float GetMirrorGizmoSize(); 
	void SetMirrorGizmoSize(float size); 

	BOOL  GetBuildNormalData();
	void  SetBuildNormalData(BOOL enable);

	BOOL  GetEnablePointGather();
	void  SetEnablePointGather(BOOL enable);

	void fnLoadCustomPointGather(INode *node, Tab<Point3*> *pointList);

	BitArray* fnGetPointGatherHitVerts(INode *node);
	float fnGetPointGatherHitWeight(INode *node, int index);
	float fnGetPointGatherHitStr(INode *node, int index);
	Point3 *fnGetPointGatherHitPoint(INode *node, int index);
	Point3 *fnGetPointGatherHitNormal(INode *node, int index);

	float fnGetNormalScale();
	void  fnSetNormalScale(float scale);

	float fnGetMarker();
	void  fnSetMarker(float mlen);
	BOOL  fnGetMarkerEnable();
	void  fnSetMarkerEnable(BOOL enable);


	int fnGetOffMeshHitType();
	void fnSetOffMeshHitType(int type);

	float fnGetOffMeshHitZDepth();
	void fnSetOffMeshHitZDepth(float depth);

	Point3* fnGetOffMeshHitPos();
	void fnSetOffMeshHitPos(Point3 pos);

	void fnScriptFunctions(Value* startStroke,Value* paintStroke,Value* endStroke,Value* cancelStroke,Value* systemEnd);

	void fnUndoStart();
	void fnUndoAccept();
	void fnUndoCancel();

private:

	Point3 offMeshHitPos;

	Point3 customHitPointWorld,customHitNormalWorld;
	Point3 customHitPointLocal,customHitNormalLocal;
	Point3 customBary;
	int customFaceIndex;
	float customStrFalloff;
	Point3 customMirrorHitPointWorld,customMirrorHitNormalWorld;
	Point3 customMirrorHitPointLocal,customMirrorHitNormalLocal;
	INode *customINode;
	BOOL customMirror;
	Point3 dirVec;
	Point2 customHitMousePos;

};

extern PainterInterfaceClassDesc painterInterfaceDesc;


#endif // __PAINTERINTERFACE__H
