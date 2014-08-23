/**********************************************************************
 *<
	FILE: painterInterface.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.



	Need to fix the point in spline to take into account z depth



**********************************************************************/

#include "painterInterface.h"
#include "IpainterInterface.h"


#include "meshadj.h"


static PainterInterfaceClassDesc painterInterfaceDesc;

ClassDesc2* GetPainterInterfaceDesc() { return &painterInterfaceDesc; }


static ParamBlockDesc2 painterinterface_param_blk ( 
	painterinterface_param, _T("painterInterface"),  0, &painterInterfaceDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
//table of nodes that we check to hit against
	painterinterface_nodelist,    _T("nodes"),  TYPE_INODE_TAB,		0,	P_VARIABLE_SIZE,	IDS_PW_NODES,
		end,

	painterinterface_drawring,    _T("drawRing"),  TYPE_BOOL,	0,	IDS_PW_DRAWRING,
		p_default,		TRUE,
		end,
	painterinterface_drawnormal,    _T("drawNormal"),  TYPE_BOOL,	0,	IDS_PW_DRAWNORMAL,
		p_default,		TRUE,
		end,
	painterinterface_drawtrace,    _T("drawTrace"),  TYPE_BOOL,	0,	IDS_PW_DRAWTRACE,
		p_default,		TRUE,
		end,

	painterinterface_minsize,    _T("minSize"),  TYPE_FLOAT,	0,	IDS_PW_MINSIZE,
		p_default,		0.0f,
		end,
	painterinterface_maxsize,    _T("maxSize"),  TYPE_FLOAT,	0,	IDS_PW_MAXSIZE,
		p_default,		10.0f,
		end,

	painterinterface_minstr,    _T("minStr"),  TYPE_FLOAT,	0,	IDS_PW_MINSTR,
		p_default,		0.0f,
		end,
	painterinterface_maxstr,    _T("maxStr"),  TYPE_FLOAT,	0,	IDS_PW_MAXSTR,
		p_default,		1.0f,
		end,

	painterinterface_additivemode,    _T("additiveMode"),  TYPE_BOOL,	0,	IDS_PW_ADDITIVEMODE,
		p_default,		FALSE,
		end,
	painterinterface_falloffgraph,    _T("falloffGraph"), 	TYPE_REFTARG, 	P_SUBANIM, 	IDS_PW_FALLOFFGRAPH,
		end,

	painterinterface_pressureenable,    _T("pressureEnable"),  TYPE_BOOL,	0,	IDS_PW_PRESSUREENABLE,
		p_default,		TRUE,
		end,
	painterinterface_pressureaffects,    _T("pressureAffects"),  TYPE_INT,	0,	IDS_PW_PRESSUREAFFECTS,
		p_default,		0,
		end,

	painterinterface_updateonmouseup,    _T("updateOnMouseUp"),  TYPE_BOOL,	0,	IDS_PW_UPDATEONMOUSEUP,
		p_default,		FALSE,
		end,

	painterinterface_quaddepth,    _T("quadDepth"),  TYPE_INT,	0,	IDS_PW_QUADDEPTH,
		p_range, 		2,10, 
		p_default,		6,
		end,


	painterinterface_predefinedstrenable,    _T("predefinedStrEnable"),  TYPE_BOOL,	0,	IDS_PW_PREDEFINEDSTRENABLE,
		p_default,		FALSE,
		end,
	painterinterface_predefinedstrgraph,    _T("predefinedStrGraph"), 	TYPE_REFTARG, 	P_SUBANIM, 	IDS_PW_PREDEFINEDSTRGRAPH,
		end,


	painterinterface_predefinedsizeenable,    _T("predefinedSizeEnable"),  TYPE_BOOL,	0,	IDS_PW_PREDEFINEDSIZEENABLE,
		p_default,		FALSE,
		end,
	painterinterface_predefinedsizegraph,    _T("predefinedSizeGraph"), 	TYPE_REFTARG, 	P_SUBANIM, 	IDS_PW_PREDEFINEDSIZEGRAPH,
		end,


	painterinterface_mirrorenable,    _T("mirrorEnable"),  TYPE_BOOL,	0,	IDS_PW_MIRRORENABLE,
		p_default,		FALSE,
		end,
	painterinterface_mirroraxis,    _T("mirrorAxis"),  TYPE_INT,	0,	IDS_PW_MIRRORAXIS,
		p_range, 		0,2,
		p_default,		0,
		end,
	painterinterface_mirroroffset,    _T("mirrorOffset"),  TYPE_FLOAT,	0,	IDS_PW_MIRROROFFSET,
		p_default,		0.0f,
		end,
	painterinterface_mirrorgizmosize,    _T("mirrorGizmoSize"),  TYPE_FLOAT,	0,	IDS_PW_MIRRORGIZMOSIZE,
		p_default,		50.0f,
		end,

	painterinterface_enablepointgather,    _T("pointGatherEnable"),  TYPE_BOOL,	0,	IDS_PW_POINTGATHERENABLE,
		p_default,		FALSE,
		end,

	painterinterface_buildvnormal,    _T("buildVNormals"),  TYPE_BOOL,	0,	IDS_PW_BUILDVNORMALS,
		p_default,		FALSE,
		end,

	painterinterface_lagrate,    _T("lagRate"), TYPE_INT,	0,	IDS_PW_LAGRATE,
		p_default,		0,
		end,

	painterinterface_normalscale,    _T("normalScale"), TYPE_FLOAT,	0,	IDS_PW_NORMALSCALE,
		p_default,		10.0f,
		end,

	painterinterface_marker,    _T("marker"), TYPE_FLOAT,	0,	IDS_PW_MARKER,
		p_default,		1.0f,
		end,

	painterinterface_markerenable,    _T("markerEnable"), TYPE_BOOL,	0,	IDS_PW_MARKERENABLE,
		p_default,		FALSE,
		end,

	painterinterface_offmeshhittype,    _T("offMeshHitType"), TYPE_BOOL,	0,	IDS_PW_OFFMESHHITTYPE,
		p_default,		0,
		end,

	painterinterface_offmeshhitzdepth,    _T("offMeshHitZDepth"), TYPE_FLOAT,	0,	IDS_PW_OFFMESHHITZDEPTH,
		p_default,		100.0f,
		end,

	painterinterface_offmeshhitpos,    _T("offMeshHitPos"), TYPE_POINT3,	0,	IDS_PW_OFFMESHHITPOS,
		end,
		
	end
	);



PaintMode*      PainterInterface::paintMode = NULL;

BOOL PainterInterface::loadedDLL = FALSE;

PainterInterface thePainterInterface;






//Function publishing declaration

static PainterInterfaceActionsIMP painterInterfaceIMP(PAINTERINTERFACEV5_INTERFACE, _T("thePainterInterface"), 0, &painterInterfaceDesc, 0 ,
  //item ID,  internal_name, description_string, flags

	painterfn_initializenodes, _T("initializeNodes"), 0, TYPE_VOID, 0, 2,
			_T("flags"), 0, TYPE_INT,
			_T("nodeList"), 0, TYPE_INODE_TAB,

	painterfn_numbernodes, _T("numberNodes"), 0, TYPE_INT, 0, 0,
	painterfn_getnode, _T("getNode"), 0, TYPE_INODE, 0, 1,
			_T("index"), 0, TYPE_INT,


	painterfn_updatemeshes, _T("updateMeshes"), 0, TYPE_VOID, 0, 1,
			_T("updatePointGather"), 0, TYPE_BOOL,

	painterfn_startpaintsession, _T("startPaintSession"), 0, TYPE_VOID, 0, 0,
	painterfn_endpaintsession, _T("endPaintSession"), 0, TYPE_VOID, 0, 0,
	painterfn_paintoptions, _T("paintOptions"), 0, TYPE_VOID, 0, 0,
	painterfn_paintmacro, _T("macroCallback"), 0, TYPE_VOID, 0, 2,
			_T("macroName"), 0, TYPE_STRING,
			_T("category"), 0, TYPE_STRING,

	painterfn_inpaintmode, _T("inPaintMode"), 0, TYPE_BOOL, 0, 0,

	painterfn_paintgetishit, _T("getIsHit"), 0, TYPE_BOOL, 0, 1,
			_T("tabIndex"), 0, TYPE_INT,
	painterfn_paintgethitpointdata, _T("getHitPointData"), 0, TYPE_VOID, 0, 7,
			_T("localHit"), 0, TYPE_POINT3_BR,
			_T("localNormal"), 0, TYPE_POINT3_BR,
			_T("worldHit"), 0, TYPE_POINT3_BR,
			_T("worldNormal"), 0, TYPE_POINT3_BR,
			_T("radius"), 0, TYPE_FLOAT_BR,
			_T("str"), 0, TYPE_FLOAT_BR,
			_T("tabIndex"), 0, TYPE_INT,

	painterfn_paintgethitfacedata, _T("getHitFaceData"), 0, TYPE_VOID, 0, 4,
			_T("bary"), 0, TYPE_POINT3_BR,
			_T("faceIndex"), 0, TYPE_INT_BR,
			_T("node"), 0, TYPE_INODE,
			_T("tabIndex"), 0, TYPE_INT,

	painterfn_paintgethitpressuredata, _T("getHitPressureData"), 0, TYPE_VOID, 0, 5,
			_T("shift"), 0, TYPE_BOOL_BR,
			_T("ctrl"), 0, TYPE_BOOL_BR,
			_T("alt"), 0, TYPE_BOOL_BR,
			_T("pressure"), 0, TYPE_FLOAT_BR,
			_T("tabIndex"), 0, TYPE_INT,

	painterfn_paintgetmirrorhitpointdata, _T("getMirrorHitPointData"), 0, TYPE_VOID, 0, 5,
			_T("localMirrorHit"), 0, TYPE_POINT3_BR,
			_T("localMirrorNormal"), 0, TYPE_POINT3_BR,
			_T("worldMirrorHit"), 0, TYPE_POINT3_BR,
			_T("worldMirrorNormal"), 0, TYPE_POINT3_BR,
			_T("tabIndex"), 0, TYPE_INT,


	painterfn_paintgethitmousepos, _T("getHitMousePos"), 0, TYPE_POINT2, 0, 1,
			_T("tabIndex"), 0, TYPE_INT,
	painterfn_paintgethittime, _T("getHitTime"), 0, TYPE_INT, 0, 1,
			_T("tabIndex"), 0, TYPE_INT,
	painterfn_paintgethitdist, _T("getHitDist"), 0, TYPE_FLOAT, 0, 1,
			_T("tabIndex"), 0, TYPE_INT,
	painterfn_paintgethitvec, _T("getHitVec"), 0, TYPE_POINT3, 0, 1,
			_T("tabIndex"), 0, TYPE_INT,


	painterfn_paintgethitcount, _T("getHitCount"), 0, TYPE_INT, 0, 0,

	painterfn_paintloadcustompointgather, _T("loadCustomPointGather"), 0, TYPE_VOID, 0, 2,
			_T("node"), 0,TYPE_INODE,
			_T("pointList"), 0,TYPE_POINT3_TAB,

	painterfn_paintgetpointgatherhitverts, _T("getPointGatherHits"), 0, TYPE_BITARRAY, 0, 1,
			_T("node"), 0, TYPE_INODE,

	painterfn_paintgetpointgatherhitpoint, _T("getPointGatherPoint"), 0, TYPE_POINT3, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
	painterfn_paintgetpointgatherhitstr, _T("getPointGatherStr"), 0, TYPE_FLOAT, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
	painterfn_paintgetpointgatherhitweight, _T("getPointGatherWeight"), 0, TYPE_FLOAT, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,
	painterfn_paintgetpointgatherhitnormal, _T("getPointGatherNormal"), 0, TYPE_POINT3, 0, 2,
			_T("node"), 0, TYPE_INODE,
			_T("index"), 0, TYPE_INT,

	painterfn_paintclearstroke, _T("clearStroke"), 0, TYPE_VOID, 0, 0,
	painterfn_paintaddtostroke, _T("addToStroke"), 0, TYPE_VOID, 0, 3,
			_T("pos"), 0, TYPE_POINT2,
			_T("updatePointGather"), 0, TYPE_BOOL,
			_T("updateViewPort"), 0, TYPE_BOOL,

	painterfn_paintgetrandomhit, _T("getRandomHitOnPoint"), 0, TYPE_BOOL, 0, 1,
			_T("index"), 0, TYPE_INT,

	painterfn_paintgetrandomhitalongstroke, _T("getRandomHitAlongStroke"), 0, TYPE_BOOL, 0, 1,
			_T("index"), 0, TYPE_INT,

	painterfn_paintgettesthit, _T("getTestHit"), 0, TYPE_BOOL, 0, 1,
			_T("Point"), 0, TYPE_POINT2,

	painterfn_paintgetcustomhitpointdata, _T("getCustomHitPointData"), 0, TYPE_VOID, 0, 5,
			_T("localHit"), 0, TYPE_POINT3_BR,
			_T("localNormal"), 0, TYPE_POINT3_BR,
			_T("worldHit"), 0, TYPE_POINT3_BR,
			_T("worldNormal"), 0, TYPE_POINT3_BR,
			_T("str"), 0, TYPE_FLOAT_BR,

	painterfn_paintgetcustomhitfacedata, _T("getCustomHitFaceData"), 0, TYPE_VOID, 0, 3,
			_T("bary"), 0, TYPE_POINT3_BR,
			_T("faceIndex"), 0, TYPE_INT_BR,
			_T("node"), 0, TYPE_INODE,

	painterfn_paintpointinside, _T("isPointInside"), 0, TYPE_BOOL, 0, 2,
			_T("testPoint"), 0, TYPE_POINT2,
			_T("pointList"), 0, TYPE_POINT2_TAB,



	painterfn_getmirrorcenter, _T("getMirrorCenter"), 0, TYPE_POINT3, 0, 0,

	painterfn_scriptfunctions, _T("ScriptFunctions"), 0, TYPE_VOID, 0, 5,
			_T("startStroke"), 0, TYPE_VALUE,
			_T("paintStroke"), 0, TYPE_VALUE,
			_T("endStroke"), 0, TYPE_VALUE,
			_T("cancelStroke"), 0, TYPE_VALUE,
			_T("systemEnd"), 0, TYPE_VALUE,
	painterfn_undostart, _T("undoStart"), 0, TYPE_VOID, 0, 0,
	painterfn_undoaccept, _T("undoAccept"), 0, TYPE_VOID, 0, 0,
	painterfn_undocancel, _T("undoCancel"), 0, TYPE_VOID, 0, 0,

	properties,

	painterfn_getupdateonmouseup, painterfn_setupdateonmouseup, _T("updateOnMouseUp"), IDS_PW_UPDATEONMOUSEUP,
		TYPE_BOOL,
	painterfn_gettreedepth, painterfn_settreedepth, _T("treeDepth"), IDS_PW_QUADDEPTH,
		TYPE_INT, f_range, 2, 10,

	painterfn_getminstr, painterfn_setminstr, _T("minStr"), IDS_PW_MINSTR,
		TYPE_FLOAT, f_range, 0.0f, 1000000.0f,
	painterfn_getmaxstr, painterfn_setmaxstr, _T("maxStr"), IDS_PW_MAXSTR,
		TYPE_FLOAT, f_range, 0.0f, 1000000.0f,

	painterfn_getminsize, painterfn_setminsize, _T("minSize"), IDS_PW_MINSIZE,
		TYPE_FLOAT, f_range, 0.0f, 1000000.0f,
	painterfn_getmaxsize, painterfn_setmaxsize, _T("maxSize"), IDS_PW_MAXSIZE,
		TYPE_FLOAT, f_range, 0.0f, 1000000.0f,

	painterfn_getadditivemode, painterfn_setadditivemode, _T("additiveMode"), IDS_PW_ADDITIVEMODE,
		TYPE_BOOL,

	painterfn_getdrawring, painterfn_setdrawring, _T("drawRing"), IDS_PW_DRAWRING,
		TYPE_BOOL,
	painterfn_getdrawnormal, painterfn_setdrawnormal, _T("drawNormal"), IDS_PW_DRAWRING,
		TYPE_BOOL,
	painterfn_getdrawtrace, painterfn_setdrawtrace, _T("drawTrace"), IDS_PW_DRAWTRACE,
		TYPE_BOOL,

	painterfn_getenablepressure, painterfn_setenablepressure, _T("pressureEnable"), IDS_PW_PRESSUREENABLE,
		TYPE_BOOL,

	painterfn_getpressureaffects, painterfn_setpressureaffects, _T("pressureAffects"), IDS_PW_PRESSUREAFFECTS,
		TYPE_INT, f_range, 1, 4,

	painterfn_getprefinedstrenable, painterfn_setprefinedstrenable, _T("predefinedStrEnable"), IDS_PW_PREDEFINEDSTRENABLE,
		TYPE_BOOL,

	painterfn_getprefinedsizeenable, painterfn_setprefinedsizeenable, _T("predefinedSizeEnable"), IDS_PW_PREDEFINEDSIZEENABLE,
		TYPE_BOOL,

	painterfn_getmirrorenable, painterfn_setmirrorenable, _T("mirrorEnable"), IDS_PW_MIRRORENABLE,
		TYPE_BOOL,
	painterfn_getmirroraxis, painterfn_setmirroraxis, _T("mirrorAxis"), IDS_PW_MIRRORAXIS,
		TYPE_INT, f_range, 1, 3,
	painterfn_getmirroroffset, painterfn_setmirroroffset, _T("mirrorOffset"), IDS_PW_MIRROROFFSET,
		TYPE_FLOAT, f_range, -1000000.0f, 1000000.0f,
	painterfn_getmirrorgizmosize, painterfn_setmirrorgizmosize, _T("mirrorGizmoSize"), IDS_PW_MIRRORGIZMOSIZE,
		TYPE_FLOAT, f_range, 0.0f, 1000000.0f,


	painterfn_getpointgatherenable, painterfn_setpointgatherenable, _T("pointGatherEnable"), IDS_PW_POINTGATHERENABLE,
		TYPE_BOOL,
	painterfn_getbuildnormals, painterfn_setbuildnormals, _T("buildNormals"), IDS_PW_BUILDNORMALS,
		TYPE_BOOL,

	painterfn_getnormalscale, painterfn_setnormalscale, _T("normalScale"), IDS_PW_NORMALSCALE,
		TYPE_FLOAT,
	painterfn_getmarker, painterfn_setmarker, _T("marker"), IDS_PW_MARKER,
		TYPE_FLOAT,
	painterfn_getmarkerenable, painterfn_setmarkerenable, _T("markerEnable"), IDS_PW_MARKERENABLE,
		TYPE_BOOL,

	painterfn_getoffmeshhittype, painterfn_setoffmeshhittype, _T("offMeshHitType"), IDS_PW_OFFMESHHITTYPE,
		TYPE_INT,
	painterfn_getoffmeshhitzdepth, painterfn_setoffmeshhitzdepth, _T("offMeshHitZDepth"), IDS_PW_OFFMESHHITZDEPTH,
		TYPE_FLOAT,
	painterfn_getoffmeshhitpos, painterfn_setoffmeshhitpos, _T("offMeshHitPos"), IDS_PW_OFFMESHHITPOS,
		TYPE_POINT3,

		
	end
);




