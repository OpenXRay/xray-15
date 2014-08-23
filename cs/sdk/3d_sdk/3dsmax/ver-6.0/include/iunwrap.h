//-------------------------------------------------------------
// Access to UVW Unwrap
//


#ifndef __IUNWRAP__H
#define __IUNWRAP__H

#include "iFnPub.h"


#define UNWRAP_CLASSID	Class_ID(0x02df2e3a,0x72ba4e1f)

// Flags
#define CONTROL_FIT			(1<<0)
#define CONTROL_CENTER		(1<<1)
#define CONTROL_ASPECT		(1<<2)
#define CONTROL_UNIFORM		(1<<3)
#define CONTROL_HOLD		(1<<4)
#define CONTROL_INIT		(1<<5)
#define CONTROL_OP			(CONTROL_FIT|CONTROL_CENTER|CONTROL_ASPECT|CONTROL_UNIFORM)
#define CONTROL_INITPARAMS	(1<<10)

#define IS_MESH		1
#define IS_PATCH	2
#define IS_NURBS	3
#define IS_MNMESH	4




#define FLAG_DEAD		1
#define FLAG_HIDDEN		2
#define FLAG_FROZEN		4
//#define FLAG_QUAD		8
#define FLAG_SELECTED	16
#define FLAG_CURVEDMAPPING	32
#define FLAG_INTERIOR	64
#define FLAG_WEIGHTMODIFIED	128
#define FLAG_HIDDENEDGEA	256
#define FLAG_HIDDENEDGEB	512
#define FLAG_HIDDENEDGEC	1024

#define ID_TOOL_SELECT	0x0001
#define ID_TOOL_MOVEPIVOT	0x0002
#define ID_TOOL_MOVE	0x0100
#define ID_TOOL_ROTATE	0x0110
#define ID_TOOL_SCALE	0x0120
#define ID_TOOL_PAN		0x0130
#define ID_TOOL_ZOOM    0x0140
#define ID_TOOL_PICKMAP	0x0160
#define ID_TOOL_ZOOMREG 0x0170
#define ID_TOOL_UVW		0x0200
#define ID_TOOL_PROP	0x0210 
#define ID_TOOL_SHOWMAP	0x0220
#define ID_TOOL_UPDATE	0x0230
#define ID_TOOL_ZOOMEXT	0x0240
#define ID_TOOL_BREAK	0x0250
#define ID_TOOL_WELD	0x0260
#define ID_TOOL_WELD_SEL 0x0270
#define ID_TOOL_HIDE	 0x0280
#define ID_TOOL_UNHIDE	 0x0290
#define ID_TOOL_FREEZE	 0x0300
#define ID_TOOL_UNFREEZE	 0x0310
#define ID_TOOL_TEXTURE_COMBO 0x0320
#define ID_TOOL_SNAP 0x0330
#define ID_TOOL_LOCKSELECTED 0x0340
#define ID_TOOL_MIRROR 0x0350
#define ID_TOOL_FILTER_SELECTEDFACES 0x0360
#define ID_TOOL_FILTER_MATID 0x0370
#define ID_TOOL_INCSELECTED 0x0380
#define ID_TOOL_FALLOFF 0x0390
#define ID_TOOL_FALLOFF_SPACE 0x0400
#define ID_TOOL_FLIP 0x0410
#define ID_TOOL_DECSELECTED 0x0420




#define FILL_MODE_OFF		1
#define FILL_MODE_SOLID		2
#define FILL_MODE_BDIAGONAL	3
#define FILL_MODE_CROSS		4
#define FILL_MODE_DIAGCROSS	5
#define FILL_MODE_FDIAGONAL	6
#define FILL_MODE_HORIZONAL	7
#define FILL_MODE_VERTICAL	8


#define SKETCH_SELPICK		1
#define SKETCH_SELDRAG		2
#define SKETCH_SELCURRENT	3
#define SKETCH_DRAWMODE		4
#define SKETCH_APPLYMODE	5

#define SKETCH_FREEFORM		1
#define SKETCH_LINE			2
#define SKETCH_BOX			3
#define SKETCH_CIRCLE		4




class IUnwrapMod;

//***************************************************************
//Function Publishing System stuff   
//****************************************************************
#define UNWRAP_CLASSID	Class_ID(0x02df2e3a,0x72ba4e1f)
#define UNWRAP_INTERFACE Interface_ID(0x53b3409b, 0x18ff7ab8)
#define UNWRAP_INTERFACE2 Interface_ID(0x53b3409b, 0x18ff7ab9)
//5.1.05
#define UNWRAP_INTERFACE3 Interface_ID(0x53b3409b, 0x18ff7ac0)



#define GetIUnwrapInterface(cd) \
			(IUnwrapMod *)(cd)->GetInterface(UNWRAP_INTERFACE)

enum {  unwrap_planarmap,unwrap_save,unwrap_load, unwrap_reset, unwrap_edit,
		unwrap_setMapChannel,unwrap_getMapChannel,
		unwrap_setProjectionType,unwrap_getProjectionType,
		unwrap_setVC,unwrap_getVC,
		unwrap_move,unwrap_moveh,unwrap_movev,
		unwrap_rotate,
		unwrap_scale,unwrap_scaleh,unwrap_scalev,
		unwrap_mirrorh,unwrap_mirrorv,
		unwrap_expandsel, unwrap_contractsel,
		unwrap_setFalloffType,unwrap_getFalloffType,
		unwrap_setFalloffSpace,unwrap_getFalloffSpace,
		unwrap_setFalloffDist,unwrap_getFalloffDist,
		unwrap_breakselected,
		unwrap_weldselected, unwrap_weld,
		unwrap_updatemap,unwrap_displaymap,unwrap_ismapdisplayed,
		unwrap_setuvspace, unwrap_getuvspace,
		unwrap_options,
		unwrap_lock,
		unwrap_hide, unwrap_unhide,
		unwrap_freeze, unwrap_thaw,
		unwrap_filterselected,
		unwrap_pan,unwrap_zoom, unwrap_zoomregion, unwrap_fit, unwrap_fitselected,
		unwrap_snap,
		unwrap_getcurrentmap,unwrap_setcurrentmap, unwrap_numbermaps,
		unwrap_getlinecolor,unwrap_setlinecolor,
		unwrap_getselectioncolor,unwrap_setselectioncolor,
		unwrap_getrenderwidth,unwrap_setrenderwidth,
		unwrap_getrenderheight,unwrap_setrenderheight,
		unwrap_getusebitmapres,unwrap_setusebitmapres,
		unwrap_getweldtheshold,unwrap_setweldtheshold,

		unwrap_getconstantupdate,unwrap_setconstantupdate,
		unwrap_getshowselectedvertices,unwrap_setshowselectedvertices,
		unwrap_getmidpixelsnap,unwrap_setmidpixelsnap,

		unwrap_getmatid,unwrap_setmatid, unwrap_numbermatids,
		unwrap_getselectedverts, unwrap_selectverts,
		unwrap_isvertexselected,

		unwrap_moveselectedvertices,
		unwrap_rotateselectedverticesc,
		unwrap_rotateselectedvertices,
		unwrap_scaleselectedverticesc,
		unwrap_scaleselectedvertices,
		unwrap_getvertexposition,
		unwrap_numbervertices,

		unwrap_movex, unwrap_movey, unwrap_movez,

		unwrap_getselectedpolygons, unwrap_selectpolygons, unwrap_ispolygonselected,
		unwrap_numberpolygons,

		unwrap_detachedgeverts,
		unwrap_fliph,unwrap_flipv ,

		unwrap_setlockaspect,unwrap_getlockaspect,

		unwrap_setmapscale,unwrap_getmapscale,
		unwrap_getselectionfromface,

		unwrap_forceupdate,
		unwrap_zoomtogizmo,

		unwrap_setvertexposition,
		unwrap_addvertex,
		unwrap_markasdead,

		unwrap_numberpointsinface,
		unwrap_getvertexindexfromface,
		unwrap_gethandleindexfromface,
		unwrap_getinteriorindexfromface,

		unwrap_getvertexgindexfromface,
		unwrap_gethandlegindexfromface,
		unwrap_getinteriorgindexfromface,


		unwrap_addpointtoface,
		unwrap_addpointtohandle,
		unwrap_addpointtointerior,

		unwrap_setfacevertexindex,
		unwrap_setfacehandleindex,
		unwrap_setfaceinteriorindex,

		unwrap_updateview,