//--- PainterInterface -------------------------------------------------------
PainterInterface::PainterInterface()
	{

	fnStartStrokeHandler = NULL;
	fnPaintStrokeHandler = NULL;
	fnEndStrokeHandler = NULL;
	fnCancelStrokeHandler = NULL;
	fnSystemEndHandler = NULL;

	pblock = NULL;
	
	inPaintMode = FALSE;

	hitState = FALSE;
	canvas = NULL;

	paintMode = new PaintMode(this,(class IObjParam*) GetCOREInterface());
	GetPainterInterfaceDesc()->MakeAutoParamBlocks(this);
	painterOptionsWindow = NULL;
	tabletExists = FALSE;

	hWinTabDLL = NULL;

	fpressure = 1.0f;


	pCurve = NULL;
	pPredefinedSizeCurve = NULL;
	pPredefinedStrCurve = NULL;

	curveCtl = NULL;
	curvePredefinedSizeCtl = NULL;
	curvePredefinedSizeCtl = NULL;

//	scriptCallback = FALSE;
	colorInitialized = FALSE;


//	UpdateColors();

	inStroke = FALSE;
	enablePointGather = FALSE;

	systemEndPaintSession = TRUE;

	inStrChange = FALSE;
	inRadiusChange = FALSE;

//	LoadWinTabDLL();


	}

PainterInterface::~PainterInterface()
	{


	GetCOREInterface()->DeleteMode(paintMode);
	delete paintMode; 
	paintMode = NULL;
	meshSearchTree.FreeCaches();
	DeleteAllRefsFromMe();

	if (loadedDLL)
		UnLoadWinTabDLL();

	}
void PainterInterface::BuildParamBlock()
	{
	if (pblock == NULL)
		GetPainterInterfaceDesc()->MakeAutoParamBlocks(this);
	}

void PainterInterface::UpdateColors()
	{
	if (!colorInitialized)
		{
		colorInitialized = TRUE;
		TSTR name,category;

		name.printf("%s",_T(GetString(IDS_PW_MIRRORREPROJECT)));
		category.printf("%s",_T(GetString(IDS_PW_PAINTERCOLORS)));

		bool iret;
		iret = ColorMan()->RegisterColor(RINGCOLOR,				name,		category, RGB(43,255,255));
		name.printf("%s",GetString(IDS_PW_NORMALCOLOR));
		iret = ColorMan()->RegisterColor(NORMALCOLOR,			name,		category, RGB(43,255,255));
		name.printf("%s",GetString(IDS_PW_RINGPRESSEDCOLOR));
		iret = ColorMan()->RegisterColor(RINGPRESSEDCOLOR,		name, category, RGB(255,0,0));
		name.printf("%s",GetString(IDS_PW_NORMALPRESSEDCOLOR));
		iret = ColorMan()->RegisterColor(NORMALPRESSEDCOLOR,	name,category, RGB(255,0,0));
		name.printf("%s",GetString(IDS_PW_TRACECOLOR));
		iret = ColorMan()->RegisterColor(TRACECOLOR,			name,	category,		RGB(255,0,0));
		name.printf("%s",GetString(IDS_PW_GIZMOCOLOR));
		iret = ColorMan()->RegisterColor(GIZMOCOLOR,			name,		category, RGB(252,146,0));
		}


	COLORREF cringColor =  ColorMan()->GetColor(RINGCOLOR);
	COLORREF cnormalColor =  ColorMan()->GetColor(NORMALCOLOR);

	COLORREF cringPressedColor =  ColorMan()->GetColor(RINGPRESSEDCOLOR);
	COLORREF cnormalPressedColor =  ColorMan()->GetColor(NORMALPRESSEDCOLOR);

	COLORREF ctraceColor =  ColorMan()->GetColor(TRACECOLOR);
	COLORREF cgizmoColor =  ColorMan()->GetColor(GIZMOCOLOR);

	ringColor = Color(cringColor);
	normalColor = Color(cnormalColor);
	ringPressedColor = Color(cringPressedColor);
	normalPressedColor = Color(cnormalPressedColor);
	traceColor = Color(ctraceColor);
	gizmoColor = Color(cgizmoColor);
	}

void PainterInterface::LoadWinTabDLL()
	{
	hWinTabDLL = NULL;
	hWinTabDLL = LoadLibrary("WINTAB32");
	if (hWinTabDLL)
		{
		PaintWTPacket = (WTPACKET)GetProcAddress(hWinTabDLL,"WTPacket");
		PaintWTOpen = (WTOPEN)GetProcAddress(hWinTabDLL,"WTOpenA");
		PaintWTInfo = (WTINFO)GetProcAddress(hWinTabDLL,"WTInfoA");
   		}
	}

void PainterInterface::UnLoadWinTabDLL()
	{
	if (hWinTabDLL)
		FreeLibrary(hWinTabDLL);       
	hWinTabDLL = NULL;
	PaintWTPacket = NULL;
	PaintWTOpen = NULL;
	PaintWTInfo = NULL;
	}


void PainterInterface::CreateCurveControl()
	{
	ReferenceTarget *ref;
	pblock->GetValue(painterinterface_falloffgraph,0,ref, FOREVER);
	if (ref==NULL)
		{
//setup curve control
		curveCtl = (ICurveCtl *) CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID);
		if (curveCtl)
			{
			pblock->SetValue(painterinterface_falloffgraph,0,curveCtl);

			curveCtl->SetXRange(0.0f,1.0f);
			curveCtl->SetYRange(-1.5f,1.5f);

			curveCtl->RegisterResourceMaker((ReferenceMaker *)this);

			curveCtl->SetNumCurves(1);
			curveCtl->SetZoomValues(278.0f,60.4f);
			curveCtl->SetScrollValues(-14,-29);

			BitArray ba;
			ba.SetSize(1);
			ba.Set(0);//ba.Set(0);ba.Set(0);ba.Set(3);
			curveCtl->SetDisplayMode(ba);
	
			DWORD flags = CC_NONE;
	
			flags |=  CC_DRAWBG ;
			flags |=  CC_DRAWGRID ;
			flags |=  CC_DRAWUTOOLBAR ;
			flags |=  CC_DRAWLTOOLBAR ;
			flags |=  CC_AUTOSCROLL ;
	
	
			flags |=  CC_RCMENU_MOVE_XY ;
			flags |=  CC_RCMENU_MOVE_X ;
			flags |=  CC_RCMENU_MOVE_Y ;
			flags |=  CC_RCMENU_SCALE ;
			flags |=  CC_RCMENU_INSERT_CORNER ;
			flags |=  CC_RCMENU_INSERT_BEZIER ;
			flags |=  CC_RCMENU_DELETE ;

			curveCtl->SetCCFlags(flags);

			pCurve = NULL;
	
			pCurve = curveCtl->GetControlCurve(0);
			SetCurveShape(pCurve, IDC_SMOOTH);
/*			CurvePoint pt;

			pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
			pt.out.x = 0.33f;
			pt.out.y = 0.0f;
			pt.in.x = -0.33f;
			pt.in.y = 0.0f;

			pt.p.x = 0.0f;			
			pt.p.y = 1.0f;			
			pCurve->SetPoint(0,0,&pt);
		
			pt.p.x = 1.0f;			
			pt.p.y = 0.0f;			
			pt.in.x = -.33f;
			pt.in.y = 0.0f;
			pt.out.x = .33f;
			pt.out.y = 0.0f;

			pCurve->SetPoint(0,1,&pt);
			pCurve->SetLookupTableSize(1000);
*/
			}
		}

	
	pblock->GetValue(painterinterface_predefinedstrgraph,0,ref, FOREVER);
	if (ref==NULL)
		{
//setup curve control
		curvePredefinedStrCtl = (ICurveCtl *) CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID);
		if (curvePredefinedStrCtl)
			{
			pblock->SetValue(painterinterface_predefinedstrgraph,0,curvePredefinedStrCtl);

			curvePredefinedStrCtl->SetXRange(0.0f,1.0f);
			curvePredefinedStrCtl->SetYRange(-1.5f,1.5f);

			curvePredefinedStrCtl->RegisterResourceMaker((ReferenceMaker *)this);

			curvePredefinedStrCtl->SetNumCurves(1);
			curvePredefinedStrCtl->SetZoomValues(278.0f,60.4f);
			curvePredefinedStrCtl->SetScrollValues(-14,-29);
			curvePredefinedStrCtl->SetTitle(GetString(IDS_PW_PREDEFINEDSTRGRAPH));

			BitArray ba;
			ba.SetSize(1);
			ba.Set(0);//ba.Set(0);ba.Set(0);ba.Set(3);
			curvePredefinedStrCtl->SetDisplayMode(ba);
	
			DWORD flags = CC_NONE;
	
			flags |=  CC_DRAWBG ;
			flags |=  CC_DRAWGRID ;
			flags |=  CC_DRAWUTOOLBAR ;
			flags |=  CC_DRAWLTOOLBAR ;
			flags |=  CC_AUTOSCROLL ;
			flags |=  CC_ASPOPUP ;
	
	
			flags |=  CC_RCMENU_MOVE_XY ;
			flags |=  CC_RCMENU_MOVE_X ;
			flags |=  CC_RCMENU_MOVE_Y ;
			flags |=  CC_RCMENU_SCALE ;
			flags |=  CC_RCMENU_INSERT_CORNER ;
			flags |=  CC_RCMENU_INSERT_BEZIER ;
			flags |=  CC_RCMENU_DELETE ;


			curvePredefinedStrCtl->SetCCFlags(flags);

//			ICurve *pCurve = NULL;
	
			pPredefinedStrCurve = curvePredefinedStrCtl->GetControlCurve(0);			
			SetCurveShape(pPredefinedStrCurve, IDC_SMOOTH);
/*
			CurvePoint pt;

			pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
			pt.out.x = 0.33f;
			pt.out.y = 0.0f;
			pt.in.x = -0.33f;
			pt.in.y = 0.0f;

			pt.p.x = 0.0f;			
			pt.p.y = 1.0f;			
			pCurve->SetPoint(0,0,&pt);
		
			pt.p.x = 1.0f;			
			pt.p.y = 0.0f;			
			pt.in.x = -.33f;
			pt.in.y = 0.0f;
			pt.out.x = .33f;
			pt.out.y = 0.0f;

			pCurve->SetPoint(0,1,&pt);
			pCurve->SetLookupTableSize(1000);
*/
			}
		}
	pblock->GetValue(painterinterface_predefinedsizegraph,0,ref, FOREVER);
	if (ref==NULL)
		{
//setup curve control
		curvePredefinedSizeCtl = (ICurveCtl *) CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID);
		if (curvePredefinedSizeCtl)
			{
			pblock->SetValue(painterinterface_predefinedsizegraph,0,curvePredefinedSizeCtl);

			curvePredefinedSizeCtl->SetXRange(0.0f,1.0f);
			curvePredefinedSizeCtl->SetYRange(-1.5f,1.5f);

			curvePredefinedSizeCtl->RegisterResourceMaker((ReferenceMaker *)this);

			curvePredefinedSizeCtl->SetNumCurves(1);
			curvePredefinedSizeCtl->SetZoomValues(278.0f,60.4f);
			curvePredefinedSizeCtl->SetScrollValues(-14,-29);
			curvePredefinedSizeCtl->SetTitle(GetString(IDS_PW_PREDEFINEDSIZEGRAPH));

			BitArray ba;
			ba.SetSize(1);
			ba.Set(0);//ba.Set(0);ba.Set(0);ba.Set(3);
			curvePredefinedSizeCtl->SetDisplayMode(ba);
	
			DWORD flags = CC_NONE;
	
			flags |=  CC_DRAWBG ;
			flags |=  CC_DRAWGRID ;
			flags |=  CC_DRAWUTOOLBAR ;
			flags |=  CC_DRAWLTOOLBAR ;
			flags |=  CC_AUTOSCROLL ;
			flags |=  CC_ASPOPUP ;
	
	
			flags |=  CC_RCMENU_MOVE_XY ;
			flags |=  CC_RCMENU_MOVE_X ;
			flags |=  CC_RCMENU_MOVE_Y ;
			flags |=  CC_RCMENU_SCALE ;
			flags |=  CC_RCMENU_INSERT_CORNER ;
			flags |=  CC_RCMENU_INSERT_BEZIER ;
			flags |=  CC_RCMENU_DELETE ;


			curvePredefinedSizeCtl->SetCCFlags(flags);

			ICurve *pCurve = NULL;
	
			pCurve = curvePredefinedSizeCtl->GetControlCurve(0);
			SetCurveShape(pCurve, IDC_SMOOTH);
/*
			CurvePoint pt;

			pt.flags = CURVEP_CORNER | CURVEP_BEZIER | CURVEP_ENDPOINT;
			pt.out.x = 0.33f;
			pt.out.y = 0.0f;
			pt.in.x = -0.33f;
			pt.in.y = 0.0f;

			pt.p.x = 0.0f;			
			pt.p.y = 1.0f;			
			pCurve->SetPoint(0,0,&pt);
		
			pt.p.x = 1.0f;			
			pt.p.y = 0.0f;			
			pt.in.x = -.33f;
			pt.in.y = 0.0f;
			pt.out.x = .33f;
			pt.out.y = 0.0f;

			pCurve->SetPoint(0,1,&pt);
			pCurve->SetLookupTableSize(1000);
*/
			}
		}

	}

#define NNUM_SEGS 48

void PainterInterface::DrawCrossSection(Point3 a, Point3 align, float length,float strength, GraphicsWindow *gw)

	{


	Point3 plist[NNUM_SEGS+1];
	Point3 mka,mkb,mkc,mkd;

	align = Normalize(align);
	
	int ct = 0;
	float angle = TWOPI/float(NNUM_SEGS) ;
	Matrix3 rtm = RotAngleAxisMatrix(align, angle);
	Point3 p(0.0f,0.0f,0.0f);
	if (align.x == 1.0f)
		{
		p.z = length;
		}
	else if (align.y == 1.0f)
		{
		p.x = length;
		}
	else if (align.z == 1.0f)
		{
		p.y = length;
		}
	else if (align.x == -1.0f)
		{
		p.z = -length;
		}
	else if (align.y == -1.0f)
		{
		p.x = -length;
		}
	else if (align.z == -1.0f)
		{
		p.y = -length;
		}
	else 
		{
		p = Normalize(align^Point3(1.0f,0.0f,0.0f))*length;
		}

	for (int i=0; i<NNUM_SEGS; i++) {
		p = p * rtm;
		plist[ct++] = p;
		}

	p = p * rtm;
	plist[ct++] = p;


	for (i=0; i<NNUM_SEGS+1; i++) 
		{
		plist[i].x += a.x;
		plist[i].y += a.y;
		plist[i].z += a.z;
		}
	
	mka = plist[45];
	mkb = plist[9];
	mkc = plist[21];
	mkd = plist[33];

	if (drawRing)
		{
		if (inStroke)
			gw->setColor(LINE_COLOR,ringPressedColor);
		else gw->setColor(LINE_COLOR,ringColor);

		gw->polyline(NNUM_SEGS/2+1, &plist[0], NULL, NULL, 0);

		gw->polyline(NNUM_SEGS/2+1, &plist[NNUM_SEGS/2], NULL, NULL, 0);

		plist[0] = mka;
		plist[1] = mkc;
		gw->polyline(2, plist, NULL, NULL, 0);

		plist[0] = mkb;
		plist[1] = mkd;
		gw->polyline(2, plist, NULL, NULL, 0);

		}


	if (drawNormal)
		{
		plist[0] = a;
		plist[1] = a + (align * strength);

		if (inStroke)
			gw->setColor(LINE_COLOR,normalPressedColor);
		else gw->setColor(LINE_COLOR,normalColor);

		gw->polyline(2, plist, NULL, NULL, 0);
		}



	Point3 center= plist[1];

	}


void PainterInterface::Display(TimeValue t, ViewExp *vpt, int flags)
	{

	mouseFreeMoveCount = 0;

	pblock->GetValue(painterinterface_mirrorenable,0,mirrorEnable,FOREVER);
	pblock->GetValue(painterinterface_mirroraxis,0,mirrorAxis,FOREVER);
	pblock->GetValue(painterinterface_mirrorgizmosize,0,mirrorGizmoSize,FOREVER);
	pblock->GetValue(painterinterface_mirroroffset,0,mirrorOffset,FOREVER);

	GraphicsWindow *gw = vpt->getGW();
	int savedLimits;
	gw->setRndLimits((savedLimits = gw->getRndLimits()) & (~GW_ILLUM) & GW_Z_BUFFER);

	if (hitState)
		{

	
		if (inPaintMode)
			{		
			pblock->GetValue(painterinterface_drawring,0,drawRing,FOREVER);
			pblock->GetValue(painterinterface_drawnormal,0,drawNormal,FOREVER);
			pblock->GetValue(painterinterface_drawtrace,0,drawTrace,FOREVER);

			pblock->GetValue(painterinterface_maxstr,0,maxStr,FOREVER);
			pblock->GetValue(painterinterface_maxsize,0,maxSize,FOREVER);




			//put in pressure weights 
			float str = maxStr;
			float radius = maxSize;

			if (inStrChange)
				{
				IPoint3 ipoint;
				gw->wTransPoint(&worldSpaceHit, &ipoint);
				TSTR strn;
				strn.printf("Str %0.2f",str);
				ipoint.x += 30;
				ipoint.y -= 30;
				gw->wText(&ipoint, strn);
				}
			if (inRadiusChange)
				{
				IPoint3 ipoint;
				gw->wTransPoint(&worldSpaceHit, &ipoint);
				TSTR strn;
				strn.printf("Radius %0.2f",radius);
				ipoint.x += 30;
				ipoint.y -= 30;
				gw->wText(&ipoint, strn);
				}

			GetSizeAndStr(radius, str);
			radius *= 0.5f;
			str *= GetNormalScale();
			
			DrawCrossSection(worldSpaceHit, worldSpaceNormal, radius, str, gw); 
			if (GetMarkerEnable())
				DrawCrossSection(worldSpaceHit+(worldSpaceNormal*(GetMarker()*GetNormalScale())), worldSpaceNormal, radius*0.1f, 0.0f, gw);

			if (mirrorEnable)
				DrawCrossSection(worldSpaceHitMirror, worldSpaceNormalMirror, radius, str, gw); 

			if (drawTrace)
				{
				
				gw->setColor(LINE_COLOR,traceColor);

				for (int i = 1; i < traceWorldSpace.Count(); i++)
					{
					Point3 plist[2+1];
					plist[0] = traceWorldSpace[i-1];
					plist[1] = traceWorldSpace[i];
					gw->polyline(2, plist, NULL, NULL, 0);
					}
				if (mirrorEnable)
					{
					for (i = 1; i < traceWorldSpaceMirror.Count(); i++)
						{
						Point3 plist[2+1];
						plist[0] = traceWorldSpaceMirror[i-1];
						plist[1] = traceWorldSpaceMirror[i];
						gw->polyline(2, plist, NULL, NULL, 0);
						}
					}

				}


				//draw mirror pointer
				//draw mirror trace
			


			}


		;
		}
	if (mirrorEnable)
		{

		gw->setColor(LINE_COLOR,gizmoColor);

		Point3 tempMirrorCenter = mirrorCenter;
		//draw gizmo
		float size = mirrorGizmoSize * 0.5f;
		Point3 plist[4+1];
		if (mirrorAxis == 0)  //x axis
			{
			tempMirrorCenter.x += mirrorOffset;
			plist[0]  = tempMirrorCenter;
			plist[1]  = tempMirrorCenter;
			plist[2]  = tempMirrorCenter;
			plist[3]  = tempMirrorCenter;
			plist[0].y -= size;
			plist[0].z -= size;
			plist[1].y += size;
			plist[1].z -= size;
			plist[2].y += size;
			plist[2].z += size;
			plist[3].y -= size;
			plist[3].z += size;
			}
		else if (mirrorAxis == 1)  //y axis
			{
			tempMirrorCenter.y += mirrorOffset;
			plist[0]  = tempMirrorCenter;
			plist[1]  = tempMirrorCenter;
			plist[2]  = tempMirrorCenter;
			plist[3]  = tempMirrorCenter;
			plist[0].x -= size;
			plist[0].z -= size;
			plist[1].x += size;
			plist[1].z -= size;
			plist[2].x += size;
			plist[2].z += size;
			plist[3].x -= size;
			plist[3].z += size;
			}
		else //z axis
			{
			tempMirrorCenter.z += mirrorOffset;
			plist[0]  = tempMirrorCenter;
			plist[1]  = tempMirrorCenter;
			plist[2]  = tempMirrorCenter;
			plist[3]  = tempMirrorCenter;
			plist[0].x -= size;
			plist[0].y -= size;
			plist[1].x += size;
			plist[1].y -= size;
			plist[2].x += size;
			plist[2].y += size;
			plist[3].x -= size;
			plist[3].y += size;
			}
		gw->polyline(4, plist, NULL, NULL, 1,NULL);
		}
	gw->setRndLimits(savedLimits);

	if ((inStroke) && (canvas!=NULL) )
		{

		if (canvas)
			{
			canvas->PainterDisplay(t, vpt, flags);
			}
		}


	}

void PainterInterface::GetViewportRect(TimeValue t, ViewExp *vpt, Rect *rect)
	{
	int nodeCount = pblock->Count(painterinterface_nodelist);
	if(nodeCount == 0)
		return;

	Box3 box;
	Matrix3 identTM(1);

	for (int i = 0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);

		rect->SetEmpty();
		if (node)
			{
			ObjectState os = node->EvalWorldState(t);
			os.obj->GetDeformBBox(t,box);
	// Put box in world space
			Matrix3 mat = node->GetObjectTM(t);
			box = box * mat;
	// Get a screen bound box
			GraphicsWindow *gw = vpt->getGW();
			gw->setTransform(identTM);		
			DWORD cf;
			DWORD orCf = 0;
			DWORD andCf = 0xffff;
			IPoint3 pt;
			for ( int i = 0; i < 8; i++ ) 
				{
				cf = gw->wTransPoint( &box[i], &pt );
				*rect += IPoint2(pt.x,pt.y);
				}

			}
		}
	// Grow the box to allow for markers
		//FIX groe by strenght and radius
	float expand = maxSize;
	if (maxStr > expand)
		expand = maxStr;
	if (minStr > expand)
		expand = minStr;
	if (minSize > expand)
		expand = minSize;

	int iexpand = (int) expand;

	rect->left   -= iexpand; 
	rect->top    -= iexpand; 
	rect->right  += iexpand; 
	rect->bottom += iexpand; 
	}



void* PainterInterface::GetInterface(ULONG id)
{
	switch(id)
	{
		case PAINTERINTERFACE_V5 : return (IPainterInterface_V5*) this;
			break;
		default: return ReferenceTarget::GetInterface(id);
			break;
	}
}

BOOL  PainterInterface::InitializeCallback(ReferenceTarget *canvas)
	{
	if (canvas != NULL)
		this->canvas = (IPainterCanvasInterface_V5 *) canvas->GetInterface(PAINTERCANVASINTERFACE_V5);
	else this->canvas = NULL;
	if (canvas != NULL)
		this->canvas_5_1 = (IPainterCanvasInterface_V5_1 *) canvas->GetInterface(PAINTERCANVASINTERFACE_V5_1);
	else this->canvas_5_1 = NULL;
	
//	scriptCallback = FALSE;
	return TRUE;
	}

BOOL PainterInterface::SetCallbackCategory(TSTR categoryString)
	{	
//	macroCallbackCategory = categoryString;
//	if (scriptCallback)
//		{
//		this->canvas = NULL;
//		return TRUE;
//		}
	return FALSE;

	
	}


BOOL  PainterInterface::InitializeNodes(int flags, Tab<INode*> &nodeList)
	{
	UpdateColors();
	if	(pblock)
		{
		pblock->ZeroCount(painterinterface_nodelist);
		for (int i =0; i < nodeList.Count(); i++)
			{
			INode *node = nodeList[i];
			if (node)
				{
				pblock->Append(painterinterface_nodelist,1,&node);
				node->FlagForeground(GetCOREInterface()->GetTime());
				}
			}
//update quad trees now with the meshes
		UpdateMeshes(TRUE);

		return TRUE;
		}
	else
		{
		DebugPrint("Error PainterInterface::InitializeNodes for some reason pblock is not initialized\n");
		return FALSE;
		}

	
	}