		unwrap_getfaceselfromstack,

//UNFOLD STUFF
		unwrap_selectfacesbynormal,
		unwrap_selectclusterbynormal,
		unwrap_selectpolygonsupdate,

		unwrap_normalmap,
		unwrap_normalmapnoparams,
		unwrap_normalmapdialog,

		unwrap_flattenmap,
		unwrap_flattenmapdialog,
		unwrap_flattenmapnoparams,

		unwrap_unfoldmap,
		unwrap_unfoldmapnoparams,
		unwrap_unfoldmapdialog,
		

		unwrap_hideselectedpolygons,
		unwrap_unhideallpolygons,

		unwrap_getnormal,
		unwrap_setseedface,

		unwrap_showvertexconnectionlist,

//COPY PASTE
		unwrap_copy,
		unwrap_paste,
		unwrap_pasteinstance,

		unwrap_setdebuglevel,

//STITCH STUFF
		unwrap_stitchverts,
		unwrap_stitchvertsnoparams,
		unwrap_stitchvertsdialog,
		unwrap_selectelement,

//TILEOPTIONS
		unwrap_gettilemap,
		unwrap_settilemap,
		unwrap_gettilemaplimit,
		unwrap_settilemaplimit,
		unwrap_gettilemapcontrast,
		unwrap_settilemapcontrast,

		unwrap_getshowmap,unwrap_setshowmap,

		unwrap_setlimitsoftsel,
		unwrap_getlimitsoftsel,

		
//SELECTION TOOLS AND OPTIONS
		unwrap_setlimitsoftselrange,
		unwrap_getlimitsoftselrange,

		unwrap_getvertexweight, unwrap_setvertexweight,
		unwrap_isweightmodified,unwrap_modifyweight,

		unwrap_getgeom_elemmode,unwrap_setgeom_elemmode,

		unwrap_getgeom_planarmode,unwrap_setgeom_planarmode,
		unwrap_getgeom_planarmodethreshold,unwrap_setgeom_planarmodethreshold,

		unwrap_getwindowx, unwrap_getwindowy, unwrap_getwindoww, unwrap_getwindowh, 

		unwrap_getbackfacecull,unwrap_setbackfacecull,

		unwrap_getoldselmethod,unwrap_setoldselmethod,
		unwrap_selectbymatid,
		unwrap_selectbysg,

		unwrap_gettvelementmode,unwrap_settvelementmode,

		unwrap_geomexpandsel, unwrap_geomcontractsel,

		unwrap_getalwaysedit,unwrap_setalwaysedit,

		unwrap_getshowvertexconnectionlist,unwrap_setshowvertexconnectionlist,

		unwrap_getfilterselected,unwrap_setfilterselected,

		unwrap_getsnap,	unwrap_setsnap,
		unwrap_getlock,	unwrap_setlock,

		unwrap_pack,
		unwrap_packnoparams,
		unwrap_packdialog,

		unwrap_gettvsubobjectmode,unwrap_settvsubobjectmode,

		unwrap_getselectedfaces, unwrap_selectfaces,
		unwrap_isfaceselected,

		unwrap_getfillmode,unwrap_setfillmode,

		unwrap_moveselected,
		unwrap_rotateselectedc,
		unwrap_rotateselected,
		unwrap_scaleselectedc,
		unwrap_scaleselected,

		unwrap_getselectededges, unwrap_selectedges,
		unwrap_isedgeselected,

		unwrap_getdisplayopenedge,
		unwrap_setdisplayopenedge,
		
		unwrap_getopenedgecolor,
		unwrap_setopenedgecolor,

		unwrap_getuvedgemode,
		unwrap_setuvedgemode,

		unwrap_getopenedgemode,
		unwrap_setopenedgemode,

		unwrap_uvedgeselect,
		unwrap_openedgeselect,

		unwrap_selectverttoedge,
		unwrap_selectverttoface,

		unwrap_selectedgetovert,
		unwrap_selectedgetoface,

		unwrap_selectfacetovert,
		unwrap_selectfacetoedge,

		unwrap_getdisplayhiddenedge,
		unwrap_setdisplayhiddenedge,

		unwrap_gethandlecolor,unwrap_sethandlecolor,

		unwrap_getfreeformmode,unwrap_setfreeformmode,

		unwrap_getfreeformcolor,unwrap_setfreeformcolor,
		unwrap_scaleselectedxy,

		unwrap_snappivot,
		unwrap_getpivotoffset,unwrap_setpivotoffset,
		unwrap_getselcenter,

		unwrap_sketch,
		unwrap_sketchnoparams,
		unwrap_sketchdialog,

		unwrap_sketchreverse,
		
		unwrap_gethitsize,unwrap_sethitsize,

		unwrap_getresetpivotonsel,unwrap_setresetpivotonsel,

		unwrap_getpolymode,unwrap_setpolymode,
		unwrap_polyselect,

		unwrap_getselectioninsidegizmo,unwrap_setselectioninsidegizmo,

		unwrap_setasdefaults,
		unwrap_loaddefaults,

		unwrap_getshowshared,unwrap_setshowshared,
		unwrap_getsharedcolor,unwrap_setsharedcolor,
		
		unwrap_showicon,

		unwrap_getsyncselectionmode,unwrap_setsyncselectionmode,
		unwrap_synctvselection,unwrap_syncgeomselection,

		unwrap_getbackgroundcolor,unwrap_setbackgroundcolor,

		unwrap_updatemenubar,

		unwrap_getbrightcentertile,unwrap_setbrightcentertile,

		unwrap_getblendtiletoback,unwrap_setblendtiletoback,

		unwrap_getblendtoback,unwrap_setblendtoback,

		unwrap_getpaintmode,unwrap_setpaintmode,
		unwrap_getpaintsize,unwrap_setpaintsize,
		unwrap_incpaintsize,unwrap_decpaintsize,

		unwrap_getticksize,unwrap_setticksize,

//NEW
		unwrap_getgridsize,unwrap_setgridsize,
		unwrap_getgridsnap,unwrap_setgridsnap,
		unwrap_getgridvisible,unwrap_setgridvisible,
		unwrap_getgridcolor,unwrap_setgridcolor,
		unwrap_getgridstr,unwrap_setgridstr,

		unwrap_getautomap,unwrap_setautomap,
//flatten defaults
		unwrap_getflattenangle,unwrap_setflattenangle,
		unwrap_getflattenspacing,unwrap_setflattenspacing,
		unwrap_getflattennormalize,unwrap_setflattennormalize,
		unwrap_getflattenrotate,unwrap_setflattenrotate,
		unwrap_getflattenfillholes,unwrap_setflattenfillholes,

		unwrap_getpreventflattening,unwrap_setpreventflattening,

		unwrap_getenablesoftselection,unwrap_setenablesoftselection,
		unwrap_getapplytowholeobject,unwrap_setapplytowholeobject,

		unwrap_setvertexposition2,
		unwrap_relax,
		unwrap_fitrelax,
//5.1.05
		unwrap_getautobackground,unwrap_setautobackground,

//5.1.06
		unwrap_relax2, unwrap_relax2dialog,
		unwrap_setrelaxamount,unwrap_getrelaxamount,
		unwrap_setrelaxiter,unwrap_getrelaxiter,

		unwrap_setrelaxboundary,unwrap_getrelaxboundary,
		unwrap_setrelaxsaddle,unwrap_getrelaxsaddle,




		};
//****************************************************************