BOOL  PainterInterface::UpdateMeshes(BOOL updatePointGather) 
	{

	Interface *ip = GetCOREInterface();
	TimeValue t = ip->GetTime();
	


	//loop through all our nodes
	
	
	
	ViewExp *vpt = GetCOREInterface()->GetActiveViewport();
	vpt->GetAffineTM(viewTm);

	meshSearchTree.SetDepth(GetTreeDepth());
	meshSearchTree.SetBuildVNorms(GetBuildNormalData());
	meshSearchTree.FreeCaches();

	int numberNodes = 0;
	for (int i =0; i < pblock->Count(painterinterface_nodelist); i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,t,i);
//get mesh
		
		ObjectState sos;
		if (node)
			{
			sos = node->EvalWorldState(t);
		
			Mesh *msh = NULL;
			TriObject *collapsedtobj = NULL;

			Matrix3  basetm;
			basetm = node->GetObjectTM(t);
	
		
			if (sos.obj)
				{
				if (sos.obj->IsSubClassOf(triObjectClassID))
					{
					TriObject *tobj = (TriObject*)sos.obj;
					msh = &tobj->GetMesh();
					}
//collapse it to a mesh
				else if (sos.obj->CanConvertToType(triObjectClassID))
					{
					collapsedtobj = (TriObject*) sos.obj->ConvertToType(t,triObjectClassID);
					msh = &collapsedtobj->GetMesh();
					}
				if (msh != NULL)
					{
					meshSearchTree.BuildMeshData(msh,basetm,vpt,i);
					numberNodes++;
					}
				else 
					{	
					pblock->Delete(painterinterface_nodelist,i,1);
					i--;
					}
				if ((collapsedtobj) && (collapsedtobj != sos.obj)) collapsedtobj->DeleteThis();

				node->FlagForeground(GetCOREInterface()->GetTime());
				}
			}
		
		}

	int nodeCount = pblock->Count(painterinterface_nodelist);

	mirrorCenter = meshSearchTree.GetWorldCenter();

	if (GetEnablePointGather() && updatePointGather)
		{
		Box3 box = meshSearchTree.GetWorldBounds();
		int ct = 0;
		pointGather.SetNumberNodes(numberNodes);
		pointGather.CreateCells(50, box);



		for (int i =0; i < nodeCount; i++)
			{
			INode *node = pblock->GetINode(painterinterface_nodelist,t,i);
//get mesh
			if (node)
				{
				Point3 *worldSpacePoints = NULL;
				int pointCount;

				if ((i < customPoints.Count()) && (customPoints[i] != NULL))
					{
					pointCount = customPointsCount[i];
					worldSpacePoints = customPoints[i];
					}
				else
					{
					worldSpacePoints = meshSearchTree.GetWorldData(ct,pointCount);
					}

				pointGather.SetCount(ct, pointCount);
				for (int j = 0; j < pointCount; j++)
					{
					pointGather.Add(&worldSpacePoints[j], j, ct);
					}
				ct++;
				}
			}
		}

	GetCOREInterface()->ReleaseViewport(vpt);
	meshSearchTree.BuildQuadTree();
	return TRUE;
	}

BOOL PainterInterface::TestHit(IPoint2 mousePos,
						  Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal
						  )
{

	float z;
	Matrix3 toWorld;

	int nodeIndex;
	DWORD findex;
	BOOL hit = meshSearchTree.HitQuadTree(mousePos,nodeIndex, findex, localPoint, localNormal, bary, z, toWorld);
	if (hit)
		{

		float fpressure, radius, str;
		if (!GetPressureEnable())
			fpressure = 1.0f;
		GetSizeAndStr(radius, str);

		index = findex;
		node = pblock->GetINode(painterinterface_nodelist,0,nodeIndex);
		worldPoint = localPoint * toWorld;
		worldNormal = VectorTransform(toWorld,localNormal);



			

		if (mirrorEnable)
			{
			worldMirrorPoint = worldPoint;
			worldMirrorNormal = worldNormal;

			MirrorPoint(worldMirrorPoint, worldMirrorNormal);
				
			localMirrorPoint = worldMirrorPoint * Inverse(toWorld);
			localMirrorNormal = VectorTransform(Inverse(toWorld), worldMirrorNormal);
			}
		}
	return hit;
}


BOOL PainterInterface::RandomHit(Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  float &strFromFalloff,
						  INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal,
						  int tabIndex)
{
BOOL hit = FALSE;
if (hitState)
	{
	Matrix3 toViewSpace;
	ViewExp *ve = GetCOREInterface()->GetActiveViewport();
	ve->GetAffineTM(toViewSpace);
	
	GraphicsWindow *gw = ve->getGW();	

//put in pressure weights 
	float fpressure, radius, str;
	if (!GetPressureEnable())
		fpressure = 1.0f;
	GetSizeAndStr(radius, str);

	radius *= 0.5f;


	int ct = 0;
	if ((tabIndex <0) || (tabIndex >=mousePosList.Count()))
		ct = mousePosList.Count()-1;
	else ct = tabIndex;
	Point3 lastHitPoint = worldPointList[ct];

	int count = 0;
	
	while ((!hit) && (count < 10))
		{
		float per = (float)rand()/float(RAND_MAX);
		Point3 vec(0.0f,0.0f,0.0f);
		vec.x = 5.0f - (10.0f * per);
		per = (float)rand()/float(RAND_MAX);
		vec.y = 5.0f - (10.0f * per);
		vec.z = 0;
		vec = VectorTransform(Inverse(toViewSpace),vec);
		per = (float)rand()/float(RAND_MAX);
		vec = Normalize(vec) * (radius*per);
		
		

		Point3 hitPoint = lastHitPoint + vec;

		Point3 inPoint,outPoint;
		inPoint = hitPoint;
		DWORD flag = gw->transPoint(&inPoint, &outPoint);

		IPoint2 iPt;
		iPt.x = (int)outPoint.x;
		iPt.y = (int)outPoint.y;

		float z;
		Matrix3 toWorld;

		int nodeIndex;
		DWORD findex;
		hit = meshSearchTree.HitQuadTree(iPt,nodeIndex, findex, localPoint, localNormal, bary, z, toWorld);
		if (hit)
			{
			index = findex;
			node = pblock->GetINode(painterinterface_nodelist,0,nodeIndex);
			worldPoint = localPoint * toWorld;
			worldNormal = VectorTransform(toWorld,localNormal);

			if (Length(worldPoint-lastHitPoint)> radius)
				return FALSE;

			
			strFromFalloff = pCurve->GetValue(0,per) *str;

			if (mirrorEnable)
				{
				worldMirrorPoint = worldPoint;
				worldMirrorNormal = worldNormal;

				MirrorPoint(worldMirrorPoint, worldMirrorNormal);
				
				localMirrorPoint = worldMirrorPoint * Inverse(toWorld);
				localMirrorNormal = VectorTransform(Inverse(toWorld), worldMirrorNormal);
				}
			}
		count++;
		}
//get our world space hit point
//convert into into screen space
	GetCOREInterface()->ReleaseViewport(ve);
	}
return hit;

}

BOOL PainterInterface::RandomHitAlongStroke(Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  float &strFromFalloff,
						  INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal,
						  int tabIndex
						  )
{
BOOL hit = FALSE;
if (hitState)
	{
//get the current stroke dist
	int ct = 0;
	ct = mousePosList.Count();
	if (index >= ct) index = ct -1;
	float dist = 0.0f;

	if (ct == 1)
		{
		return RandomHit(worldPoint, worldNormal,
						 localPoint, localNormal,
						 bary,  index,
						 strFromFalloff,
						 node,
						 mirrorOn,
						 worldMirrorPoint, worldMirrorNormal,
						 localMirrorPoint, localMirrorNormal,
						  -1);
		}
	Matrix3 toViewSpace;
	ViewExp *ve = GetCOREInterface()->GetActiveViewport();
	ve->GetAffineTM(toViewSpace);
	
	GraphicsWindow *gw = ve->getGW();	

//put in pressure weights 
//	float fpressure, radius, str;
//	if (!GetPressureEnable())
//		fpressure = 1.0f;
//	GetSizeAndStr(radius, str);


	for (int i = 1; i < ct; i++)
		{
		Point2 p1((float)mousePosList[i].x,(float)mousePosList[i].y);
		Point2 p2((float)mousePosList[i-1].x,(float)mousePosList[i-1].y);
		dist += Length(p1-p2);
		}	
	
	float totalDist = dist;

//get a random u
	float u = (float)rand()/float(RAND_MAX);
	dist = 0;
	int seg = 0;
	float lastU = 0.0f;
	float randomu = u;
//find the segment that contains that u
	if (tabIndex < 0)
		{
		for (i = 1; i < ct; i++)
			{
			Point2 p1((float)mousePosList[i].x,(float)mousePosList[i].y);
			Point2 p2((float)mousePosList[i-1].x,(float)mousePosList[i-1].y);
			dist += Length(p1-p2);
			float currentU = (dist/totalDist);
			if ( currentU > u)
				{
				seg = i-1;
				u = u-lastU;
				float distU = currentU-lastU;
				u = u/distU;
				i = ct;
				}
			lastU = currentU;
			}
		}
	else
		{
		seg = tabIndex;
		if ((seg+1) >= worldPointList.Count()) seg = worldPointList.Count()-2;
		}
/*
	int ct = 0;
	if ((tabIndex <0) || (tabIndex >=mousePosList.Count()))
		ct = mousePosList.Count()-1;
	else ct = tabIndex;
*/
	Point3 lastHitPoint = worldPointList[seg]+((worldPointList[seg+1]-worldPointList[seg])*u);
	float radius = radiusList[seg]+((radiusList[seg+1]-radiusList[seg])*u);
	float str = strList[seg]+((strList[seg+1]-strList[seg])*u);
	int count = 0;

//DebugPrint("randomu %d %d u %f\n",randomu,seg,u);
	
	while ((!hit) && (count < 10))
		{
		float per = (float)rand()/float(RAND_MAX);
		Point3 vec(0.0f,0.0f,0.0f);
		vec.x = 5.0f - (10.0f * per);
		per = (float)rand()/float(RAND_MAX);
		vec.y = 5.0f - (10.0f * per);
		vec.z = 0;
		vec = VectorTransform(Inverse(toViewSpace),vec);
		per = (float)rand()/float(RAND_MAX);
		vec = Normalize(vec) * (radius*per);
		
//DebugPrint("last hit point %f %f %f\n",lastHitPoint.x,lastHitPoint.y,lastHitPoint.z);		

		Point3 hitPoint = lastHitPoint + vec;

		Point3 inPoint,outPoint;
		inPoint = hitPoint;
		DWORD flag = gw->transPoint(&inPoint, &outPoint);

		IPoint2 iPt;
		iPt.x = (int)outPoint.x;
		iPt.y = (int)outPoint.y;

		float z;
		Matrix3 toWorld;

		int nodeIndex;
		DWORD findex;
		hit = meshSearchTree.HitQuadTree(iPt,nodeIndex, findex, localPoint, localNormal, bary, z, toWorld);

		if (hit)
			{
			index = findex;
			node = pblock->GetINode(painterinterface_nodelist,0,nodeIndex);
			worldPoint = localPoint * toWorld;
			worldNormal = VectorTransform(toWorld,localNormal);

			if (Length(worldPoint-lastHitPoint) <= radius)
				{

			
				strFromFalloff = pCurve->GetValue(0,per) *str;
	
				if (mirrorEnable)
					{
					worldMirrorPoint = worldPoint;
					worldMirrorNormal = worldNormal;

					MirrorPoint(worldMirrorPoint, worldMirrorNormal);
					
					localMirrorPoint = worldMirrorPoint * Inverse(toWorld);
					localMirrorNormal = VectorTransform(Inverse(toWorld), worldMirrorNormal);
					}	
				}
			}
		count++;
		}
//get our world space hit point
//convert into into screen space
	GetCOREInterface()->ReleaseViewport(ve);
	}
return hit;

}

BOOL PainterInterface::ClearStroke()
{
ZeroTempList();
ZeroTempMirrorList();
return TRUE;
}

BOOL PainterInterface::AddToStroke(IPoint2 mousePos, BOOL rebuildPointGather, BOOL updateViewPort)
{
	int mousePoint = worldPointList.Count();

	ViewExp *vpt = GetCOREInterface()->GetActiveViewport();

	Matrix3 tm; 
	vpt->GetAffineTM(tm);
	GetCOREInterface()->ReleaseViewport(vpt);

	if (!(viewTm == tm))
		{
		UpdateMeshes(FALSE);
		}


	Point3 hitPoint(0.0f,0.0f,0.0f);
	Point3 normal(1.0f,0.0f,0.0f);
	Point3 bary(0.0f,0.0f,0.0f);
	int index = 0;
 
	float pressure = 1.0f;
	float str = 1.0f, radius = 10.0f;
	INode *node = NULL;

	Point3 hitPointLocalMirror(0.0f,0.0f,0.0f);
	Point3 normalLocalMirror(1.0f,0.0f,0.0f);
//first hit
	UpdateColors();

				
	pblock->GetValue(painterinterface_drawtrace,0,drawTrace,FOREVER);
	pblock->GetValue(painterinterface_updateonmouseup,0,updateOnMouseUp,FOREVER);

	float z;
	DWORD findex;
	int nodeIndex = -1;
	Matrix3 toWorld;
	BOOL hitMesh = meshSearchTree.HitQuadTree(mousePos,nodeIndex, findex, localSpaceHit, localSpaceNormal, bary, z, toWorld);

	if (GetEnablePointGather())
			pointGather.ClearWeights();

	BOOL hit = FALSE;
	if (!hitMesh)
		{
		hit = GetHitOnCP(mousePos,worldSpaceHit,worldSpaceNormal);
		if (hit)
			{
			localSpaceHit = Point3(0.0f,0.0f,0.0f);
			localSpaceNormal = Point3(0.0f,0.0f,0.0f);
//			worldSpaceNormal = worldNormalList[worldNormalList.Count()-1];
			}

		}
	else
		{
		hit = hitMesh;
		worldSpaceHit = localSpaceHit*toWorld;					
		worldSpaceNormal = VectorTransform(toWorld, localSpaceNormal);
		}
		

	if (hit)
		{
		INode *node= NULL;
		hitState = hit;
		if ((nodeIndex >= 0) && (nodeIndex < pblock->Count(painterinterface_nodelist)))
			node = pblock->GetINode(painterinterface_nodelist,0,nodeIndex);



		if (mirrorEnable) 
			{
			worldSpaceHitMirror = worldSpaceHit;
			worldSpaceNormalMirror = worldSpaceNormal;

			MirrorPoint(worldSpaceHitMirror, worldSpaceNormalMirror);
				
			hitPointLocalMirror = worldSpaceHitMirror * Inverse(toWorld);
			normalLocalMirror = VectorTransform(Inverse(toWorld), worldSpaceNormalMirror);
	
			}



//put in pressure weights 
		if (!GetPressureEnable())
			fpressure = 1.0f;
		pressure = fpressure;
		GetSizeAndStr(radius, str);

		AppendTempList(hitMesh,mousePos, 
								  worldSpaceHit, worldSpaceNormal,
								  localSpaceHit, localSpaceNormal,										 
								  bary,  findex,
								  FALSE, FALSE, FALSE,  
								  radius,str,
								  pressure, node);
		if (mirrorEnable) 
			AppendTempMirrorList( worldSpaceHitMirror, worldSpaceNormalMirror,
									  hitPointLocalMirror, normalLocalMirror);

 		if (rebuildPointGather)
			RebuildPointGatherData(TRUE);
		else
			{
			index = worldPointList.Count()-1;
		
			if (GetEnablePointGather())
				{
//FIX
				Tab<float> dist;
				dist.SetCount(worldPointList.Count());
				float totalDist = 0.0f;
				dist[0] = 0.0f;
				for (int i =1; i < dist.Count(); i++)
					{
					totalDist += Length(worldPointList[i] - worldPointList[i-1]);
					dist[i] = totalDist;
					}
					
				if (index == 0)
					{
					pointGather.AddWeightFromSegment(worldPointList[index],radiusList[index],strList[index],
												 worldPointList[index],radiusList[index],strList[index],
												 pCurve,GetAdditiveMode(),FALSE,dist[index]  );
					if (mirrorEnable)
						pointGather.AddWeightFromSegment(worldPointMirrorList[index],radiusList[index],strList[index],
												 worldPointMirrorList[index],radiusList[index],strList[index],
												 pCurve,GetAdditiveMode(),TRUE ,dist[index] );
					}
				else
					{
					pointGather.AddWeightFromSegment(worldPointList[index-1],radiusList[index-1],strList[index-1],
												 worldPointList[index],radiusList[index],strList[index],
												 pCurve,GetAdditiveMode(),FALSE,dist[index-1]  );
			 		if (mirrorEnable)
						pointGather.AddWeightFromSegment(worldPointMirrorList[index-1],radiusList[index-1],strList[index-1],
												 worldPointMirrorList[index],radiusList[index],strList[index],
												 pCurve,GetAdditiveMode(),TRUE ,dist[index-1] );
					}
				}

			}

					
		if (updateViewPort)
			{
			if (node) GetCOREInterface()->NodeInvalidateRect(node);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			}
		}
	
	return TRUE;
}

int PainterInterface::GetStrokeCount()
{
	return worldPointList.Count();
}

float *PainterInterface::GetStrokeStr()
{
	if (strList.Count() >0) return strList.Addr(0);
	else return NULL;
}

float *PainterInterface::GetStrokeRadius()
{
	if (radiusList.Count() >0) return radiusList.Addr(0);
	else return NULL;
}

Point3 *PainterInterface::GetStrokePointWorld()
{
	if (worldPointList.Count() >0) return worldPointList.Addr(0);
	else return NULL;
}

Point3 *PainterInterface::GetStrokeNormalWorld()
{
	if (worldNormalList.Count() >0) return worldNormalList.Addr(0);
	else return NULL;

}

Point3 *PainterInterface::GetStrokePointWorldMirror()
{
	if (worldPointMirrorList.Count() >0) return worldPointMirrorList.Addr(0);
	else return NULL;
}

Point3 *PainterInterface::GetStrokeNormalWorldMirror()
{
	if (worldNormalMirrorList.Count() >0) return worldNormalMirrorList.Addr(0);
	else return NULL;

}


float *PainterInterface::GetStrokePressure()
{
	if (pressureList.Count() >0) return pressureList.Addr(0);
	else return NULL;
}

Point3 *PainterInterface::GetStrokePointLocal()
{

	if (localPointList.Count() >0) return localPointList.Addr(0);
	else return NULL;
}
Point3 *PainterInterface::GetStrokeNormalLocal()
{
	if (localNormalList.Count() >0) return localNormalList.Addr(0);
	else return NULL;
}
Point3 *PainterInterface::GetStrokePointLocalMirror()
{
	if (localPointMirrorList.Count() >0) return localPointMirrorList.Addr(0);
	else return NULL;
}
Point3 *PainterInterface::GetStrokeNormalLocalMirror()
{
	if (localNormalMirrorList.Count() >0) return localNormalMirrorList.Addr(0);
	else return NULL;

}

IPoint2 *PainterInterface::GetStrokeMousePos()
{
	if (mousePosList.Count() >0) return mousePosList.Addr(0);
	else return NULL;

}
BOOL *PainterInterface::GetStrokeHitList()
{
	if (hitList.Count() >0) return hitList.Addr(0);
	else return NULL;

}

Point3 *PainterInterface::GetStrokeBary()
{
	if (baryList.Count() >0) return baryList.Addr(0);
	else return NULL;
}
int *PainterInterface::GetStrokeIndex()
{
	if (indexList.Count() >0) return indexList.Addr(0);
	else return NULL;
}


BOOL *PainterInterface::GetStrokeShift()
{
	if (shiftList.Count() >0) return shiftList.Addr(0);
	else return NULL;
}
BOOL *PainterInterface::GetStrokeCtrl()
{
	if (ctrlList.Count() >0) return ctrlList.Addr(0);
	else return NULL;
}
BOOL *PainterInterface::GetStrokeAlt()
{
	if (altList.Count() >0) return altList.Addr(0);
	else return NULL;
}


INode **PainterInterface::GetStrokeNode()
{
	if (nodeList.Count() >0) return nodeList.Addr(0);
	else return NULL;

}
int *PainterInterface::GetStrokeTime()
{
	if (timeList.Count() >0) return timeList.Addr(0);
	else return NULL;

}

void PainterInterface::StartPaintMode()
{
	if (canvas_5_1)
		{
		canvas_5_1->CanvasStartPaint();
		GetCOREInterface()->RegisterViewportDisplayCallback(FALSE, (ViewportDisplayCallback *)this);

		inPaintMode = TRUE;
		CreateCurveControl();
	//make sure to update the meshes
		UpdateMeshes(TRUE);
		CreateTabletWindow();
		systemEndPaintSession = TRUE;
		mouseFreeMoveCount = 0;

		}
}
void PainterInterface::EndPaintMode()
{
	if (canvas_5_1)
		{
		canvas_5_1->CanvasEndPaint();
		systemEndPaintSession = FALSE;
		pblock->ZeroCount(painterinterface_nodelist);
		GetCOREInterface()->UnRegisterViewportDisplayCallback(FALSE, (ViewportDisplayCallback *)this);

		inPaintMode = FALSE;
	//make sure we release the bounds tree since it can consume lots of mememory
		meshSearchTree.FreeCaches();
		DestroyTabletWindow();
	//	canvas = NULL;
	//end paint mode
		int ct = pblock->Count(painterinterface_nodelist);

		fnStartStrokeHandler = NULL;
		fnPaintStrokeHandler = NULL;
		fnEndStrokeHandler = NULL;
		fnCancelStrokeHandler = NULL;
		fnSystemEndHandler = NULL;


		}

}


BOOL  PainterInterface::StartPaintSession() 
	{
	if (canvas_5_1)
		{
		if (GetCOREInterface()->GetCommandMode()== paintMode)
			GetCOREInterface()->SetStdCommandMode(CID_OBJMOVE);
		else GetCOREInterface()->SetCommandMode(paintMode);
		return TRUE;
		}
	else
		{
		GetCOREInterface()->RegisterViewportDisplayCallback(FALSE, (ViewportDisplayCallback *)this);

		inPaintMode = TRUE;
	//start paint mode
		GetCOREInterface()->SetCommandMode(paintMode);
		CreateCurveControl();
	//make sure to update the meshes
		UpdateMeshes(TRUE);
		CreateTabletWindow();
		systemEndPaintSession = TRUE;
		mouseFreeMoveCount = 0;
		return TRUE;
		}
	}

BOOL  PainterInterface::EndPaintSession()  
	{
	if (canvas_5_1)
		{
		return TRUE;
		}
	else
		{

		systemEndPaintSession = FALSE;
		pblock->ZeroCount(painterinterface_nodelist);
		GetCOREInterface()->UnRegisterViewportDisplayCallback(FALSE, (ViewportDisplayCallback *)this);

		inPaintMode = FALSE;
		GetCOREInterface()->SetStdCommandMode(CID_OBJMOVE);
	//make sure we release the bounds tree since it can consume lots of mememory
		meshSearchTree.FreeCaches();
		DestroyTabletWindow();
		canvas = NULL;
	//end paint mode
		int ct = pblock->Count(painterinterface_nodelist);

		fnStartStrokeHandler = NULL;
		fnPaintStrokeHandler = NULL;
		fnEndStrokeHandler = NULL;
		fnCancelStrokeHandler = NULL;
		fnSystemEndHandler = NULL;

		return TRUE;
		}
	}

PainterInterface::SystemEndPaintSession()
	{
	if (systemEndPaintSession)
		{

		if (canvas)
			canvas->SystemEndPaintSession( );
		else RunFunction(fnSystemEndHandler);
//			if (scriptCallback)
//			RunCommand(macroCallbackCategory,"SystemEndPaintSession");


		EndPaintSession();
		}
	return TRUE;
	}


BOOL  PainterInterface::InPaintMode()
	{
	return inPaintMode;
	}


BOOL PainterInterface::LoadCustomPointGather(int ct, Point3 *points, INode *incNode)
	{
	int nodeCount = pblock->Count(painterinterface_nodelist);
	for (int i =0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
		if (node==incNode)
			{
			int count = customPoints.Count();
			if (i >= customPoints.Count())
				{
				customPointsCount.SetCount(i+1);
				customPoints.SetCount(i+1);
				}
			for (int j = count; j < customPoints.Count(); j++)
				{
				customPoints[j] = NULL;
				customPointsCount[j] = 0;
				}

			customPointsCount[i] = ct;
			customPoints[i] = new Point3[ct];
			Point3 *p2 = customPoints[i];
			memcpy(customPoints[i], points, ct * sizeof(Point3));
//			Point3 *p = customPoints[i];
//			Point3 *p1 = points;


//			pointGather.SetCustomPoints(i,ct, points);
			return TRUE;
			}
		}
	return FALSE;

	}

IPoint2 *PainterInterface::RetrieveMouseHitList(int &ct)
	{
	ct = timeList.Count();
	if (ct == 0) return NULL;
	return mousePosList.Addr(0);
	}

int *PainterInterface::RetrieveTimeList( int &ct)
	{
	ct = timeList.Count();
	if (ct == 0) return NULL;
	return timeList.Addr(0);
	}

Point3 *PainterInterface::RetrievePointGatherPoints(INode *incNode, int &ct)
	{	
	ct = 0;
	int nodeCount = pblock->Count(painterinterface_nodelist);
	for (int i =0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
		if (node==incNode)
			{
			if ((i < customPoints.Count()) && (customPoints[i] != NULL))
				{
				ct = customPointsCount[i];
				return customPoints[i];
				}
			else
				{
				return meshSearchTree.GetWorldData(i,ct);
				}
			}
		}
	return NULL;

	
	}

float *PainterInterface::RetrievePointGatherWeights(INode *incNode, int &ct)
	{	
	ct = 0;
	int nodeCount = pblock->Count(painterinterface_nodelist);
	for (int i =0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
		if (node==incNode)
			return pointGather.GetWeights(i,ct) ;
		}
	return NULL;
	}

BOOL *PainterInterface::RetrievePointGatherIsMirror(INode *incNode, int &ct)
	{	
	ct = 0;
	int nodeCount = pblock->Count(painterinterface_nodelist);
	for (int i =0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
		if (node==incNode)
			return pointGather.GetIsMirror(i,ct) ;
		}
	return NULL;
	}


Point3 *PainterInterface::RetrievePointGatherNormals(INode *incNode, int &ct)
	{	
	ct = 0;
	int nodeCount = pblock->Count(painterinterface_nodelist);
	for (int i =0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
		if (node==incNode)
			return meshSearchTree.GetNormals(i,ct) ;
		}

	return NULL;

	
	}

float *PainterInterface::RetrievePointGatherStr(INode *incNode, int &ct)
	{	
	ct =0;
	int nodeCount = pblock->Count(painterinterface_nodelist);
	for (int i =0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
		if (node==incNode)
			return pointGather.GetStr(i,ct) ;
		}
	return NULL;

	
	}
float *PainterInterface::RetrievePointGatherU(INode *incNode, int &ct)
	{
	ct = 0;
	int nodeCount = pblock->Count(painterinterface_nodelist);
	for (int i =0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
		if (node==incNode)
			return pointGather.GetU(i,ct) ;
		}
	return NULL;

	
	}

void PainterInterface::RedrawNodes()
	{
	int nodeCount = pblock->Count(painterinterface_nodelist);
	for (int i =0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
		if (node)
			GetCOREInterface()->NodeInvalidateRect(node);
		}
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}

void PainterInterface::MirrorPoint(Point3 &point, Point3 &norm)
	{
	Point3 tempMirrorCenter = mirrorCenter;
	
	//draw gizmo
	if (mirrorAxis == 0)  //x axis
		{
		tempMirrorCenter.x += mirrorOffset;
		point.x = tempMirrorCenter.x-(point.x - tempMirrorCenter.x);
		norm.x *= -1.0f;
		}
	else if (mirrorAxis == 1)  //y axis
		{
		tempMirrorCenter.y += mirrorOffset;
		point.y = tempMirrorCenter.y - (point.y - tempMirrorCenter.y);
		norm.y *= -1.0f;
		}
	else  //z axis
		{
		tempMirrorCenter.z += mirrorOffset;
		point.z = tempMirrorCenter.z - (point.z - tempMirrorCenter.z);
		norm.z *= -1.0f;
		}
	
	}

BOOL  PainterInterface::ResizeStr(IPoint2 mousePos, int &mousePoint)
	{
	static float oldStr = 0.0f;
	static IPoint2 originalPos;
	if (mousePoint == 0)
		{
		oldStr = GetMaxStr();
		originalPos = mousePos;
		inStrChange = TRUE;
		}
	else if (mousePoint == 1)
		{
		float str = oldStr +  (originalPos.y-mousePos.y)/19.0f;
		SetMaxStr(str);
		RedrawNodes();

		}
	else if (mousePoint == 4)
		{
		inStrChange = FALSE;
		SetMaxStr(oldStr);
		inStrChange = FALSE;
		RedrawNodes();

		}
	else inStrChange = FALSE;

	return TRUE;

	}
BOOL  PainterInterface::ResizeRadius(IPoint2 mousePos, int &mousePoint)
	{
	static float oldRadius = 0.0f;
	static IPoint2 originalPos;
	if (mousePoint == 0)
		{
		oldRadius = GetMaxSize();
		originalPos = mousePos;
		inRadiusChange = TRUE;

		}
	else if (mousePoint == 1)
		{
		float radius = oldRadius + (originalPos.y-mousePos.y)/10.0f;
		SetMaxSize(radius);
		RedrawNodes();
		}
	else if (mousePoint == 4)
		{
		
		inRadiusChange = FALSE;
		SetMaxSize(oldRadius);
		RedrawNodes();

		}
	else inRadiusChange = FALSE;
	return TRUE;


	}