class IUnwrapMod :  public Modifier, public FPMixinInterface 
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			VFN_0(unwrap_planarmap, fnPlanarMap);
			VFN_0(unwrap_save, fnSave);
			VFN_0(unwrap_load, fnLoad);
			VFN_0(unwrap_reset, fnReset);
			VFN_0(unwrap_edit, fnEdit);
			VFN_1(unwrap_setMapChannel, fnSetMapChannel,TYPE_INT);
			FN_0(unwrap_getMapChannel, TYPE_INT, fnGetMapChannel);
			VFN_1(unwrap_setProjectionType, fnSetProjectionType,TYPE_INT);
			FN_0(unwrap_getProjectionType, TYPE_INT, fnGetProjectionType);
			VFN_1(unwrap_setVC, fnSetVC,TYPE_BOOL);
			FN_0(unwrap_getVC, TYPE_BOOL, fnGetVC);

			VFN_0(unwrap_move, fnMove);
			VFN_0(unwrap_moveh, fnMoveH);
			VFN_0(unwrap_movev, fnMoveV);

			VFN_0(unwrap_rotate, fnRotate);

			VFN_0(unwrap_scale, fnScale);
			VFN_0(unwrap_scaleh, fnScaleH);
			VFN_0(unwrap_scalev, fnScaleV);

			VFN_0(unwrap_mirrorh, fnMirrorH);
			VFN_0(unwrap_mirrorv, fnMirrorV);
			VFN_0(unwrap_expandsel, fnExpandSelection);
			VFN_0(unwrap_contractsel, fnContractSelection);
			VFN_1(unwrap_setFalloffType, fnSetFalloffType,TYPE_INT);
			FN_0(unwrap_getFalloffType, TYPE_INT, fnGetFalloffType);
			VFN_1(unwrap_setFalloffSpace, fnSetFalloffSpace,TYPE_INT);
			FN_0(unwrap_getFalloffSpace, TYPE_INT, fnGetFalloffSpace);
			VFN_1(unwrap_setFalloffDist, fnSetFalloffDist,TYPE_FLOAT);
			FN_0(unwrap_getFalloffDist, TYPE_FLOAT, fnGetFalloffDist);
			VFN_0(unwrap_breakselected, fnBreakSelected);
			VFN_0(unwrap_weld, fnWeld);
			VFN_0(unwrap_weldselected, fnWeldSelected);
			VFN_0(unwrap_updatemap, fnUpdatemap);
			VFN_1(unwrap_displaymap, fnDisplaymap, TYPE_BOOL);
			FN_0(unwrap_ismapdisplayed, TYPE_BOOL, fnIsMapDisplayed);
			VFN_1(unwrap_setuvspace, fnSetUVSpace,TYPE_INT);
			FN_0(unwrap_getuvspace, TYPE_INT, fnGetUVSpace);
			VFN_0(unwrap_options, fnOptions);
			VFN_0(unwrap_lock, fnLock);
			VFN_0(unwrap_hide, fnHide);
			VFN_0(unwrap_unhide, fnUnhide);
			VFN_0(unwrap_freeze, fnFreeze);
			VFN_0(unwrap_thaw, fnThaw);
			VFN_0(unwrap_filterselected, fnFilterSelected);

			VFN_0(unwrap_pan, fnPan);
			VFN_0(unwrap_zoom, fnZoom);
			VFN_0(unwrap_zoomregion, fnZoomRegion);
			VFN_0(unwrap_fit, fnFit);
			VFN_0(unwrap_fitselected, fnFitSelected);

			VFN_0(unwrap_snap, fnSnap);

			FN_0(unwrap_getcurrentmap,TYPE_INT, fnGetCurrentMap);
			VFN_1(unwrap_setcurrentmap, fnSetCurrentMap,TYPE_INT);
			FN_0(unwrap_numbermaps,TYPE_INT, fnNumberMaps);

			FN_0(unwrap_getlinecolor,TYPE_POINT3, fnGetLineColor);
			VFN_1(unwrap_setlinecolor, fnSetLineColor,TYPE_POINT3);
			FN_0(unwrap_getselectioncolor,TYPE_POINT3, fnGetSelColor);
			VFN_1(unwrap_setselectioncolor, fnSetSelColor,TYPE_POINT3);

			FN_0(unwrap_getrenderwidth,TYPE_INT, fnGetRenderWidth);
			VFN_1(unwrap_setrenderwidth, fnSetRenderWidth,TYPE_INT);
			FN_0(unwrap_getrenderheight,TYPE_INT, fnGetRenderHeight);
			VFN_1(unwrap_setrenderheight, fnSetRenderHeight,TYPE_INT);

			FN_0(unwrap_getusebitmapres,TYPE_BOOL, fnGetUseBitmapRes);
			VFN_1(unwrap_setusebitmapres, fnSetUseBitmapRes,TYPE_BOOL);

			FN_0(unwrap_getweldtheshold,TYPE_FLOAT, fnGetWeldThresold);
			VFN_1(unwrap_setweldtheshold, fnSetWeldThreshold,TYPE_FLOAT);


			FN_0(unwrap_getconstantupdate,TYPE_BOOL, fnGetConstantUpdate);
			VFN_1(unwrap_setconstantupdate, fnSetConstantUpdate,TYPE_BOOL);

			FN_0(unwrap_getshowselectedvertices,TYPE_BOOL, fnGetShowSelectedVertices);
			VFN_1(unwrap_setshowselectedvertices, fnSetShowSelectedVertices,TYPE_BOOL);

			FN_0(unwrap_getmidpixelsnap,TYPE_BOOL, fnGetMidPixelSnape);
			VFN_1(unwrap_setmidpixelsnap, fnSetMidPixelSnape,TYPE_BOOL);


			FN_0(unwrap_getmatid,TYPE_INT, fnGetMatID);
			VFN_1(unwrap_setmatid, fnSetMatID,TYPE_INT);
			FN_0(unwrap_numbermatids,TYPE_INT, fnNumberMatIDs);

			FN_0(unwrap_getselectedverts,TYPE_BITARRAY, fnGetSelectedVerts);
			VFN_1(unwrap_selectverts, fnSelectVerts,TYPE_BITARRAY);
			FN_1(unwrap_isvertexselected,TYPE_BOOL, fnIsVertexSelected,TYPE_INT);

			VFN_1(unwrap_moveselectedvertices, fnMoveSelectedVertices,TYPE_POINT3);
			VFN_1(unwrap_rotateselectedverticesc, fnRotateSelectedVertices,TYPE_FLOAT);
			VFN_2(unwrap_rotateselectedvertices, fnRotateSelectedVertices,TYPE_FLOAT, TYPE_POINT3);
			VFN_2(unwrap_scaleselectedverticesc, fnScaleSelectedVertices,TYPE_FLOAT, TYPE_INT);
			VFN_3(unwrap_scaleselectedvertices, fnScaleSelectedVertices,TYPE_FLOAT, TYPE_INT,TYPE_POINT3);

			FN_2(unwrap_getvertexposition,TYPE_POINT3, fnGetVertexPosition, TYPE_TIMEVALUE, TYPE_INT);
			FN_0(unwrap_numbervertices,TYPE_INT, fnNumberVertices);

			VFN_1(unwrap_movex, fnMoveX,TYPE_FLOAT);
			VFN_1(unwrap_movey, fnMoveY,TYPE_FLOAT);
			VFN_1(unwrap_movez, fnMoveZ,TYPE_FLOAT);

			FN_0(unwrap_getselectedpolygons,TYPE_BITARRAY, fnGetSelectedPolygons);
			VFN_1(unwrap_selectpolygons, fnSelectPolygons,TYPE_BITARRAY);
			FN_1(unwrap_ispolygonselected,TYPE_BOOL, fnIsPolygonSelected,TYPE_INT);
			FN_0(unwrap_numberpolygons,TYPE_INT, fnNumberPolygons);
			VFN_0(unwrap_detachedgeverts, fnDetachEdgeVerts);
			VFN_0(unwrap_fliph, fnFlipH);
			VFN_0(unwrap_flipv, fnFlipV);
			
			VFN_1(unwrap_setlockaspect, fnSetLockAspect,TYPE_BOOL);
			FN_0(unwrap_getlockaspect,TYPE_BOOL, fnGetLockAspect);

			VFN_1(unwrap_setmapscale, fnSetMapScale,TYPE_FLOAT);
			FN_0(unwrap_getmapscale,TYPE_FLOAT, fnGetMapScale);

			VFN_0(unwrap_getselectionfromface, fnGetSelectionFromFace);

			VFN_1(unwrap_forceupdate, fnForceUpdate,TYPE_BOOL);

			VFN_1(unwrap_zoomtogizmo, fnZoomToGizmo,TYPE_BOOL);

			VFN_3(unwrap_setvertexposition, fnSetVertexPosition,TYPE_TIMEVALUE,TYPE_INT,TYPE_POINT3);
			VFN_1(unwrap_markasdead, fnMarkAsDead,TYPE_INT);

			FN_1(unwrap_numberpointsinface,TYPE_INT,fnNumberPointsInFace,TYPE_INT);
			FN_2(unwrap_getvertexindexfromface,TYPE_INT,fnGetVertexIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_gethandleindexfromface,TYPE_INT,fnGetHandleIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_getinteriorindexfromface,TYPE_INT,fnGetInteriorIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_getvertexgindexfromface,TYPE_INT,fnGetVertexGIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_gethandlegindexfromface,TYPE_INT,fnGetHandleGIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_getinteriorgindexfromface,TYPE_INT,fnGetInteriorGIndexFromFace,TYPE_INT,TYPE_INT);
			
			VFN_4(unwrap_addpointtoface,fnAddPoint,TYPE_POINT3,TYPE_INT,TYPE_INT,TYPE_BOOL);
			VFN_4(unwrap_addpointtohandle,fnAddHandle,TYPE_POINT3,TYPE_INT,TYPE_INT,TYPE_BOOL);
			VFN_4(unwrap_addpointtointerior,fnAddInterior,TYPE_POINT3,TYPE_INT,TYPE_INT,TYPE_BOOL);

			VFN_3(unwrap_setfacevertexindex,fnSetFaceVertexIndex,TYPE_INT,TYPE_INT,TYPE_INT);
			VFN_3(unwrap_setfacehandleindex,fnSetFaceHandleIndex,TYPE_INT,TYPE_INT,TYPE_INT);
			VFN_3(unwrap_setfaceinteriorindex,fnSetFaceInteriorIndex,TYPE_INT,TYPE_INT,TYPE_INT);

			VFN_0(unwrap_updateview,fnUpdateViews);

			VFN_0(unwrap_getfaceselfromstack,fnGetFaceSelFromStack);



		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 

		virtual void	fnPlanarMap()=0;
		virtual void	fnSave()=0;
		virtual void	fnLoad()=0;
		virtual void	fnReset()=0;
		virtual void	fnEdit()=0;

		virtual void	fnSetMapChannel(int channel)=0;
		virtual int		fnGetMapChannel()=0;

		virtual void	fnSetProjectionType(int proj)=0;
		virtual int		fnGetProjectionType()=0;

		virtual void	fnSetVC(BOOL vc)=0;
		virtual BOOL	fnGetVC()=0;

		virtual void	fnMove()=0;
		virtual void	fnMoveH()=0;
		virtual void	fnMoveV()=0;

		virtual void	fnRotate()=0;

		virtual void	fnScale()=0;
		virtual void	fnScaleH()=0;
		virtual void	fnScaleV()=0;

		virtual void	fnMirrorH()=0;
		virtual void	fnMirrorV()=0;

		virtual void	fnExpandSelection()=0;
		virtual void	fnContractSelection()=0;
		

		virtual void	fnSetFalloffType(int falloff)=0;
		virtual int		fnGetFalloffType()=0;
		virtual void	fnSetFalloffSpace(int space)=0;
		virtual int		fnGetFalloffSpace()=0;
		virtual void	fnSetFalloffDist(float dist)=0;
		virtual float	fnGetFalloffDist()=0;

		virtual void	fnBreakSelected()=0;
		virtual void	fnWeld()=0;
		virtual void	fnWeldSelected()=0;

		virtual void	fnUpdatemap()=0;
		virtual void	fnDisplaymap(BOOL update)=0;
		virtual BOOL	fnIsMapDisplayed()=0;

		virtual void	fnSetUVSpace(int space)=0;
		virtual int		fnGetUVSpace()=0;
		virtual void	fnOptions()=0;

		virtual void	fnLock()=0;
		virtual void	fnHide()=0;
		virtual void	fnUnhide()=0;

		virtual void	fnFreeze()=0;
		virtual void	fnThaw()=0;
		virtual void	fnFilterSelected()=0;

		virtual void	fnPan()=0;
		virtual void	fnZoom()=0;
		virtual void	fnZoomRegion()=0;
		virtual void	fnFit()=0;
		virtual void	fnFitSelected()=0;

		virtual void	fnSnap()=0;


		virtual int		fnGetCurrentMap()=0;
		virtual void	fnSetCurrentMap(int map)=0;
		virtual int		fnNumberMaps()=0;

		virtual Point3*	fnGetLineColor()=0;
		virtual void	fnSetLineColor(Point3 color)=0;

		virtual Point3*	fnGetSelColor()=0;
		virtual void	fnSetSelColor(Point3 color)=0;



		virtual void	fnSetRenderWidth(int dist)=0;
		virtual int		fnGetRenderWidth()=0;
		virtual void	fnSetRenderHeight(int dist)=0;
		virtual int		fnGetRenderHeight()=0;
		
		virtual void	fnSetWeldThreshold(float dist)=0;
		virtual float	fnGetWeldThresold()=0;

		virtual void	fnSetUseBitmapRes(BOOL useBitmapRes)=0;
		virtual BOOL	fnGetUseBitmapRes()=0;

		
		virtual BOOL	fnGetConstantUpdate()=0;
		virtual void	fnSetConstantUpdate(BOOL constantUpdates)=0;

		virtual BOOL	fnGetShowSelectedVertices()=0;
		virtual void	fnSetShowSelectedVertices(BOOL show)=0;

		virtual BOOL	fnGetMidPixelSnape()=0;
		virtual void	fnSetMidPixelSnape(BOOL midPixel)=0;

		virtual int		fnGetMatID()=0;
		virtual void	fnSetMatID(int matid)=0;
		virtual int		fnNumberMatIDs()=0;

		virtual BitArray* fnGetSelectedVerts()=0;
		virtual void fnSelectVerts(BitArray *sel)=0;
		virtual BOOL fnIsVertexSelected(int index)=0;

		virtual void fnMoveSelectedVertices(Point3 offset)=0;
		virtual void fnRotateSelectedVertices(float angle)=0;
		virtual void fnRotateSelectedVertices(float angle, Point3 axis)=0;
		virtual void fnScaleSelectedVertices(float scale,int dir)=0;
		virtual void fnScaleSelectedVertices(float scale,int dir,Point3 axis)=0;
		virtual Point3* fnGetVertexPosition(TimeValue t,  int index)=0;
		virtual int fnNumberVertices()=0;

		virtual void fnMoveX(float p)=0;
		virtual void fnMoveY(float p)=0;
		virtual void fnMoveZ(float p)=0;

		virtual BitArray* fnGetSelectedPolygons()=0;
		virtual void fnSelectPolygons(BitArray *sel)=0;
		virtual BOOL fnIsPolygonSelected(int index)=0;
		virtual int fnNumberPolygons()=0;

		virtual void fnDetachEdgeVerts()=0;

		virtual void fnFlipH()=0;
		virtual void fnFlipV()=0;

		virtual BOOL	fnGetLockAspect()=0;
		virtual void	fnSetLockAspect(BOOL a)=0;

		virtual float	fnGetMapScale()=0;
		virtual void	fnSetMapScale(float sc)=0;

		virtual void	fnGetSelectionFromFace()=0;
		virtual void	fnForceUpdate(BOOL update)= 0;

		virtual void	fnZoomToGizmo(BOOL all)= 0;

		virtual void	fnSetVertexPosition(TimeValue t, int index, Point3 pos) = 0;
		virtual void	fnMarkAsDead(int index) = 0;

		virtual int		fnNumberPointsInFace(int index)=0;
		virtual int		fnGetVertexIndexFromFace(int index,int vertexIndex)=0;
		virtual int		fnGetHandleIndexFromFace(int index,int vertexIndex)=0;
		virtual int		fnGetInteriorIndexFromFace(int index,int vertexIndex)=0;
		virtual int		fnGetVertexGIndexFromFace(int index,int vertexIndex)=0;
		virtual int		fnGetHandleGIndexFromFace(int index,int vertexIndex)=0;
		virtual int		fnGetInteriorGIndexFromFace(int index,int vertexIndex)=0;

		virtual void	fnAddPoint(Point3 pos, int fIndex,int ithV, BOOL sel)=0;
		virtual void	fnAddHandle(Point3 pos, int fIndex,int ithV, BOOL sel)=0;
		virtual void	fnAddInterior(Point3 pos, int fIndex,int ithV, BOOL sel)=0;

		virtual void	fnSetFaceVertexIndex(int fIndex,int ithV, int vIndex)=0;
		virtual void	fnSetFaceHandleIndex(int fIndex,int ithV, int vIndex)=0;
		virtual void	fnSetFaceInteriorIndex(int fIndex,int ithV, int vIndex)=0;

		virtual void	fnUpdateViews()=0;

		virtual void	fnGetFaceSelFromStack()=0;


	};