BOOL  PainterInterface::GetHitOnCP( IPoint2 mousePos, Point3 &worldHit, Point3 &worldNormal)
	{
	Matrix3 tm;
	int ct = worldNormalList.Count();

	worldNormal = Point3(0.0f,0.0f,0.0f);



	Point2 p((float)mousePos.x + 0.5f,(float)mousePos.y + 0.5f);


	Ray ray;
	Matrix3 itm;
	Point3 pt;
	float t;
	
	int type = GetOffMeshHitType();
	


	if (type == 0)
		{

		if (ct == 0) return FALSE;

		Point3 normal;
	
		normal = worldNormalList[ct-1];
		MatrixFromNormal(normal,tm);
		tm.SetTrans(worldPointList[ct-1]);

		ViewExp *vpt = GetCOREInterface()->GetActiveViewport();
		vpt->MapScreenToWorldRay( p.x, p.y, ray );
		GetCOREInterface()->ReleaseViewport(vpt);

		itm = Inverse( tm );

	// Map the ray into object space of the plane.
		ray.p   = itm * ray.p;
		ray.dir = VectorTransform( itm, ray.dir );
	

	// Intersect with the plane z = 0;
		if(ray.dir.z == 0.0f)
			return FALSE;
//		t = 0.0f;
		else
			t = -ray.p.z/ray.dir.z;
		pt.x = ray.p.x + ray.dir.x * t;
		pt.y = ray.p.y + ray.dir.y * t;
		pt.z = 0.0f;

		worldHit = pt * tm;
		worldNormal = normal;

	
		return TRUE;
		}
	else if (type == 1)  //zdepth
		{
		ViewExp *vpt = GetCOREInterface()->GetActiveViewport();
		vpt->MapScreenToWorldRay( p.x, p.y, ray );
		Matrix3 tm;
		vpt->GetAffineTM(tm);
		GetCOREInterface()->ReleaseViewport(vpt);

		tm = Inverse(tm);

		float zdepth = GetOffMeshHitZDepth();
		
		Point3 vecA,vecB;
		vecA = tm.GetRow(2) * -1.0f;
		vecA = Normalize(vecA);
		vecB = Normalize(ray.dir);

		float angle = acos(DotProd(vecA,vecB));
		float dist =  zdepth/cos(angle);

		worldHit = ray.p + (Normalize(ray.dir) * dist);
		worldNormal = Normalize(ray.dir)*-1.0f;

		return TRUE;
		}
	else if (type == 2)  //pos in space
		{
		ViewExp *vpt = GetCOREInterface()->GetActiveViewport();
		vpt->MapScreenToWorldRay( p.x, p.y, ray );
		Matrix3 tm;
		vpt->GetAffineTM(tm);
		GetCOREInterface()->ReleaseViewport(vpt);

		tm = Inverse(tm);

		Point3 pos = GetOffMeshHitPos();

		Point3 vecA,vecB;
		vecA = tm.GetRow(2) * -1.0f;
		vecA = Normalize(vecA);
		vecB = Normalize(pos - ray.p);

		float angle = acos(DotProd(vecA,vecB));
		float zdist =  cos(angle) * Length(pos-ray.p);


		vecA = tm.GetRow(2) * -1.0f;
		vecA = Normalize(vecA);
		vecB = Normalize(ray.dir);

		angle = acos(DotProd(vecA,vecB));
		float dist =  zdist/cos(angle);

		worldHit = ray.p + (Normalize(ray.dir) * dist);
		worldNormal = Normalize(ray.dir)*-1.0f;

		

		return TRUE;
		}
	return FALSE;
	}

/*
extern void GetConstructionTM( View3D *vpt, Matrix3 &tm );

Point3 View3D::GetPointOnCP(const IPoint2 &ps)
{
	Matrix3 tmConst;
//	GetConstructionTM( tmConst );	// Tom, this uses the active vpt -- screws up others (????) /DB
	GetConstructionTM( this, tmConst );
	return GetPointOnPlane(ps, tmConst);
}

Point3 View3D::GetPointOnPlane(const IPoint2 &ps, const Matrix3 &tm)
{
	return GetPointOnPlane(Point2((float)ps.x + 0.5f,(float)ps.y + 0.5f),tm);
}

Point3 View3D::GetPointOnPlane(const Point2 &ps, const Matrix3 &tm)
{
#ifdef _OSNAP
	OsnapManager *theMan = GetAppScene()->GetOsnapMan(); 
#endif
	Ray ray;
	Matrix3 itm;
	Point3 pt;
	float t;								  
	
	MapScreenToWorldRay( ps.x, ps.y, ray );
	itm = Inverse( tm );

	// Map the ray into object space of the plane.
	ray.p   = itm * ray.p;
	ray.dir = VectorTransform( itm, ray.dir );

	// We're assuming it's going to intersect.
//	assert( ray.dir.z != 0.0 ); -removed 6/14/95 since this happens whenever the const plane is perp to view

#if 0	// What is this doing here?  It fails (improperly) when a box is
		// created at 100000, 10000, 0 !!!   DB 4/1/96
	// If ray can't hit plane, return origin (for now)
	if(SIGN(ray.dir.z) == SIGN(ray.p.z))
		return Point3(0,0,0);
#endif

#ifdef _OSNAP
//	DebugPrint("SIGN, %f\n",ray.dir.z);
	// if the sign is positive then we're projecting back 
	// We might want to do something like warn the user by changing
	//colors.
	if(0)//ray.dir.z > 0.0f)
	{
		Point3 rgb(1.0f,0.0f,0.0f);
		theMan->sethilite(rgb);
	}
#endif

	// Intersect with the plane z = 0;
	if(ray.dir.z == 0.0f)
		t = 0.0f;
	else
		t = -ray.p.z/ray.dir.z;
	pt.x = ray.p.x + ray.dir.x * t;
	pt.y = ray.p.y + ray.dir.y * t;
	pt.z = 0.0f;

	return pt;
}
*/

void PainterInterface::RebuildPointGatherData(BOOL forceRebuild)
	{
		ApplyPreDeterminedGraphs();
		if ((GetPredefinedStrEnable()||GetPredefinedSizeEnable() || forceRebuild) && GetEnablePointGather())
			{
//FIX
			Tab<float> dist;
			dist.SetCount(worldPointList.Count());
			float totalDist = 0.0f;
			dist[0] = 0.0f;
			for (int i =1; i < dist.Count(); i++)
				{
				totalDist += Length(worldPointList[i] - worldPointList[i-1]);
				dist[i] = totalDist;
				}

			pointGather.ClearWeights();
			for (i = 0; i < worldPointList.Count(); i++)
				{
				int index = i;
				if (index >=1)
					{
					pointGather.AddWeightFromSegment(worldPointList[index-1],radiusList[index-1],strList[index-1],
														 worldPointList[index],radiusList[index],strList[index],
														 pCurve,GetAdditiveMode(),FALSE,
														 dist[index-1]
												         );
					if (mirrorEnable)
						pointGather.AddWeightFromSegment(worldPointMirrorList[index-1],radiusList[index-1],strList[index-1],
														 worldPointMirrorList[index],radiusList[index],strList[index],
														 pCurve,GetAdditiveMode(),TRUE,
														 dist[index-1]
												         );
					}
				else if (index == 0)
					{
					pointGather.AddWeightFromSegment(worldPointList[index],radiusList[index],strList[index],
													 worldPointList[index],radiusList[index],strList[index],
													 pCurve,GetAdditiveMode(),FALSE,
													 dist[index]
											         );
					if (mirrorEnable)
						pointGather.AddWeightFromSegment(worldPointMirrorList[index],radiusList[index],strList[index],
														 worldPointMirrorList[index],radiusList[index],strList[index],
														 pCurve,GetAdditiveMode(),TRUE,
														 dist[index]
												         );

					}
				
				}
			}
	}


BOOL  PainterInterface::HitTest(IPoint2 mousePos, int &mousePoint, int flags, BOOL rebuildPointGatherData )
	
	{
	ViewExp *vpt = GetCOREInterface()->GetActiveViewport();

	static int lastX = 0;
	static int lastY = 0;
	Matrix3 tm; 
	vpt->GetAffineTM(tm);

	GraphicsWindow *gw = vpt->getGW();
	int currentX = gw->getWinSizeX();
	int currentY = gw->getWinSizeX();

	GetCOREInterface()->ReleaseViewport(vpt);

	if ((!(viewTm == tm)) || (lastX!=currentX) || (lastY!=currentY))
		{
		UpdateMeshes(FALSE);
		}

	lastX = currentX;
	lastY = currentY;

//need to check to make sure we have a canvas

	Point3 hitPoint(0.0f,0.0f,0.0f);
	Point3 normal(1.0f,0.0f,0.0f);
	Point3 bary(0.0f,0.0f,0.0f);
	int index = 0;
	BOOL shift = flags & MOUSE_SHIFT;
	BOOL ctrl = flags & MOUSE_CTRL;
	BOOL alt = flags & MOUSE_ALT;  
	float pressure = 1.0f;
	float str = 1.0f, radius = 10.0f;
	INode *node = NULL;

	Point3 hitPointLocalMirror(0.0f,0.0f,0.0f);
	Point3 normalLocalMirror(1.0f,0.0f,0.0f);


//first hit
	if (mousePoint == 0)
		{
		inStroke = TRUE;
		UpdateColors();
		if (canvas) 
			canvas->StartStroke();
		else RunFunction(fnStartStrokeHandler);
//			if (scriptCallback)
//			RunCommand( macroCallbackCategory,"StartStroke");

		traceWorldSpace.ZeroCount();
		ZeroTempList();
		ZeroTempMirrorList();
				
		pblock->GetValue(painterinterface_drawtrace,0,drawTrace,FOREVER);
		pblock->GetValue(painterinterface_updateonmouseup,0,updateOnMouseUp,FOREVER);

		float z;
		DWORD findex;
		int nodeIndex = -1;
		Matrix3 toWorld;
		BOOL hitMesh = meshSearchTree.HitQuadTree(mousePos,nodeIndex, findex, localSpaceHit, localSpaceNormal, bary, z, toWorld);

		if (GetEnablePointGather())
			pointGather.ClearWeights();

		BOOL hit = FALSE;
		if (!hitMesh)
			{
			hit = GetHitOnCP(mousePos,worldSpaceHit,worldSpaceNormal);
			if (hit)
				{
				localSpaceHit = Point3(0.0f,0.0f,0.0f);
				localSpaceNormal = Point3(0.0f,0.0f,0.0f);
//				worldSpaceNormal = worldNormalList[worldNormalList.Count()-1];
				}

			}
		else
			{
			hit = hitMesh;
			worldSpaceHit = localSpaceHit*toWorld;					
			worldSpaceNormal = VectorTransform(toWorld, localSpaceNormal);
			}
		

		if (hit)
			{
			INode *node= NULL;
			hitState = hit;
			if ((nodeIndex >= 0) && (nodeIndex < pblock->Count(painterinterface_nodelist)))
				node = pblock->GetINode(painterinterface_nodelist,0,nodeIndex);



			if (mirrorEnable) 
				{
				worldSpaceHitMirror = worldSpaceHit;
				worldSpaceNormalMirror = worldSpaceNormal;

				MirrorPoint(worldSpaceHitMirror, worldSpaceNormalMirror);
				
				hitPointLocalMirror = worldSpaceHitMirror * Inverse(toWorld);
				normalLocalMirror = VectorTransform(Inverse(toWorld), worldSpaceNormalMirror);
	
				}




//put in pressure weights 
			if (!GetPressureEnable())
				fpressure = 1.0f;
			pressure = fpressure;
			GetSizeAndStr(radius, str);

			AppendTempList(hitMesh,mousePos, 
								  worldSpaceHit, worldSpaceNormal,
								  localSpaceHit, localSpaceNormal,										 
								  bary,  findex,
								  shift, ctrl, alt,  
								  radius,str,
								  pressure, node);
			if (mirrorEnable) 
				AppendTempMirrorList( worldSpaceHitMirror, worldSpaceNormalMirror,
									  hitPointLocalMirror, normalLocalMirror);

			if (GetEnablePointGather())
				{
				Tab<float> dist;
				dist.SetCount(worldPointList.Count());
				float totalDist = 0.0f;
				dist[0] = 0.0f;
				for (int i =1; i < dist.Count(); i++)
					{
					totalDist += Length(worldPointList[i] - worldPointList[i-1]);
					dist[i] = totalDist;
					}

				pointGather.AddWeightFromSegment(worldPointList[index],radiusList[index],strList[index],
												 worldPointList[index],radiusList[index],strList[index],
												 pCurve,GetAdditiveMode(),FALSE,dist[index]  );
				if (mirrorEnable)
					pointGather.AddWeightFromSegment(worldPointMirrorList[index],radiusList[index],strList[index],
												 worldPointMirrorList[index],radiusList[index],strList[index],
												 pCurve,GetAdditiveMode(),TRUE,dist[index]  );
				}


			if ( (!updateOnMouseUp) && (canvas))
				canvas->PaintStroke(hitMesh,mousePos, 
								  worldSpaceHit, worldSpaceNormal,
								  localSpaceHit, localSpaceNormal,										 
								  bary,  findex,
								  shift, ctrl, alt,  
								  radius,str,
								  pressure, node,
								  mirrorEnable,
								  worldSpaceHitMirror, worldSpaceNormalMirror,
								  hitPointLocalMirror, normalLocalMirror
								  );
			else if ((!updateOnMouseUp) ) RunFunction(fnPaintStrokeHandler);
//				&& (scriptCallback))
//				RunCommand(macroCallbackCategory,"PaintStroke");

					
			if (node) GetCOREInterface()->NodeInvalidateRect(node);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			}
		hitState = hit;
		}
//mouse move
	else if (mousePoint == 1)
		{
		float z;
		DWORD findex;
		int nodeIndex=-1;
		Matrix3 toWorld;
		BOOL hitMesh = meshSearchTree.HitQuadTree(mousePos,nodeIndex, findex, localSpaceHit, localSpaceNormal, bary, z, toWorld);

		BOOL hit = FALSE;
		if (!hitMesh)
			{
			hit = GetHitOnCP(mousePos,worldSpaceHit,worldSpaceNormal);
			if (hit)
				{
				localSpaceHit = Point3(0.0f,0.0f,0.0f);
				localSpaceNormal = Point3(0.0f,0.0f,0.0f);
//				worldSpaceNormal = worldNormalList[worldNormalList.Count()-1];
				}
			}
		else
			{
			hit = hitMesh;
			worldSpaceHit = localSpaceHit*toWorld;					
			worldSpaceNormal = VectorTransform(toWorld, localSpaceNormal);
			}


		if (hit)
			{

			mouseFreeMoveCount++;
			BringNodesToFront();

			INode *node = NULL;
			hitState = hit;
			
			if ((nodeIndex >= 0) && (nodeIndex < pblock->Count(painterinterface_nodelist)))
				node = pblock->GetINode(painterinterface_nodelist,0,nodeIndex);
//			if (nodeIndex >= 0)
//				{
//				node = pblock->GetINode(painterinterface_nodelist,0,nodeIndex);
	

			if (mirrorEnable) 
				{
				worldSpaceHitMirror = worldSpaceHit;
				worldSpaceNormalMirror = worldSpaceNormal;

				MirrorPoint(worldSpaceHitMirror, worldSpaceNormalMirror);
					
				hitPointLocalMirror = worldSpaceHitMirror * Inverse(toWorld);
				normalLocalMirror = VectorTransform(Inverse(toWorld), worldSpaceNormalMirror);
				}
	
			if (drawTrace)
				{
				traceWorldSpace.Append(1,&worldSpaceHit,100);
				if (mirrorEnable) 				
					traceWorldSpaceMirror.Append(1,&worldSpaceHitMirror,100);
				}
		
//put in pressure weights 
			if (!GetPressureEnable())
				fpressure = 1.0f;

			pressure = fpressure;
			GetSizeAndStr(radius, str);
	
//						if (updateOnMouseUp)
			AppendTempList(hitMesh,mousePos, 
							  worldSpaceHit, worldSpaceNormal,
							  localSpaceHit, localSpaceNormal,										 
							  bary,  findex,
							  shift, ctrl, alt,  
							  radius,str,
							  pressure, node);

			if (mirrorEnable) 
				AppendTempMirrorList( worldSpaceHitMirror, worldSpaceNormalMirror,
									  hitPointLocalMirror, normalLocalMirror);
		
//			if (!updateOnMouseUp)
//				ApplyPreDeterminedGraphs();

			if ( (!updateOnMouseUp) && rebuildPointGatherData)
				RebuildPointGatherData();
			

			if (GetEnablePointGather() && (GetPredefinedStrEnable()==FALSE) && (GetPredefinedSizeEnable()==FALSE) )
				{
				Tab<float> dist;
				dist.SetCount(worldPointList.Count());
				float totalDist = 0.0f;
				dist[0] = 0.0f;
				for (int i =1; i < dist.Count(); i++)
					{
					totalDist += Length(worldPointList[i] - worldPointList[i-1]);
					dist[i] = totalDist;
					}

				int index = worldPointList.Count()-1;
				if (index >=1)
					{
					pointGather.AddWeightFromSegment(worldPointList[index-1],radiusList[index-1],strList[index-1],
														 worldPointList[index],radiusList[index],strList[index],
														 pCurve,GetAdditiveMode(),FALSE,
														 dist[index-1]
												         );
					if (mirrorEnable)
						pointGather.AddWeightFromSegment(worldPointMirrorList[index-1],radiusList[index-1],strList[index-1],
														 worldPointMirrorList[index],radiusList[index],strList[index],
														 pCurve,GetAdditiveMode(),TRUE,
														 dist[index-1]
												         );
					}
				else if (index == 0)
					{
					pointGather.AddWeightFromSegment(worldPointList[index],radiusList[index],strList[index],
													 worldPointList[index],radiusList[index],strList[index],
													 pCurve,GetAdditiveMode(),FALSE,
													 dist[index]
											         );
					if (mirrorEnable)
						pointGather.AddWeightFromSegment(worldPointMirrorList[index],radiusList[index],strList[index],
														 worldPointMirrorList[index],radiusList[index],strList[index],
														 pCurve,GetAdditiveMode(),TRUE,
														 dist[index]
												         );

					}

				}


			if ( (!updateOnMouseUp) && (canvas))
				{
				canvas->PaintStroke(hitMesh,mousePos, 
							  worldSpaceHit, worldSpaceNormal,
							  localSpaceHit, localSpaceNormal,										 
							  bary,  findex,
							  shift, ctrl, alt, 
							  radius, str,
							  pressure, node,
							  mirrorEnable,
							  worldSpaceHitMirror, worldSpaceNormalMirror,
							  hitPointLocalMirror, normalLocalMirror);		
				}
			else if ((!updateOnMouseUp) ) RunFunction(fnPaintStrokeHandler);
//				&& (scriptCallback))
//			RunCommand(macroCallbackCategory,"PaintStroke" );

										  
			if (node) GetCOREInterface()->NodeInvalidateRect(node);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			}
		
		hitState = hit;
		}
//mouse up
	else if (mousePoint == 2)
		{
		inStroke = FALSE;
//		ApplyPreDeterminedGraphs();

		RebuildPointGatherData();

		if ( (updateOnMouseUp) && (canvas))
			{
			if (mirrorEnable)
				canvas->EndStroke(mousePosList.Count(),hitList.Addr(0),
								mousePosList.Addr(0), 
								worldPointList.Addr(0), worldNormalList.Addr(0),
								localPointList.Addr(0), localNormalList.Addr(0),										 
								baryList.Addr(0),  indexList.Addr(0),
								shiftList.Addr(0), ctrlList.Addr(0), altList.Addr(0),  
								radiusList.Addr(0),strList.Addr(0),
								pressureList.Addr(0), nodeList.Addr(0),
								mirrorEnable,
								worldPointMirrorList.Addr(0), worldNormalMirrorList.Addr(0),
								localPointMirrorList.Addr(0), localNormalMirrorList.Addr(0)	);
				else canvas->EndStroke(mousePosList.Count(),hitList.Addr(0),
								mousePosList.Addr(0), 
								worldPointList.Addr(0), worldNormalList.Addr(0),
								localPointList.Addr(0), localNormalList.Addr(0),										 
								baryList.Addr(0),  indexList.Addr(0),
								shiftList.Addr(0), ctrlList.Addr(0), altList.Addr(0),  
								radiusList.Addr(0),strList.Addr(0),
								pressureList.Addr(0), nodeList.Addr(0),
								mirrorEnable,
								NULL, NULL,
								NULL, NULL	);
			}
		else if (canvas) 
				{
				canvas->EndStroke();
				}
		else RunFunction(fnEndStrokeHandler);
//			if (scriptCallback)
//			RunCommand(macroCallbackCategory,"EndStroke");

		traceWorldSpace.ZeroCount();
		traceWorldSpaceMirror.ZeroCount();

		}
//cancel
	else if (mousePoint == 4)
		{
		inStroke = FALSE;
		if (canvas) 
			canvas->CancelStroke();
		else RunFunction(fnCancelStrokeHandler);
//		if (scriptCallback)
//			RunCommand(macroCallbackCategory,"CancelStroke");
		traceWorldSpace.ZeroCount();
		ZeroTempList();
		ZeroTempMirrorList();
		traceWorldSpace.ZeroCount();
		traceWorldSpaceMirror.ZeroCount();

		}
//free move
	else if (mousePoint == 5)
		{
		float z;
		DWORD findex;
		int nodeIndex = -1;
		Matrix3 toWorld;
		BOOL hit = meshSearchTree.HitQuadTree(mousePos,nodeIndex, findex, localSpaceHit, localSpaceNormal, bary, z,toWorld);
				

		if ( (hit) || (hitState && (!hit)))
			{

			mouseFreeMoveCount++;
			BringNodesToFront();

			worldSpaceHit = localSpaceHit*toWorld;					
			worldSpaceNormal = VectorTransform(toWorld, localSpaceNormal);

			if (mirrorEnable) 
				{
				worldSpaceHitMirror = worldSpaceHit;
				worldSpaceNormalMirror = worldSpaceNormal;

				MirrorPoint(worldSpaceHitMirror, worldSpaceNormalMirror);
				
				hitPointLocalMirror = worldSpaceHitMirror * Inverse(toWorld);
				normalLocalMirror = VectorTransform(Inverse(toWorld), worldSpaceNormalMirror);
	
				}

			INode *node;
			hitState = hit;
			if (nodeIndex >=0)
				{
				node = pblock->GetINode(painterinterface_nodelist,0,nodeIndex);
				if (node) GetCOREInterface()->NodeInvalidateRect(node);
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				}
			}
		hitState = hit;
				
		}
	
	return TRUE;
	}