class IUnwrapMod2 : public FPMixinInterface
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

//UNFOLD STUFF
			VFN_2(unwrap_selectpolygonsupdate, fnSelectPolygonsUpdate,TYPE_BITARRAY, TYPE_BOOL);
			VFN_3(unwrap_selectfacesbynormal,fnSelectFacesByNormal,TYPE_POINT3,TYPE_FLOAT, TYPE_BOOL);
			VFN_4(unwrap_selectclusterbynormal,fnSelectClusterByNormal,TYPE_FLOAT,TYPE_INT, TYPE_BOOL, TYPE_BOOL);

			VFN_7(unwrap_flattenmap,fnFlattenMap,TYPE_FLOAT,TYPE_POINT3_TAB,TYPE_FLOAT,TYPE_BOOL,TYPE_INT,TYPE_BOOL,TYPE_BOOL);

			VFN_6(unwrap_normalmap,fnNormalMap,TYPE_POINT3_TAB,TYPE_FLOAT,TYPE_BOOL,TYPE_INT,TYPE_BOOL,TYPE_BOOL);
			VFN_0(unwrap_normalmapnoparams,fnNormalMapNoParams);
			VFN_0(unwrap_normalmapdialog,fnNormalMapDialog);

			VFN_2(unwrap_unfoldmap,fnUnfoldSelectedPolygons,TYPE_INT,TYPE_BOOL);
			VFN_0(unwrap_unfoldmapdialog,fnUnfoldSelectedPolygonsDialog);
			VFN_0(unwrap_unfoldmapnoparams,fnUnfoldSelectedPolygonsNoParams);
			


			VFN_0(unwrap_hideselectedpolygons,fnHideSelectedPolygons);
			VFN_0(unwrap_unhideallpolygons,fnUnhideAllPolygons);

			FN_1(unwrap_getnormal,TYPE_POINT3,fnGetNormal,TYPE_INT);

			VFN_0(unwrap_setseedface,fnSetSeedFace);

			VFN_0(unwrap_showvertexconnectionlist,fnShowVertexConnectionList);