float PainterInterface::GetStrFromPoint(Point3 point)
	{
//Need to brute force this since we font know anything about the incoming point
//this just does a a bounds test agains the segments
//so devs should trim the obvious points out first before calling this

//loop through all out segments	
	
	int seg = -1;
	float dist = 0.0f;
	float finalU = 0.0f;

	for (int i =1; i < worldPointList.Count(); i++)
		{
	//do a bounds check against each segment
		Box3 segBounds;
		segBounds.Init();
		Point3 l1 = worldPointList[i-1];
		Point3 l2 = worldPointList[i];
		segBounds += l1;
		segBounds += l2;
		if (radiusList[i] > radiusList[i-1])
			segBounds.EnlargeBy(radiusList[i]);
		else segBounds.EnlargeBy(radiusList[i-1]);
	//check if in our bounds
		if (segBounds.Contains(point))
			{
			float u = 0.0f;
			float currentDist = pointGather.LineToPoint(point, l1, l2, u);
			if ((seg == -1) || (currentDist < dist ))
				{
				dist = currentDist;
				finalU = u;
				seg = i-1;
				}
			}
		}
	if (seg == -1)
		return 0.0f;
	else
		{
		float str1 = 0.0f;
		float str2 = 0.0f;
		str1 = strList[seg];
		str2 = strList[seg+1];
		float str  = str1 + (str2-str1) * finalU;
		return str;
		}

	}


int PainterInterface::GetTreeDepth()
	{
	int quadDepth;
	pblock->GetValue(painterinterface_quaddepth,0,quadDepth,FOREVER);
	return quadDepth;
	}

void PainterInterface::SetTreeDepth(int depth)
	{
	if (depth < 2) depth = 2;
	if (depth > 10) depth = 10;
	pblock->SetValue(painterinterface_quaddepth,0,depth);
	UpdateMeshes(FALSE);
	UpdateControls();

	}


BOOL PainterInterface::GetUpdateOnMouseUp()
	{
	
	pblock->GetValue(painterinterface_updateonmouseup,0,updateOnMouseUp,FOREVER);
	return updateOnMouseUp;

	}
void PainterInterface::SetUpdateOnMouseUp(BOOL update)
	{
	pblock->SetValue(painterinterface_updateonmouseup,0,update);
	UpdateControls();

	}

int PainterInterface::GetLagRate()
	{
	int rate;
	pblock->GetValue(painterinterface_lagrate,0,rate,FOREVER);
	return rate;

	}
void PainterInterface::SetLagRate(int rate)
	{
	if (rate < 0) rate = 0;
	pblock->SetValue(painterinterface_lagrate,0,rate);
	UpdateControls();

	}


float PainterInterface::GetNormalScale()
	{
	float scale;
	pblock->GetValue(painterinterface_normalscale,0,scale,FOREVER);
	return scale;

	}
void PainterInterface::SetNormalScale(float scale)
	{
	pblock->SetValue(painterinterface_normalscale,0,scale);
	UpdateControls();
	}


float PainterInterface::GetMarker()
	{
	float pos;
	pblock->GetValue(painterinterface_marker,0,pos,FOREVER);
	return pos;

	}
void PainterInterface::SetMarker(float pos)
	{
	pblock->SetValue(painterinterface_marker,0,pos);
	UpdateControls();
	}

BOOL PainterInterface::GetMarkerEnable()
	{
	BOOL enable;
	pblock->GetValue(painterinterface_markerenable,0,enable,FOREVER);
	return enable;

	}
void PainterInterface::SetMarkerEnable(BOOL enable)
	{
	pblock->SetValue(painterinterface_markerenable,0,enable);
	UpdateControls();
	}


void PainterInterface::ZeroTempList()
	{
	mousePosList.ZeroCount();
	hitList.ZeroCount();
	worldPointList.ZeroCount();
	worldNormalList.ZeroCount();
	localPointList.ZeroCount();
	localNormalList.ZeroCount();
	baryList.ZeroCount();
	indexList.ZeroCount();
	shiftList.ZeroCount();
	ctrlList.ZeroCount();
	altList.ZeroCount();
	radiusList.ZeroCount();
	strList.ZeroCount();
	pressureList.ZeroCount();
	nodeList.ZeroCount();
	timeList.ZeroCount();
	}

void PainterInterface::ZeroTempMirrorList()
	{
	worldPointMirrorList.ZeroCount();
	worldNormalMirrorList.ZeroCount();
	localPointMirrorList.ZeroCount();
	localNormalMirrorList.ZeroCount();
	}

void PainterInterface::AppendTempList(BOOL hit, IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node)
	{
	radius = radius * 0.5f;
	hitList.Append(1,&hit,100);
	mousePosList.Append(1,&mousePos,100);
	worldPointList.Append(1,&worldPoint,100);
	worldNormalList.Append(1,&worldNormal,100);
	localPointList.Append(1,&localPoint,100);
	localNormalList.Append(1,&localNormal,100);
	baryList.Append(1,&bary,100);
	indexList.Append(1,&index,100);
	shiftList.Append(1,&shift,100);
	ctrlList.Append(1,&ctrl,100);
	altList.Append(1,&alt,100);
	radiusList.Append(1,&radius,100);
	strList.Append(1,&str,100);
	pressureList.Append(1,&pressure,100);
	nodeList.Append(1,&node,100);

	int time = GetTickCount();
	if (timeList.Count() == 0)
		startTime = time;
	time = time - startTime;

	timeList.Append(1,&time,100);

	}

void PainterInterface::AppendTempMirrorList(  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal)
	{
	worldPointMirrorList.Append(1,&worldPoint,100);
	worldNormalMirrorList.Append(1,&worldNormal,100);
	localPointMirrorList.Append(1,&localPoint,100);
	localNormalMirrorList.Append(1,&localNormal,100);
	}

int  PainterInterface::GetHitCount()
	{
	return worldPointList.Count();
	}



float PainterInterface::GetMinStr()
	{
	pblock->GetValue(painterinterface_minstr,0,minStr,FOREVER);
	return minStr;
	}
void  PainterInterface::SetMinStr(float str)
	{
	pblock->SetValue(painterinterface_minstr,0,str);
	minStr = str;
	UpdateControls();
	}

float PainterInterface::GetMaxStr()
	{
	pblock->GetValue(painterinterface_maxstr,0,maxStr,FOREVER);
	return maxStr;
	}
void  PainterInterface::SetMaxStr(float str)
	{
	pblock->SetValue(painterinterface_maxstr,0,str);
	maxStr = str;
	UpdateControls();

	}

float PainterInterface::GetMinSize()
	{
	pblock->GetValue(painterinterface_minsize,0,minSize,FOREVER);
	return minSize;
	}
void  PainterInterface::SetMinSize(float size)
	{
	pblock->SetValue(painterinterface_minsize,0,size);
	minSize = size;
	UpdateControls();

	}

float PainterInterface::GetMaxSize()
	{
	pblock->GetValue(painterinterface_maxsize,0,maxSize,FOREVER);
	return maxSize;
	}
void  PainterInterface::SetMaxSize(float size)
	{
	pblock->SetValue(painterinterface_maxsize,0,size);
	maxSize = size;
	UpdateControls();

	}

BOOL  PainterInterface::GetDrawRing()
	{
	BOOL draw;
	pblock->GetValue(painterinterface_drawring,0,draw,FOREVER);
	return draw;
	}
void  PainterInterface::SetDrawRing(BOOL draw)
	{
	pblock->SetValue(painterinterface_drawring,0,draw);
	UpdateControls();
	}

BOOL  PainterInterface::GetDrawNormal()
	{
	BOOL draw;
	pblock->GetValue(painterinterface_drawnormal,0,draw,FOREVER);
	return draw;
	}
void  PainterInterface::SetDrawNormal(BOOL draw)
	{
	pblock->SetValue(painterinterface_drawnormal,0,draw);
	UpdateControls();
	}

BOOL  PainterInterface::GetDrawTrace()
	{	
	BOOL draw;
	pblock->GetValue(painterinterface_drawtrace,0,draw,FOREVER);
	return draw;

	}
void  PainterInterface::SetDrawTrace(BOOL draw)
	{
	pblock->SetValue(painterinterface_drawtrace,0,draw);
	UpdateControls();

	}

BOOL  PainterInterface::GetAdditiveMode()
	{
	pblock->GetValue(painterinterface_additivemode,0,additiveMode,FOREVER);
	return additiveMode;
	}