//COPYPASTE STUFF
			VFN_0(unwrap_copy,fnCopy);
			VFN_1(unwrap_paste,fnPaste,TYPE_BOOL);
			VFN_0(unwrap_pasteinstance,fnPasteInstance);

			VFN_1(unwrap_setdebuglevel,fnSetDebugLevel,TYPE_INT);
			VFN_2(unwrap_stitchverts,fnStitchVerts,TYPE_BOOL,TYPE_FLOAT);
			VFN_0(unwrap_stitchvertsnoparams,fnStitchVertsNoParams);
			VFN_0(unwrap_stitchvertsdialog,fnStitchVertsDialog);

			VFN_0(unwrap_selectelement,fnSelectElement);

			VFN_0(unwrap_flattenmapdialog,fnFlattenMapDialog);
			VFN_0(unwrap_flattenmapnoparams,fnFlattenMapNoParams);
//TILE STUFF
			FN_0(unwrap_gettilemap,TYPE_BOOL, fnGetTile);
			VFN_1(unwrap_settilemap,fnSetTile,TYPE_BOOL);

			FN_0(unwrap_gettilemaplimit,TYPE_INT, fnGetTileLimit);
			VFN_1(unwrap_settilemaplimit,fnSetTileLimit,TYPE_INT);

			FN_0(unwrap_gettilemapcontrast,TYPE_FLOAT, fnGetTileContrast);
			VFN_1(unwrap_settilemapcontrast,fnSetTileContrast,TYPE_FLOAT);


			FN_0(unwrap_getshowmap,TYPE_BOOL, fnGetShowMap);
			VFN_1(unwrap_setshowmap,fnSetShowMap,TYPE_BOOL);


//SELECTION STUFF
			FN_0(unwrap_setlimitsoftsel,TYPE_BOOL, fnGetLimitSoftSel);
			VFN_1(unwrap_getlimitsoftsel,fnSetLimitSoftSel,TYPE_BOOL);

			FN_0(unwrap_setlimitsoftselrange,TYPE_INT, fnGetLimitSoftSelRange);
			VFN_1(unwrap_getlimitsoftselrange,fnSetLimitSoftSelRange,TYPE_INT);

			FN_1(unwrap_getvertexweight,TYPE_FLOAT, fnGetVertexWeight,TYPE_INDEX);
			VFN_2(unwrap_setvertexweight,fnSetVertexWeight,TYPE_INT,TYPE_FLOAT);
		
			
			FN_1(unwrap_isweightmodified,TYPE_BOOL, fnIsWeightModified,TYPE_INT);
			VFN_2(unwrap_modifyweight,fnModifyWeight,TYPE_INT,TYPE_BOOL);

			FN_0(unwrap_getgeom_elemmode,TYPE_BOOL, fnGetGeomElemMode);
			VFN_1(unwrap_setgeom_elemmode,fnSetGeomElemMode,TYPE_BOOL);

			FN_0(unwrap_getgeom_planarmode,TYPE_BOOL, fnGetGeomPlanarMode);
			VFN_1(unwrap_setgeom_planarmode,fnSetGeomPlanarMode,TYPE_BOOL);

			FN_0(unwrap_getgeom_planarmodethreshold,TYPE_FLOAT, fnGetGeomPlanarModeThreshold);
			VFN_1(unwrap_setgeom_planarmodethreshold,fnSetGeomPlanarModeThreshold,TYPE_FLOAT);

			FN_0(unwrap_getwindowx,TYPE_INT, fnGetWindowX);
			FN_0(unwrap_getwindowy,TYPE_INT, fnGetWindowY);
			FN_0(unwrap_getwindoww,TYPE_INT, fnGetWindowW);
			FN_0(unwrap_getwindowh,TYPE_INT, fnGetWindowH);

			FN_0(unwrap_getbackfacecull,TYPE_BOOL, fnGetBackFaceCull);
			VFN_1(unwrap_setbackfacecull,fnSetBackFaceCull,TYPE_BOOL);

			FN_0(unwrap_getoldselmethod,TYPE_BOOL, fnGetOldSelMethod);
			VFN_1(unwrap_setoldselmethod,fnSetOldSelMethod,TYPE_BOOL);

			VFN_1(unwrap_selectbymatid,fnSelectByMatID,TYPE_INT);
			VFN_1(unwrap_selectbysg,fnSelectBySG,TYPE_INT);

			FN_0(unwrap_gettvelementmode,TYPE_BOOL, fnGetTVElementMode);
			VFN_1(unwrap_settvelementmode,fnSetTVElementMode,TYPE_BOOL);

			VFN_0(unwrap_geomexpandsel,fnGeomExpandFaceSel);
			VFN_0(unwrap_geomcontractsel,fnGeomContractFaceSel);

			FN_0(unwrap_getalwaysedit,TYPE_BOOL, fnGetAlwaysEdit);
			VFN_1(unwrap_setalwaysedit,fnSetAlwaysEdit,TYPE_BOOL);

			FN_0(unwrap_getshowvertexconnectionlist,TYPE_BOOL, fnGetShowConnection);
			VFN_1(unwrap_setshowvertexconnectionlist,fnSetShowConnection,TYPE_BOOL);

			FN_0(unwrap_getfilterselected,TYPE_BOOL, fnGetFilteredSelected);
			VFN_1(unwrap_setfilterselected,fnSetFilteredSelected,TYPE_BOOL);

			FN_0(unwrap_getsnap,TYPE_BOOL, fnGetSnap);
			VFN_1(unwrap_setsnap,fnSetSnap,TYPE_BOOL);

			FN_0(unwrap_getlock,TYPE_BOOL, fnGetLock);
			VFN_1(unwrap_setlock,fnSetLock,TYPE_BOOL);

			VFN_5(unwrap_pack, fnPack,TYPE_INT, TYPE_FLOAT, TYPE_BOOL,TYPE_BOOL,TYPE_BOOL);
			VFN_0(unwrap_packnoparams, fnPackNoParams);
			VFN_0(unwrap_packdialog, fnPackDialog);

			FN_0(unwrap_gettvsubobjectmode,TYPE_INT, fnGetTVSubMode);
			VFN_1(unwrap_settvsubobjectmode,fnSetTVSubMode,TYPE_INT);

			FN_0(unwrap_getselectedfaces,TYPE_BITARRAY, fnGetSelectedFaces);
			VFN_1(unwrap_selectfaces, fnSelectFaces,TYPE_BITARRAY);
			FN_1(unwrap_isfaceselected,TYPE_BOOL, fnIsFaceSelected,TYPE_INT);

			FN_0(unwrap_getfillmode,TYPE_INT, fnGetFillMode);
			VFN_1(unwrap_setfillmode,fnSetFillMode,TYPE_INT);


			VFN_1(unwrap_moveselected, fnMoveSelected,TYPE_POINT3);
			VFN_1(unwrap_rotateselectedc, fnRotateSelected,TYPE_FLOAT);
			VFN_2(unwrap_rotateselected, fnRotateSelected,TYPE_FLOAT, TYPE_POINT3);
			VFN_2(unwrap_scaleselectedc, fnScaleSelected,TYPE_FLOAT, TYPE_INT);
			VFN_3(unwrap_scaleselected, fnScaleSelected,TYPE_FLOAT, TYPE_INT,TYPE_POINT3);

			FN_0(unwrap_getselectededges,TYPE_BITARRAY, fnGetSelectedEdges);
			VFN_1(unwrap_selectedges, fnSelectEdges,TYPE_BITARRAY);
			FN_1(unwrap_isedgeselected,TYPE_BOOL, fnIsEdgeSelected,TYPE_INT);


			FN_0(unwrap_getdisplayopenedge,TYPE_BOOL, fnGetDisplayOpenEdges);
			VFN_1(unwrap_setdisplayopenedge,fnSetDisplayOpenEdges,TYPE_BOOL);
		

			FN_0(unwrap_getopenedgecolor,TYPE_POINT3, fnGetOpenEdgeColor);
			VFN_1(unwrap_setopenedgecolor, fnSetOpenEdgeColor,TYPE_POINT3);

			FN_0(unwrap_getuvedgemode,TYPE_BOOL, fnGetUVEdgeMode);
			VFN_1(unwrap_setuvedgemode,fnSetUVEdgeMode,TYPE_BOOL);

			FN_0(unwrap_getopenedgemode,TYPE_BOOL, fnGetOpenEdgeMode);
			VFN_1(unwrap_setopenedgemode,fnSetOpenEdgeMode,TYPE_BOOL);

			VFN_0(unwrap_uvedgeselect,fnUVEdgeSelect);

			VFN_0(unwrap_openedgeselect,fnOpenEdgeSelect);

			VFN_0(unwrap_selectverttoedge,fnVertToEdgeSelect);
			VFN_0(unwrap_selectverttoface,fnVertToFaceSelect);

			VFN_0(unwrap_selectedgetovert,fnEdgeToVertSelect);
			VFN_0(unwrap_selectedgetoface,fnEdgeToFaceSelect);

			VFN_0(unwrap_selectfacetovert,fnFaceToVertSelect);
			VFN_0(unwrap_selectfacetoedge,fnFaceToEdgeSelect);

			FN_0(unwrap_getdisplayhiddenedge,TYPE_BOOL, fnGetDisplayHiddenEdges);
			VFN_1(unwrap_setdisplayhiddenedge,fnSetDisplayHiddenEdges,TYPE_BOOL);


			FN_0(unwrap_gethandlecolor,TYPE_POINT3, fnGetHandleColor);
			VFN_1(unwrap_sethandlecolor, fnSetHandleColor,TYPE_POINT3);

			FN_0(unwrap_getfreeformmode,TYPE_BOOL, fnGetFreeFormMode);
			VFN_1(unwrap_setfreeformmode,fnSetFreeFormMode,TYPE_BOOL);

			FN_0(unwrap_getfreeformcolor,TYPE_POINT3, fnGetFreeFormColor);
			VFN_1(unwrap_setfreeformcolor, fnSetFreeFormColor,TYPE_POINT3);

			VFN_3(unwrap_scaleselectedxy, fnScaleSelectedXY,TYPE_FLOAT, TYPE_FLOAT,TYPE_POINT3);

			VFN_1(unwrap_snappivot, fnSnapPivot,TYPE_INT);
			FN_0(unwrap_getpivotoffset,TYPE_POINT3, fnGetPivotOffset);
			VFN_1(unwrap_setpivotoffset, fnSetPivotOffset,TYPE_POINT3);		
			FN_0(unwrap_getselcenter,TYPE_POINT3, fnGetSelCenter);

			VFN_2(unwrap_sketch, fnSketch,TYPE_INT_TAB,TYPE_POINT3_TAB);
			VFN_0(unwrap_sketchnoparams, fnSketchNoParams);
			VFN_0(unwrap_sketchdialog, fnSketchDialog);
			VFN_0(unwrap_sketchreverse, fnSketchReverse);
 
			FN_0(unwrap_gethitsize,TYPE_INT, fnGetHitSize);
			VFN_1(unwrap_sethitsize, fnSetHitSize,TYPE_INT);

			FN_0(unwrap_getresetpivotonsel,TYPE_BOOL, fnGetResetPivotOnSel);
			VFN_1(unwrap_setresetpivotonsel, fnSetResetPivotOnSel,TYPE_BOOL);

			FN_0(unwrap_getpolymode,TYPE_BOOL, fnGetPolyMode);
			VFN_1(unwrap_setpolymode, fnSetPolyMode,TYPE_BOOL);
			VFN_0(unwrap_polyselect, fnPolySelect);


			FN_0(unwrap_getselectioninsidegizmo,TYPE_BOOL, fnGetAllowSelectionInsideGizmo);
			VFN_1(unwrap_setselectioninsidegizmo, fnSetAllowSelectionInsideGizmo,TYPE_BOOL);

			VFN_0(unwrap_setasdefaults, fnSetAsDefaults);
			VFN_0(unwrap_loaddefaults, fnLoadDefaults);


			
			FN_0(unwrap_getshowshared,TYPE_BOOL, fnGetShowShared);
			VFN_1(unwrap_setshowshared, fnSetShowShared,TYPE_BOOL);
			FN_0(unwrap_getsharedcolor,TYPE_POINT3, fnGetSharedColor);
			VFN_1(unwrap_setsharedcolor, fnSetSharedColor,TYPE_POINT3);

			VFN_2(unwrap_showicon, fnShowIcon,TYPE_INT,TYPE_BOOL);


			FN_0(unwrap_getsyncselectionmode,TYPE_BOOL, fnGetSyncSelectionMode);
			VFN_1(unwrap_setsyncselectionmode, fnSetSyncSelectionMode,TYPE_BOOL);

			VFN_0(unwrap_synctvselection, fnSyncTVSelection);
			VFN_0(unwrap_syncgeomselection, fnSyncGeomSelection);

			FN_0(unwrap_getbackgroundcolor,TYPE_POINT3, fnGetBackgroundColor);
			VFN_1(unwrap_setbackgroundcolor, fnSetBackgroundColor,TYPE_POINT3);

			VFN_0(unwrap_updatemenubar, fnUpdateMenuBar);


			FN_0(unwrap_getbrightcentertile,TYPE_BOOL, fnGetBrightCenterTile);
			VFN_1(unwrap_setbrightcentertile, fnSetBrightCenterTile,TYPE_BOOL);

			FN_0(unwrap_getblendtoback,TYPE_BOOL, fnGetBlendToBack);
			VFN_1(unwrap_setblendtoback, fnSetBlendToBack,TYPE_BOOL);

			FN_0(unwrap_getpaintmode,TYPE_BOOL, fnGetPaintMode);
			VFN_1(unwrap_setpaintmode, fnSetPaintMode,TYPE_BOOL);

			FN_0(unwrap_getpaintsize,TYPE_INT, fnGetPaintSize);
			VFN_1(unwrap_setpaintsize, fnSetPaintSize,TYPE_INT);

			VFN_0(unwrap_incpaintsize, fnIncPaintSize);
			VFN_0(unwrap_decpaintsize, fnDecPaintSize);

			FN_0(unwrap_getticksize,TYPE_INT, fnGetTickSize);
			VFN_1(unwrap_setticksize, fnSetTickSize,TYPE_INT);

//NEW
			FN_0(unwrap_getgridsize,TYPE_FLOAT, fnGetGridSize);
			VFN_1(unwrap_setgridsize, fnSetGridSize,TYPE_FLOAT);

			FN_0(unwrap_getgridsnap,TYPE_BOOL, fnGetGridSnap);
			VFN_1(unwrap_setgridsnap, fnSetGridSnap,TYPE_BOOL);

			FN_0(unwrap_getgridvisible,TYPE_BOOL, fnGetGridVisible);
			VFN_1(unwrap_setgridvisible, fnSetGridVisible,TYPE_BOOL);

			FN_0(unwrap_getgridcolor,TYPE_POINT3, fnGetGridColor);
			VFN_1(unwrap_setgridcolor, fnSetGridColor,TYPE_POINT3);

			FN_0(unwrap_getgridstr,TYPE_FLOAT, fnGetGridStr);
			VFN_1(unwrap_setgridstr, fnSetGridStr,TYPE_FLOAT);


			FN_0(unwrap_getautomap,TYPE_BOOL, fnGetAutoMap);
			VFN_1(unwrap_setautomap, fnSetAutoMap,TYPE_BOOL);