void  PainterInterface::SetAdditiveMode(BOOL enable)
	{
	additiveMode = enable;

	pblock->SetValue(painterinterface_additivemode,0,enable);

//	UpdateControls();

//	if (curveCtl) curveCtl->SetActive(enable);
	}

BOOL  PainterInterface::GetPressureEnable()	
	{
	pblock->GetValue(painterinterface_pressureenable,0,pressureEnable,FOREVER);
	return pressureEnable;
	}

void  PainterInterface::SetPressureEnable(BOOL enable)
	{
	pressureEnable = enable;
	pblock->SetValue(painterinterface_pressureenable,0,enable);
//	if (enable)
	InitTablet(enable);

	UpdateControls();

//    HWND hPressureAffects = GetDlgItem(painterOptionsWindow,IDC_PRESSURE_AFFECTS_COMBO);
//	EnableWindow(hPressureAffects,enable);

	}

BOOL  PainterInterface::GetPressureAffects()
	{
	pblock->GetValue(painterinterface_pressureaffects,0,pressureAffects,FOREVER);
	return pressureAffects;
	}

void  PainterInterface::SetPressureAffects(int affect)
	{
	if (affect < 0) affect = 0;
	if (affect > 3) affect = 3;

	pressureAffects = affect;
	pblock->SetValue(painterinterface_pressureaffects,0,affect);

	UpdateControls();


	}

void PainterInterface::GetSizeAndStr(float &size, float &str)
	{
	str = maxStr;
	size = maxSize;
	if ((GetPressureAffects()==PRESSURE_AFFECTS_STR) && (hWinTabDLL))
		{
		str = minStr + (maxStr - minStr) * fpressure;
		}
	else if ((GetPressureAffects()==PRESSURE_AFFECTS_SIZE) && (hWinTabDLL))
		{
		size = minSize + (maxSize - minSize) * fpressure;
		}
	else if ((GetPressureAffects()==PRESSURE_AFFECTS_BOTH) && (hWinTabDLL))
		{
		size = minSize + (maxSize - minSize) * fpressure;
		str = minStr + (maxStr - minStr) * fpressure;
		}	
	}
void PainterInterface::GetSizeAndStr(int ct, float *size, float *str)
	{
	for (int i =0; i < ct; i++)
		{
		}
	}




BOOL  PainterInterface::GetPredefinedStrEnable()
	{
	pblock->GetValue(painterinterface_predefinedstrenable,0,predefinedStrEnable,FOREVER);
	return predefinedStrEnable;
	}
void  PainterInterface::SetPredefinedStrEnable(BOOL enable)
	{
	predefinedStrEnable = enable;
	pblock->SetValue(painterinterface_predefinedstrenable,0,predefinedStrEnable);

	UpdateControls();
//    HWND hPressure = GetDlgItem(painterOptionsWindow,IDC_PREDEFINEDSTR_BUTTON);
//	EnableWindow(hPressure,enable);
	}

BOOL  PainterInterface::GetPredefinedSizeEnable()
	{
	pblock->GetValue(painterinterface_predefinedsizeenable,0,predefinedSizeEnable,FOREVER);
	return predefinedSizeEnable;

	}
void  PainterInterface::SetPredefinedSizeEnable(BOOL enable)	
	{
	predefinedSizeEnable = enable;
	pblock->SetValue(painterinterface_predefinedsizeenable,0,predefinedSizeEnable);

	UpdateControls();

//    HWND hPressure = GetDlgItem(painterOptionsWindow,IDC_PREDEFINEDSIZE_BUTTON);
//	EnableWindow(hPressure,enable);


	}


BOOL  PainterInterface::GetMirrorEnable()
	{
	pblock->GetValue(painterinterface_mirrorenable,0,mirrorEnable,FOREVER);
	return mirrorEnable;
	}
void  PainterInterface::SetMirrorEnable(BOOL enable)
	{
	mirrorEnable = enable;
	pblock->SetValue(painterinterface_mirrorenable,0,mirrorEnable);

	UpdateControls();
	RedrawNodes();
	}

int   PainterInterface::GetMirrorAxis()
	{
	pblock->GetValue(painterinterface_mirroraxis,0,mirrorAxis,FOREVER);
	return mirrorAxis;

	}
void  PainterInterface::SetMirrorAxis(int axis)
	{
	if (axis < 0) axis = 0;
	if (axis > 2) axis = 2;
	mirrorAxis = axis;
	pblock->SetValue(painterinterface_mirroraxis,0,mirrorAxis);

	UpdateControls();
	RedrawNodes();

	}

Point3 PainterInterface::GetMirrorPlaneCenter()
	{
	return mirrorCenter;
	}

float PainterInterface::GetMirrorOffset()
	{
	pblock->GetValue(painterinterface_mirroroffset,0,mirrorOffset,FOREVER);
	return mirrorOffset;
	}

void  PainterInterface::SetMirrorOffset(float offset)
	{
	mirrorOffset = offset;
	pblock->SetValue(painterinterface_mirroroffset,0,mirrorOffset);

	UpdateControls();
	RedrawNodes();

	}

float PainterInterface::GetMirrorGizmoSize()
	{
	pblock->GetValue(painterinterface_mirrorgizmosize,0,mirrorGizmoSize,FOREVER);
	return mirrorGizmoSize;

	}
void  PainterInterface::SetMirrorGizmoSize(float size)
	{
	mirrorGizmoSize = size;
	pblock->SetValue(painterinterface_mirrorgizmosize,0,mirrorGizmoSize);

	UpdateControls();
	RedrawNodes();

	}

BOOL  PainterInterface::GetEnablePointGather()
	{
	pblock->GetValue(painterinterface_enablepointgather,0,enablePointGather,FOREVER);


	return enablePointGather;	
	}
void  PainterInterface::SetEnablePointGather(BOOL enable)
	{
	enablePointGather = enable;
	pblock->SetValue(painterinterface_enablepointgather,0,enablePointGather);
//	if (enablePointGather)
//		UpdateMeshes(TRUE);
	}

BOOL  PainterInterface::GetBuildNormalData()
	{
	BOOL enable;
	pblock->GetValue(painterinterface_buildvnormal,0,enable,FOREVER);
	return enable;

	}
void  PainterInterface::SetBuildNormalData(BOOL enable)
	{
	pblock->SetValue(painterinterface_buildvnormal,0,enable);
//	if (enable)
//		UpdateMeshes(TRUE);
	}


void PainterInterface::ApplyPreDeterminedGraphs()
	{

	float totalDist = 0;
	Tab<float> dist;
	
	if ((GetPredefinedStrEnable()||GetPredefinedSizeEnable()) && (worldPointList.Count()> 0))
		{
		dist.SetCount(worldPointList.Count());
		dist[0] = 0.0f;
		for (int i = 1; i < worldPointList.Count(); i++)
			{
			float d = Length(worldPointList[i] - worldPointList[i-1]);
			dist[i] = dist[i-1]+d;
			totalDist += d;
			}
		}

	float mstr,mrad;
	pblock->GetValue(painterinterface_maxstr,0,mstr,FOREVER);
	pblock->GetValue(painterinterface_maxsize,0,mrad,FOREVER);

	if (GetPredefinedStrEnable() && (worldPointList.Count()> 0))
		{
		
		if (curvePredefinedStrCtl)
			{
			
			pPredefinedStrCurve = curvePredefinedStrCtl->GetControlCurve(0);
			for (int i = 0; i < strList.Count(); i++)
				{
				float per = dist[i]/totalDist;
				strList[i] = pPredefinedStrCurve->GetValue(0,per) * mstr;
				}
			}

		}
	if (GetPredefinedSizeEnable() && (worldPointList.Count()> 0))
		{
		if (curvePredefinedSizeCtl)
			{
			
			pPredefinedSizeCurve = curvePredefinedSizeCtl->GetControlCurve(0);
			for (int i = 0; i < radiusList.Count(); i++)
				{
				float per = dist[i]/totalDist;
				radiusList[i] = pPredefinedSizeCurve->GetValue(0,per) * mrad;
				}
			}
		}
	}
float *PainterInterface::GetPredefineStrStrokeData(int &ct)
	{
	ct = 0;
	if (!GetPredefinedStrEnable())  return NULL;
	ct = strList.Count();
	return strList.Addr(0);
	}
float *PainterInterface::GetPredefineSizeStrokeData(int &ct)
	{
	ct = 0;
	if (!GetPredefinedSizeEnable())  return NULL;
	ct = radiusList.Count();
	return radiusList.Addr(0);
	}
/*
void PainterInterface::GetPredefineStrokeData(int &ct, float **radius, float **str)
	{
	
	str = NULL;
	if (GetPredefinedStrEnable())
		*str = strList.Addr(0);
	if (GetPredefinedSizeEnable())
		*radius = radiusList.Addr(0);

	}
*/
BOOL PainterInterface::GetIsHit(int tabindex)
	{
	if ( (tabindex < 0) || (tabindex >= worldPointList.Count()) )
		tabindex = worldPointList.Count()-1;
	if (tabindex < 0) return FALSE;

	return hitList[tabindex];
	}

void PainterInterface::GetHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld,
						 float &radius, float &str, int tabindex)
	{
	if ( (tabindex < 0) || (tabindex >= worldPointList.Count()) )
		tabindex = worldPointList.Count()-1;
	if (tabindex < 0) return;

	
	hitPointWorld = worldPointList[tabindex];
	hitNormalWorld = worldNormalList[tabindex];
	hitPointLocal = localPointList[tabindex];
	hitNormalLocal =  localNormalList[tabindex];
	radius =  radiusList[tabindex];
	str =  strList[tabindex];

	}

void PainterInterface::GetHitFaceData(Point3 &bary, int &index, INode *node, int tabindex)
	{
	if ( (tabindex < 0) || (tabindex >= worldPointList.Count()) )
		tabindex = worldPointList.Count()-1;
	if (tabindex < 0) return;

	bary = baryList[tabindex];
	index = indexList[tabindex];
	node = nodeList[tabindex];

	}

void PainterInterface::GetHitPressureData(BOOL &shift, BOOL &ctrl, BOOL &alt, float &pressure , int tabindex)
	{
	if ( (tabindex < 0) || (tabindex >= worldPointList.Count()) )
		tabindex = worldPointList.Count()-1;
	if (tabindex < 0) return;

	shift = shiftList[tabindex];
	ctrl = ctrlList[tabindex];
	alt = altList[tabindex];
	pressure = pressureList[tabindex];

	}

void PainterInterface::GetMirrorHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld, int tabindex)
	{
	if ( (tabindex < 0) || (tabindex >= worldPointMirrorList.Count()) )
		tabindex = worldPointMirrorList.Count()-1;
	if (tabindex < 0) return;

	
	hitPointWorld = worldPointMirrorList[tabindex];
	hitNormalWorld = worldNormalMirrorList[tabindex];
	hitPointLocal = localPointMirrorList[tabindex];
	hitNormalLocal =  localNormalMirrorList[tabindex];

	}


BitArray *PainterInterface::GetPointGatherHitVerts(INode *incNode)
	{
	int nodeCount = pblock->Count(painterinterface_nodelist);
	for (int i =0; i < nodeCount; i++)
		{
		INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
		if (node==incNode)
			return pointGather.GetHits(i);
		}
	return NULL;

	}


void PainterInterface::BringNodesToFront()
	{
	if (mouseFreeMoveCount> 5)
		{
		int nodeCount = pblock->Count(painterinterface_nodelist);
		if(nodeCount == 0)
			return;

		for (int i = 0; i < nodeCount; i++)
			{
			INode *node = pblock->GetINode(painterinterface_nodelist,0,i);
			if (node) node->FlagForeground(GetCOREInterface()->GetTime());
			}
		mouseFreeMoveCount =  0;
		}
	}


ICurve *PainterInterface::GetFalloffGraph()

	{
	if (curveCtl == NULL)
		{
		ReferenceTarget *ref;
		pblock->GetValue(painterinterface_falloffgraph,0,ref, FOREVER);
		if (ref)
			{
			curveCtl = (ICurveCtl *) ref;
			}	
		}

	if (curveCtl)
		pCurve = curveCtl->GetControlCurve(0);
	else pCurve = NULL;

	return pCurve;

	}

ICurve *PainterInterface::GetPredefineSizeStrokeGraph()
	{
	if (curvePredefinedStrCtl == NULL)
		{
		ReferenceTarget *ref;
		pblock->GetValue(painterinterface_predefinedstrgraph,0,ref, FOREVER);
		if (ref)
			{
			curvePredefinedStrCtl = (ICurveCtl *) ref;
			}	
		}

	if (curvePredefinedStrCtl)
		pPredefinedStrCurve = curvePredefinedStrCtl->GetControlCurve(0);
	else pPredefinedStrCurve = NULL;

	return pPredefinedStrCurve;

	}
ICurve *PainterInterface::GetPredefineStrStrokeGraph()
	{
	if (curvePredefinedSizeCtl == NULL)
		{
		ReferenceTarget *ref;
		pblock->GetValue(painterinterface_predefinedsizegraph,0,ref, FOREVER);
		if (ref)
			{
			curvePredefinedSizeCtl = (ICurveCtl *) ref;
			}	
		}

	if (curvePredefinedSizeCtl)
		pPredefinedSizeCurve = curvePredefinedSizeCtl->GetControlCurve(0);
	else pPredefinedSizeCurve = NULL;

	return pPredefinedStrCurve;


	}



int PainterInterface::GetOffMeshHitType()
{
	int offMeshHitType;
	pblock->GetValue(painterinterface_offmeshhittype,0,offMeshHitType,FOREVER);


	return offMeshHitType;	
}
void PainterInterface::SetOffMeshHitType(int type)
{
		pblock->SetValue(painterinterface_offmeshhittype,0,type);
}

float PainterInterface::GetOffMeshHitZDepth()
{
	float offMeshHitZDepth;
	pblock->GetValue(painterinterface_offmeshhitzdepth,0,offMeshHitZDepth,FOREVER);


	return offMeshHitZDepth;	
}
void PainterInterface::SetOffMeshHitZDepth(float depth)
{
		pblock->SetValue(painterinterface_offmeshhitzdepth,0,depth);
}

Point3 PainterInterface::GetOffMeshHitPos()
{
	Point3 offMeshHitPos;
	pblock->GetValue(painterinterface_offmeshhitpos,0,offMeshHitPos,FOREVER);


	return offMeshHitPos;	

}
void PainterInterface::SetOffMeshHitPos(Point3 pos)
{
	pblock->SetValue(painterinterface_offmeshhitpos,0,pos);
}

void PainterInterface::ScriptFunctions(Value* startStroke,Value* paintStroke,Value* endStroke,Value* cancelStroke,Value* systemEnd)

{
  	fnStartStrokeHandler = startStroke;
  	fnPaintStrokeHandler = paintStroke;
  	fnEndStrokeHandler = endStroke;
  	fnCancelStrokeHandler = cancelStroke;
  	fnSystemEndHandler = systemEnd;

}