//flatten defaults
			FN_0(unwrap_getflattenangle,TYPE_FLOAT, fnGetFlattenAngle);
			VFN_1(unwrap_setflattenangle, fnSetFlattenAngle,TYPE_ANGLE);

			FN_0(unwrap_getflattenspacing,TYPE_FLOAT, fnGetFlattenSpacing);
			VFN_1(unwrap_setflattenspacing, fnSetFlattenSpacing,TYPE_FLOAT);

			FN_0(unwrap_getflattennormalize,TYPE_BOOL, fnGetFlattenNormalize);
			VFN_1(unwrap_setflattennormalize, fnSetFlattenNormalize,TYPE_BOOL);

			FN_0(unwrap_getflattenrotate,TYPE_BOOL, fnGetFlattenRotate);
			VFN_1(unwrap_setflattenrotate, fnSetFlattenRotate,TYPE_BOOL);

			FN_0(unwrap_getflattenfillholes,TYPE_BOOL, fnGetFlattenFillHoles);
			VFN_1(unwrap_setflattenfillholes, fnSetFlattenFillHoles,TYPE_BOOL);

			FN_0(unwrap_getpreventflattening,TYPE_BOOL, fnGetPreventFlattening);
			VFN_1(unwrap_setpreventflattening, fnSetPreventFlattening,TYPE_BOOL);			

			FN_0(unwrap_getenablesoftselection,TYPE_BOOL, fnGetEnableSoftSelection);
			VFN_1(unwrap_setenablesoftselection, fnSetEnableSoftSelection,TYPE_BOOL);			

			FN_0(unwrap_getapplytowholeobject,TYPE_BOOL, fnGetApplyToWholeObject);
			VFN_1(unwrap_setapplytowholeobject, fnSetApplyToWholeObject,TYPE_BOOL);			

			VFN_5(unwrap_setvertexposition2, fnSetVertexPosition2,TYPE_TIMEVALUE,TYPE_INT,TYPE_POINT3,TYPE_BOOL,TYPE_BOOL);

			VFN_4(unwrap_relax, fnRelax,TYPE_INT,TYPE_FLOAT,TYPE_BOOL,TYPE_BOOL);
			VFN_2(unwrap_fitrelax, fnFit,TYPE_INT,TYPE_FLOAT);


		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 

//UNFOLD STUFF
		virtual void	fnSelectPolygonsUpdate(BitArray *sel, BOOL update)=0;
		virtual void	fnSelectFacesByNormal(Point3 Normal, float angleThreshold, BOOL update)=0;
		virtual void	fnSelectClusterByNormal(float angleThreshold, int seedIndex, BOOL relative, BOOL update)=0;

		virtual void	fnFlattenMap(float angleThreshold, Tab<Point3*> *normaList, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL alignWidth)=0;

		virtual void	fnNormalMap(Tab<Point3*> *normaList, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL alignWidth)=0;
		virtual void	fnNormalMapNoParams()=0;
		virtual void	fnNormalMapDialog()=0;

		virtual void	fnUnfoldSelectedPolygons(int unfoldMethod,BOOL normalize)=0;
		virtual void	fnUnfoldSelectedPolygonsDialog()=0;
		virtual void	fnUnfoldSelectedPolygonsNoParams()=0;

		virtual void	fnHideSelectedPolygons()=0;
		virtual void	fnUnhideAllPolygons()=0;

		virtual Point3*	fnGetNormal(int index)=0;
		virtual void	fnSetSeedFace()=0;


		virtual void	fnShowVertexConnectionList() = 0;

//COPYPASTE STUFF
		virtual void	fnCopy() = 0;
		virtual void	fnPaste(BOOL rotate) = 0;
		virtual void	fnPasteInstance() = 0;

		virtual void	fnSetDebugLevel(int level) = 0;

		virtual void	fnStitchVerts(BOOL bAlign, float fBias) = 0;
		virtual void	fnStitchVertsNoParams() = 0;
		virtual void	fnStitchVertsDialog() = 0;
		virtual void	fnSelectElement() = 0;

		virtual void	fnFlattenMapDialog() = 0;
		virtual void	fnFlattenMapNoParams() = 0;

//TILE STUFF
		virtual BOOL	fnGetTile() = 0;
		virtual void	fnSetTile(BOOL tile) = 0;

		virtual int		fnGetTileLimit() = 0;
		virtual void	fnSetTileLimit(int lmit) = 0;

		virtual float	fnGetTileContrast() = 0;
		virtual void	fnSetTileContrast(float contrast) = 0;

		virtual BOOL	fnGetShowMap() = 0;
		virtual void	fnSetShowMap(BOOL smap) = 0;


		virtual BOOL	fnGetLimitSoftSel() = 0;
		virtual void	fnSetLimitSoftSel(BOOL limit) = 0;

		virtual int		fnGetLimitSoftSelRange() = 0;
		virtual void	fnSetLimitSoftSelRange(int range) = 0;

		virtual float	fnGetVertexWeight(int index) = 0;
		virtual void	fnSetVertexWeight(int index,float weight) = 0;

		virtual BOOL	fnIsWeightModified(int index) = 0;
		virtual void	fnModifyWeight(int index, BOOL modified) = 0;

		virtual BOOL	fnGetGeomElemMode() = 0;
		virtual void	fnSetGeomElemMode(BOOL elem) = 0;

		virtual BOOL	fnGetGeomPlanarMode() = 0;
		virtual void	fnSetGeomPlanarMode(BOOL planar) = 0;

		virtual float	fnGetGeomPlanarModeThreshold() = 0;
		virtual void	fnSetGeomPlanarModeThreshold(float threshold) = 0;

		virtual int		fnGetWindowX() = 0;
		virtual int		fnGetWindowY() = 0;
		virtual int		fnGetWindowW() = 0;
		virtual int		fnGetWindowH() = 0;


		virtual BOOL	fnGetBackFaceCull() = 0;
		virtual void	fnSetBackFaceCull(BOOL backFaceCull) = 0;

		virtual BOOL	fnGetOldSelMethod() = 0;
		virtual void	fnSetOldSelMethod(BOOL oldSelMethod) = 0;

		virtual void	fnSelectByMatID(int matID) = 0;
		virtual void	fnSelectBySG(int sg) = 0;




		virtual BOOL	fnGetTVElementMode() = 0;
		virtual void	fnSetTVElementMode(BOOL mode) = 0;

		virtual void	fnGeomExpandFaceSel() = 0;
		virtual void	fnGeomContractFaceSel() = 0;

		virtual BOOL	fnGetAlwaysEdit() = 0;
		virtual void	fnSetAlwaysEdit(BOOL always) = 0;

		virtual BOOL	fnGetShowConnection() = 0;
		virtual void	fnSetShowConnection(BOOL show) = 0;


		virtual BOOL	fnGetFilteredSelected() = 0;
		virtual void	fnSetFilteredSelected(BOOL filter) = 0;

		virtual BOOL	fnGetSnap() = 0;
		virtual void	fnSetSnap(BOOL snap) = 0;

		virtual BOOL	fnGetLock() = 0;
		virtual void	fnSetLock(BOOL snap) = 0;

		virtual void	fnPack(int method,  float spacing, BOOL normalize, BOOL rotate, BOOL fillHoles) = 0;
		virtual void	fnPackNoParams() = 0;
		virtual void	fnPackDialog() = 0;

		virtual int		fnGetTVSubMode() = 0;
		virtual void	fnSetTVSubMode(int mode) = 0;

		virtual BitArray* fnGetSelectedFaces()=0;
		virtual void	fnSelectFaces(BitArray *sel)=0;
		virtual BOOL	fnIsFaceSelected(int index)=0;

		virtual int		fnGetFillMode() = 0;
		virtual void	fnSetFillMode(int mode) = 0;

		virtual void fnMoveSelected(Point3 offset)=0;
		virtual void fnRotateSelected(float angle)=0;
		virtual void fnRotateSelected(float angle, Point3 axis)=0;
		virtual void fnScaleSelected(float scale,int dir)=0;
		virtual void fnScaleSelected(float scale,int dir,Point3 axis)=0;


		virtual BitArray* fnGetSelectedEdges()=0;
		virtual void	fnSelectEdges(BitArray *sel)=0;
		virtual BOOL	fnIsEdgeSelected(int index)=0;



		virtual BOOL	fnGetDisplayOpenEdges() = 0;
		virtual void	fnSetDisplayOpenEdges(BOOL openEdgeDisplay) = 0;
		
		virtual Point3*	fnGetOpenEdgeColor()=0;
		virtual void	fnSetOpenEdgeColor(Point3 color)=0;

		virtual BOOL	fnGetUVEdgeMode() = 0;
		virtual void	fnSetUVEdgeMode(BOOL uvmode) = 0;

		virtual BOOL	fnGetOpenEdgeMode() = 0;
		virtual void	fnSetOpenEdgeMode(BOOL uvmode) = 0;

		virtual void	fnUVEdgeSelect() = 0;
		virtual void	fnOpenEdgeSelect() = 0;

		virtual void	fnVertToEdgeSelect() = 0;
		virtual void	fnVertToFaceSelect() = 0;

		virtual void	fnEdgeToVertSelect() = 0;
		virtual void	fnEdgeToFaceSelect() = 0;

		virtual void	fnFaceToVertSelect() = 0;
		virtual void	fnFaceToEdgeSelect() = 0;


		virtual BOOL	fnGetDisplayHiddenEdges() = 0;
		virtual void	fnSetDisplayHiddenEdges(BOOL hiddenEdgeDisplay) = 0;

		virtual Point3*	fnGetHandleColor()=0;
		virtual void	fnSetHandleColor(Point3 color)=0;

		virtual BOOL	fnGetFreeFormMode() = 0;
		virtual void	fnSetFreeFormMode(BOOL freeFormMode) = 0;

		virtual Point3*	fnGetFreeFormColor()=0;
		virtual void	fnSetFreeFormColor(Point3 color)=0;

		virtual void	fnScaleSelectedXY(float scaleX,float scaleY,Point3 axis)=0;

		virtual void	fnSnapPivot(int pos)=0;
		virtual Point3*	fnGetPivotOffset()=0;
		virtual void	fnSetPivotOffset(Point3 color)=0;
		virtual Point3*	fnGetSelCenter()=0;
		
		virtual void	fnSketch(Tab<int> *indexList, Tab<Point3*> *positionList)=0;
		virtual void	fnSketchNoParams()=0;
		virtual void	fnSketchDialog()=0;
		virtual void	fnSketchReverse()=0;

		virtual int		fnGetHitSize()=0;
		virtual void	fnSetHitSize(int size)=0;

		virtual BOOL	fnGetResetPivotOnSel()=0;
		virtual void	fnSetResetPivotOnSel(BOOL reset)=0;

		virtual BOOL	fnGetPolyMode()=0;
		virtual void	fnSetPolyMode(BOOL pmode)=0;
		virtual void	fnPolySelect()=0;


		virtual BOOL	fnGetAllowSelectionInsideGizmo()=0;
		virtual void	fnSetAllowSelectionInsideGizmo(BOOL select)=0;

		virtual void	fnSetAsDefaults()=0;
		virtual void	fnLoadDefaults()=0;


		virtual void	fnSetSharedColor(Point3 color)=0;
		virtual Point3*	fnGetSharedColor()=0;

		virtual BOOL	fnGetShowShared()=0;
		virtual void	fnSetShowShared(BOOL select)=0;

		virtual void	fnShowIcon(int icon, BOOL show)=0;


		virtual BOOL	fnGetSyncSelectionMode()=0;
		virtual void	fnSetSyncSelectionMode(BOOL sync)=0;

		virtual void	fnSyncTVSelection()=0;
		virtual void	fnSyncGeomSelection()=0;


		virtual Point3*	fnGetBackgroundColor()=0;
		virtual void	fnSetBackgroundColor(Point3 color)=0;

		virtual void	fnUpdateMenuBar() = 0;


		virtual BOOL	fnGetBrightCenterTile()=0;
		virtual void	fnSetBrightCenterTile(BOOL bright)=0;

		virtual BOOL	fnGetBlendToBack()=0;
		virtual void	fnSetBlendToBack(BOOL blend)=0;

		virtual BOOL	fnGetPaintMode()=0;
		virtual void	fnSetPaintMode(BOOL paint)=0;

		virtual int		fnGetPaintSize()=0;
		virtual void	fnSetPaintSize(int size)=0;

		virtual void	fnIncPaintSize()=0;
		virtual void	fnDecPaintSize()=0;


		virtual int		fnGetTickSize()=0;
		virtual void	fnSetTickSize(int size)=0;

//new


		virtual float	fnGetGridSize()=0;
		virtual void	fnSetGridSize(float size)=0;

		virtual BOOL	fnGetGridSnap()=0;
		virtual void	fnSetGridSnap(BOOL snap)=0;
		virtual BOOL	fnGetGridVisible()=0;
		virtual void	fnSetGridVisible(BOOL visible)=0;

		virtual Point3*	fnGetGridColor()=0;
		virtual void	fnSetGridColor(Point3 color)=0;

		virtual float	fnGetGridStr()=0;
		virtual void	fnSetGridStr(float size)=0;


		virtual BOOL	fnGetAutoMap()=0;
		virtual void	fnSetAutoMap(BOOL autoMap)=0;

		virtual float	fnGetFlattenAngle()=0;				//Angle is in degrees
		virtual void	fnSetFlattenAngle(float angle)=0;	//Angle is in degrees

		virtual float	fnGetFlattenSpacing()=0;
		virtual void	fnSetFlattenSpacing(float spacing)=0;

		virtual BOOL	fnGetFlattenNormalize()=0;
		virtual void	fnSetFlattenNormalize(BOOL normalize)=0;

		virtual BOOL	fnGetFlattenRotate()=0;
		virtual void	fnSetFlattenRotate(BOOL rotate)=0;

		virtual BOOL	fnGetFlattenFillHoles()=0;
		virtual void	fnSetFlattenFillHoles(BOOL fillHoles)=0;

		virtual BOOL	fnGetPreventFlattening()=0;
		virtual void	fnSetPreventFlattening(BOOL preventFlattening)=0;

		virtual BOOL	fnGetEnableSoftSelection()=0;
		virtual void	fnSetEnableSoftSelection(BOOL enable)=0;

		virtual BOOL	fnGetApplyToWholeObject() = 0;
		virtual void	fnSetApplyToWholeObject(BOOL wholeObject) = 0;

		virtual void	fnSetVertexPosition2(TimeValue t, int index, Point3 pos, BOOL hold, BOOL update) = 0;
		virtual void	fnRelax(int iteration, float str, BOOL lockEdges, BOOL matchArea) = 0;
		virtual void	fnFit(int iteration, float str) = 0;


	};


//5.1.05
class IUnwrapMod3 : public FPMixinInterface  //interface for R6
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

//TILE STUFF
			FN_0(unwrap_getautobackground,TYPE_BOOL, fnGetAutoBackground);
			VFN_1(unwrap_setautobackground,fnSetAutoBackground,TYPE_BOOL);

//RELAX
//5.1.06
			FN_0(unwrap_getrelaxamount,TYPE_FLOAT, fnGetRelaxAmount);
			VFN_1(unwrap_setrelaxamount, fnSetRelaxAmount,TYPE_FLOAT);			

			FN_0(unwrap_getrelaxiter,TYPE_INT, fnGetRelaxIter);
			VFN_1(unwrap_setrelaxiter, fnSetRelaxIter,TYPE_INT);			

			FN_0(unwrap_getrelaxboundary,TYPE_BOOL, fnGetRelaxBoundary);
			VFN_1(unwrap_setrelaxboundary, fnSetRelaxBoundary,TYPE_BOOL);			

			FN_0(unwrap_getrelaxsaddle,TYPE_BOOL, fnGetRelaxSaddle);
			VFN_1(unwrap_setrelaxsaddle, fnSetRelaxSaddle,TYPE_BOOL);			

			VFN_0(unwrap_relax2, fnRelax2);			
			VFN_0(unwrap_relax2dialog, fnRelax2Dialog);			

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 

//auto background
		virtual BOOL	fnGetAutoBackground()=0;
		virtual void	fnSetAutoBackground(BOOL autoBackground)=0;

//5.1.06

		virtual float	fnGetRelaxAmount() = 0;
		virtual void	fnSetRelaxAmount(float amount) = 0;

		virtual int		fnGetRelaxIter() = 0;
		virtual void	fnSetRelaxIter(int iter) = 0;

		virtual BOOL	fnGetRelaxBoundary() = 0;
		virtual void	fnSetRelaxBoundary(BOOL boundary) = 0;

		virtual BOOL	fnGetRelaxSaddle() = 0;
		virtual void	fnSetRelaxSaddle(BOOL saddle) = 0;

		virtual void	fnRelax2()=0;
		virtual void	fnRelax2Dialog()=0;
	};




#endif // __IUWNRAP__H
