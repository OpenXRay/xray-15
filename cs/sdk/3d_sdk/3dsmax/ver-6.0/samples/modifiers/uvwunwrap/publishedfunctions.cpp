
#include "unwrap.h"
#include "modstack.h"

static UnwrapClassDesc unwrapDesc;


ClassDesc* GetUnwrapModDesc() {return &unwrapDesc;}

//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc unwrap_interface(
    UNWRAP_INTERFACE, _T("unwrap"), 0, &unwrapDesc, FP_MIXIN,
		unwrap_planarmap, _T("planarMap"), 0, TYPE_VOID, 0, 0,
		unwrap_save, _T("save"), 0, TYPE_VOID, 0, 0,
		unwrap_load, _T("load"), 0, TYPE_VOID, 0, 0,

		unwrap_reset, _T("reset"), 0, TYPE_VOID, 0, 0,
		unwrap_edit, _T("edit"), 0, TYPE_VOID, 0, 0,

		unwrap_setMapChannel, _T("setMapChannel"),0, TYPE_VOID, 0, 1,
			_T("mapChannel"), 0, TYPE_INT,
		unwrap_getMapChannel, _T("getMapChannel"),0, TYPE_INT, 0, 0,

		unwrap_setProjectionType, _T("setProjectionType"),0, TYPE_VOID, 0, 1,
			_T("mapChannel"), 0, TYPE_INT,
		unwrap_getProjectionType, _T("getProjectionType"),0, TYPE_INT, 0, 0,

		unwrap_setVC, _T("setVC"),0, TYPE_VOID, 0, 1,
			_T("vertexColor"), 0, TYPE_BOOL,
		unwrap_getVC, _T("getVC"),0, TYPE_BOOL, 0, 0,

		unwrap_move, _T("move"),0, TYPE_VOID, 0, 0,
		unwrap_moveh, _T("moveh"),0, TYPE_VOID, 0, 0,
		unwrap_movev, _T("movev"),0, TYPE_VOID, 0, 0,

		unwrap_rotate, _T("rotate"),0, TYPE_VOID, 0, 0,

		unwrap_scale, _T("scale"),0, TYPE_VOID, 0, 0,
		unwrap_scaleh, _T("scaleh"),0, TYPE_VOID, 0, 0,
		unwrap_scalev, _T("scalev"),0, TYPE_VOID, 0, 0,

		unwrap_mirrorh, _T("mirrorH"),0, TYPE_VOID, 0, 0,
		unwrap_mirrorv, _T("mirrorV"),0, TYPE_VOID, 0, 0,

		unwrap_expandsel, _T("expandSelection"),0, TYPE_VOID, 0, 0,
		unwrap_contractsel, _T("contractSelection"),0, TYPE_VOID, 0, 0,

		unwrap_setFalloffType, _T("setFalloffType"),0, TYPE_VOID, 0, 1,
			_T("falloffType"), 0, TYPE_INT,
		unwrap_getFalloffType, _T("getFalloffType"),0, TYPE_INT, 0, 0,
		unwrap_setFalloffSpace, _T("setFalloffSpace"),0, TYPE_VOID, 0, 1,
			_T("falloffSpace"), 0, TYPE_INT,
		unwrap_getFalloffSpace, _T("getFalloffSpace"),0, TYPE_INT, 0, 0,
		unwrap_setFalloffDist, _T("setFalloffDist"),0, TYPE_VOID, 0, 1,
			_T("falloffDist"), 0, TYPE_FLOAT,
		unwrap_getFalloffDist, _T("getFalloffDist"),0, TYPE_FLOAT, 0, 0,

		unwrap_breakselected, _T("breakSelected"),0, TYPE_VOID, 0, 0,
		unwrap_weld, _T("weld"),0, TYPE_VOID, 0, 0,
		unwrap_weldselected, _T("weldSelected"),0, TYPE_VOID, 0, 0,

		unwrap_updatemap, _T("updateMap"),0, TYPE_VOID, 0, 0,
		unwrap_displaymap, _T("DisplayMap"),0, TYPE_VOID, 0, 1,
			_T("displayMap"), 0, TYPE_BOOL,
		unwrap_ismapdisplayed, _T("IsMapDisplayed"),0, TYPE_BOOL, 0, 0,

		unwrap_setuvspace, _T("setUVSpace"),0, TYPE_VOID, 0, 1,
			_T("UVSpace"), 0, TYPE_INT,
		unwrap_getuvspace, _T("getUVSpace"),0, TYPE_INT, 0, 0,

		unwrap_options, _T("options"),0, TYPE_VOID, 0, 0,

		unwrap_lock, _T("lock"),0, TYPE_VOID, 0, 0,

		unwrap_hide, _T("hide"),0, TYPE_VOID, 0, 0,
		unwrap_unhide, _T("unhide"),0, TYPE_VOID, 0, 0,

		unwrap_freeze, _T("freeze"),0, TYPE_VOID, 0, 0,
		unwrap_thaw, _T("unfreeze"),0, TYPE_VOID, 0, 0,
		unwrap_filterselected, _T("filterselected"),0, TYPE_VOID, 0, 0,

		unwrap_pan, _T("pan"),0, TYPE_VOID, 0, 0,
		unwrap_zoom, _T("zoom"),0, TYPE_VOID, 0, 0,
		unwrap_zoomregion, _T("zoomRegion"),0, TYPE_VOID, 0, 0,
		unwrap_fit, _T("fit"),0, TYPE_VOID, 0, 0,
		unwrap_fitselected, _T("fitselected"),0, TYPE_VOID, 0, 0,

		unwrap_snap, _T("snap"),0, TYPE_VOID, 0, 0,

		unwrap_getcurrentmap, _T("getCurrentMap"),0, TYPE_INT, 0, 0,
		unwrap_setcurrentmap, _T("setCurrentMap"),0, TYPE_VOID, 0, 1,
			_T("map"), 0, TYPE_INT,
		unwrap_numbermaps, _T("numberMaps"),0, TYPE_INT, 0, 0,

		unwrap_getlinecolor, _T("getLineColor"),0, TYPE_POINT3, 0, 0,
		unwrap_setlinecolor, _T("setLineColor"),0, TYPE_VOID, 0, 1,
		_T("color"), 0, TYPE_POINT3,

		unwrap_getselectioncolor, _T("getSelectionColor"),0, TYPE_POINT3, 0, 0,
		unwrap_setselectioncolor, _T("setSelectionColor"),0, TYPE_VOID, 0, 1,
		_T("color"), 0, TYPE_POINT3,



		unwrap_getrenderwidth, _T("getRenderWidth"),0, TYPE_INT, 0, 0,
		unwrap_setrenderwidth, _T("setRenderWidth"),0, TYPE_VOID, 0, 1,
			_T("width"), 0, TYPE_INT,
		unwrap_getrenderheight, _T("getRenderHeight"),0, TYPE_INT, 0, 0,
		unwrap_setrenderheight, _T("setRenderHeight"),0, TYPE_VOID, 0, 1,
			_T("height"), 0, TYPE_INT,

		unwrap_getusebitmapres, _T("getUseBitmapRes"),0, TYPE_BOOL, 0, 0,
		unwrap_setusebitmapres, _T("setUseBitmapRes"),0, TYPE_VOID, 0, 1,
			_T("useRes"), 0, TYPE_BOOL,

		unwrap_getweldtheshold, _T("getWeldThreshold"),0, TYPE_FLOAT, 0, 0,
		unwrap_setweldtheshold, _T("setWeldThreshold"),0, TYPE_VOID, 0, 1,
			_T("height"), 0, TYPE_FLOAT,

		unwrap_getconstantupdate, _T("getConstantUpdate"),0, TYPE_BOOL, 0, 0,
		unwrap_setconstantupdate, _T("setConstantUpdate"),0, TYPE_VOID, 0, 1,
			_T("update"), 0, TYPE_BOOL,

		unwrap_getshowselectedvertices, _T("getShowSelectedVertices"),0, TYPE_BOOL, 0, 0,
		unwrap_setshowselectedvertices, _T("setShowSelectedVertices"),0, TYPE_VOID, 0, 1,
			_T("show"), 0, TYPE_BOOL,

		unwrap_getmidpixelsnap, _T("getMidPixelSnap"),0, TYPE_BOOL, 0, 0,
		unwrap_setmidpixelsnap, _T("setMidPixelSnap"),0, TYPE_VOID, 0, 1,
			_T("snap"), 0, TYPE_BOOL,

		unwrap_getmatid, _T("getMatID"),0, TYPE_INT, 0, 0,
		unwrap_setmatid, _T("setMatID"),0, TYPE_VOID, 0, 1,
			_T("matid"), 0, TYPE_INT,
		unwrap_numbermatids, _T("numberMatIDs"),0, TYPE_INT, 0, 0,

		unwrap_getselectedverts, _T("getSelectedVertices"),0,TYPE_BITARRAY,0,0,
		unwrap_selectverts, _T("selectVertices"),0,TYPE_VOID,0,1,
			_T("selection"),0,TYPE_BITARRAY,

		unwrap_isvertexselected, _T("isVertexSelected"),0,TYPE_BOOL,0,1,
			_T("index"),0,TYPE_INT,

		unwrap_moveselectedvertices, _T("MoveSelectedVertices"),0,TYPE_VOID,0,1,
			_T("offset"),0,TYPE_POINT3,

		unwrap_rotateselectedverticesc, _T("RotateSelectedVerticesCenter"),0,TYPE_VOID,0,1,
			_T("angle"),0,TYPE_FLOAT,
		unwrap_rotateselectedvertices, _T("RotateSelectedVertices"),0,TYPE_VOID,0,2,
			_T("angle"),0,TYPE_FLOAT,
			_T("axis"),0,TYPE_POINT3,
		unwrap_scaleselectedverticesc, _T("ScaleSelectedVerticesCenter"),0,TYPE_VOID,0,2,
			_T("scale"),0,TYPE_FLOAT,
			_T("dir"),0,TYPE_INT,
		unwrap_scaleselectedvertices, _T("ScaleSelectedVertices"),0,TYPE_VOID,0,3,
			_T("scale"),0,TYPE_FLOAT,
			_T("dir"),0,TYPE_INT,
			_T("axis"),0,TYPE_POINT3,



		unwrap_getvertexposition, _T("GetVertexPosition"),0,TYPE_POINT3,0,2,
			_T("time"),0,TYPE_TIMEVALUE,
			_T("index"),0,TYPE_INT,
		unwrap_numbervertices, _T("numberVertices"),0,TYPE_INT,0,0,

		unwrap_movex, _T("moveX"),0,TYPE_VOID,0,1,
			_T("p"),0,TYPE_FLOAT,
		unwrap_movey, _T("moveY"),0,TYPE_VOID,0,1,
			_T("p"),0,TYPE_FLOAT,
		unwrap_movez, _T("moveZ"),0,TYPE_VOID,0,1,
			_T("p"),0,TYPE_FLOAT,

		unwrap_getselectedpolygons,_T("getSelectedPolygons"),0,TYPE_BITARRAY,0,0,
		unwrap_selectpolygons, _T("selectPolygons"),0,TYPE_VOID,0,1,
			_T("selection"),0,TYPE_BITARRAY,
		unwrap_ispolygonselected, _T("isPolygonSelected"),0,TYPE_BOOL,0,1,
			_T("index"),0,TYPE_INT,
		unwrap_numberpolygons, _T("numberPolygons"),0, TYPE_INT, 0, 0,
		unwrap_detachedgeverts, _T("detachEdgeVertices"),0, TYPE_VOID, 0, 0,

		unwrap_fliph, _T("flipHorizontal"),0, TYPE_VOID, 0, 0,
		unwrap_flipv, _T("flipVertical"),0, TYPE_VOID, 0, 0,

		unwrap_getlockaspect, _T("getLockAspect"),0, TYPE_BOOL, 0, 0,
		unwrap_setlockaspect, _T("setLockAspect"),0, TYPE_VOID, 0, 1,
			_T("aspect"), 0, TYPE_BOOL,

		unwrap_getmapscale, _T("getMapScale"),0, TYPE_FLOAT, 0, 0,
		unwrap_setmapscale, _T("setMapScale"),0, TYPE_VOID, 0, 1,
			_T("scale"), 0, TYPE_FLOAT,

		unwrap_getselectionfromface, _T("getSelectionFromFace"),0, TYPE_VOID, 0, 0,

		unwrap_forceupdate, _T("forceUpdate"),0, TYPE_VOID, 0, 1,
			_T("update"), 0, TYPE_BOOL,

		unwrap_zoomtogizmo, _T("zoomToGizmo"),0, TYPE_VOID, 0, 1,
			_T("all"), 0, TYPE_BOOL,

		unwrap_setvertexposition, _T("setVertexPosition"),0,TYPE_VOID,0,3,
			_T("time"), 0, TYPE_TIMEVALUE,
			_T("index"), 0, TYPE_INT,
			_T("pos"), 0, TYPE_POINT3,
		unwrap_markasdead, _T("markAsDead"),0,TYPE_VOID,0,1,
			_T("index"), 0, TYPE_INT,

		unwrap_numberpointsinface, _T("numberPointsInFace"),0,TYPE_INT,0,1,
			_T("index"), 0, TYPE_INT,
			
		unwrap_getvertexindexfromface, _T("getVertexIndexFromFace"),0,TYPE_INT,0,2,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
		unwrap_gethandleindexfromface, _T("getHandleIndexFromFace"),0,TYPE_INT,0,2,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
		unwrap_getinteriorindexfromface, _T("getInteriorIndexFromFace"),0,TYPE_INT,0,2,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,

		unwrap_getvertexgindexfromface, _T("getVertexGeomIndexFromFace"),0,TYPE_INT,0,2,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
		unwrap_gethandlegindexfromface, _T("getHandleGeomIndexFromFace"),0,TYPE_INT,0,2,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
		unwrap_getinteriorgindexfromface, _T("getInteriorGeomIndexFromFace"),0,TYPE_INT,0,2,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,

		unwrap_addpointtoface, _T("setFaceVertex"),0,TYPE_VOID,0,4,
		  	_T("pos"), 0, TYPE_POINT3,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
			_T("sel"), 0, TYPE_BOOL,
		unwrap_addpointtohandle, _T("setFaceHandle"),0,TYPE_VOID,0,4,
		  	_T("pos"), 0, TYPE_POINT3,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
			_T("sel"), 0, TYPE_BOOL,

		unwrap_addpointtointerior, _T("setFaceInterior"),0,TYPE_VOID,0,4,
		  	_T("pos"), 0, TYPE_POINT3,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
			_T("sel"), 0, TYPE_BOOL,

		unwrap_setfacevertexindex, _T("setFaceVertexIndex"),0,TYPE_VOID,0,3,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
			_T("vertexIndex"), 0, TYPE_INT,
		unwrap_setfacehandleindex, _T("setFaceHandleIndex"),0,TYPE_VOID,0,3,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
			_T("vertexIndex"), 0, TYPE_INT,
		unwrap_setfaceinteriorindex, _T("setFaceInteriorIndex"),0,TYPE_VOID,0,3,
			_T("faceIndex"), 0, TYPE_INT,
			_T("ithVertex"), 0, TYPE_INT,
			_T("vertexIndex"), 0, TYPE_INT,
		unwrap_updateview, _T("updateView"),0,TYPE_VOID,0,0,
		unwrap_getfaceselfromstack, _T("getFaceSelectionFromStack"),0,TYPE_VOID,0,0,




      end
      );

			

static FPInterfaceDesc unwrap_interface2(
    UNWRAP_INTERFACE2, _T("unwrap2"), 0, &unwrapDesc, FP_MIXIN,

		//UNFOLD STUFF

		unwrap_selectpolygonsupdate, _T("selectPolygonsUpdate"),0,TYPE_VOID,0,2,
			_T("selection"),0,TYPE_BITARRAY,
			_T("update"),0,TYPE_BOOL,
		unwrap_selectfacesbynormal, _T("selectFacesByNormal"), 0, TYPE_VOID, 0, 3,
			_T("normal"), 0, TYPE_POINT3,
			_T("threshold"), 0, TYPE_FLOAT,
			_T("update"),0,TYPE_BOOL,
		unwrap_selectclusterbynormal, _T("selectClusterByNormal"), 0, TYPE_VOID, 0, 4,
			_T("threshold"), 0, TYPE_FLOAT,
			_T("faceIndexSeed"), 0, TYPE_INT,
			_T("relative"),0,TYPE_BOOL,
			_T("update"),0,TYPE_BOOL,

		unwrap_flattenmap, _T("flattenMap"), 0, TYPE_VOID, 0, 7,
			_T("angleThreshold"), 0, TYPE_FLOAT,
			_T("normalList"), 0, TYPE_POINT3_TAB,
			_T("spacing"), 0, TYPE_FLOAT,
			_T("normalize"), 0, TYPE_BOOL,
			_T("layOutType"), 0, TYPE_INT,
			_T("rotateClusters"), 0, TYPE_BOOL,
			_T("fillHoles"), 0, TYPE_BOOL,

		
		unwrap_normalmap, _T("normalMap"),0, TYPE_VOID, 0, 6,
			_T("normalList"), 0, TYPE_POINT3_TAB,
			_T("spacing"), 0, TYPE_FLOAT,
			_T("normalize"), 0, TYPE_BOOL,
			_T("layOutType"), 0, TYPE_INT,
			_T("rotateClusters"), 0, TYPE_BOOL,
			_T("alignWidth"), 0, TYPE_BOOL,
		unwrap_normalmapnoparams, _T("normalMapNoParams"),0, TYPE_VOID, 0, 0,
		unwrap_normalmapdialog, _T("normalMapDialog"),0, TYPE_VOID, 0, 0,

		unwrap_unfoldmap, _T("unfoldMap"), 0, TYPE_VOID, 0, 1,
			_T("unfoldMethod"), 0, TYPE_INT,
		unwrap_unfoldmapnoparams, _T("unfoldMapNoParams"), 0, TYPE_VOID, 0, 0,
		unwrap_unfoldmapdialog, _T("unfoldMapDialog"), 0, TYPE_VOID, 0, 0,

		unwrap_hideselectedpolygons, _T("hideSelectedPolygons"), 0, TYPE_VOID, 0, 0,
		unwrap_unhideallpolygons, _T("unhideAllPolygons"), 0, TYPE_VOID, 0, 0,


		unwrap_getnormal, _T("getNormal"),0,TYPE_POINT3,0,1,
			_T("faceIndex"), 0, TYPE_INT,
		unwrap_setseedface, _T("setSeedFace"),0, TYPE_VOID, 0, 0,
		unwrap_showvertexconnectionlist, _T("toggleVertexConnection"),0, TYPE_VOID, 0, 0,
//COPYPASTE STUF
		unwrap_copy, _T("copy"),0, TYPE_VOID, 0, 0,
		unwrap_paste, _T("paste"),0, TYPE_VOID, 0, 1,
			_T("rotate"), 0, TYPE_BOOL,

		unwrap_pasteinstance, _T("pasteInstance"),0, TYPE_VOID, 0, 0,

		unwrap_setdebuglevel, _T("setDebugLevel"), 0, TYPE_VOID, 0, 1,
			_T("level"), 0, TYPE_INT,

		unwrap_stitchverts, _T("stitchVerts"), 0, TYPE_VOID, 0, 2,
			_T("align"), 0, TYPE_BOOL,
			_T("bias"), 0, TYPE_FLOAT,

		unwrap_stitchvertsnoparams, _T("stitchVertsNoParams"), 0, TYPE_VOID, 0, 0,
		unwrap_stitchvertsdialog, _T("stitchVertsDialog"), 0, TYPE_VOID, 0, 0,

		unwrap_selectelement, _T("selectElement"), 0, TYPE_VOID, 0, 0,

		unwrap_flattenmapdialog, _T("flattenMapDialog"), 0, TYPE_VOID, 0, 0,
		unwrap_flattenmapnoparams, _T("flattenMapNoParams"), 0, TYPE_VOID, 0, 0,

//TILE STUFF
		unwrap_gettilemap, _T("getTileMap"), 0, TYPE_BOOL, 0, 0,
		unwrap_settilemap, _T("setTileMap"), 0, TYPE_VOID, 0, 1,
			_T("tile"), 0, TYPE_BOOL,

		unwrap_gettilemaplimit, _T("getTileMapLimit"), 0, TYPE_INT, 0, 0,
		unwrap_settilemaplimit, _T("setTileMapLimit"), 0, TYPE_VOID, 0, 1,
			_T("limit"), 0, TYPE_INT,

		unwrap_gettilemapcontrast, _T("getTileMapBrightness"), 0, TYPE_FLOAT, 0, 0,
		unwrap_settilemapcontrast, _T("setTileMapBrightness"), 0, TYPE_VOID, 0, 1,
			_T("contrast"), 0, TYPE_FLOAT,

		unwrap_getshowmap, _T("getShowMap"), 0, TYPE_BOOL, 0, 0,
		unwrap_setshowmap, _T("setShowMap"), 0, TYPE_VOID, 0, 1,
			_T("showMap"), 0, TYPE_BOOL,


		unwrap_setlimitsoftsel, _T("getLimitSoftSel"), 0, TYPE_BOOL, 0, 0,
		unwrap_getlimitsoftsel, _T("setLimitSoftSel"), 0, TYPE_VOID, 0, 1,
			_T("limit"), 0, TYPE_BOOL,

		unwrap_setlimitsoftselrange, _T("getLimitSoftSelRange"), 0, TYPE_INT, 0, 0,
		unwrap_getlimitsoftselrange, _T("setLimitSoftSelRange"), 0, TYPE_VOID, 0, 1,
			_T("range"), 0, TYPE_INT,


		unwrap_getvertexweight, _T("getVertexWeight"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INT,
		unwrap_setvertexweight, _T("setVertexWeight"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("weight"), 0, TYPE_FLOAT,

		unwrap_isweightmodified, _T("isWeightModified"), 0, TYPE_BOOL, 0, 1,
			_T("index"), 0, TYPE_INT,
		unwrap_modifyweight, _T("modifyWeight"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("modify"), 0, TYPE_BOOL,


		unwrap_getgeom_elemmode, _T("getGeomSelectElementMode"), 0, TYPE_BOOL, 0, 0,
		unwrap_setgeom_elemmode, _T("setGeomSelectElementMode"), 0, TYPE_VOID, 0, 1,
			_T("mode"), 0, TYPE_BOOL,


		unwrap_getgeom_planarmode, _T("getGeomPlanarThresholdMode"), 0, TYPE_BOOL, 0, 0,
		unwrap_setgeom_planarmode, _T("setGeomPlanarThresholdMode"), 0, TYPE_VOID, 0, 1,
			_T("mode"), 0, TYPE_BOOL,

		unwrap_getgeom_planarmodethreshold, _T("getGeomPlanarThreshold"), 0, TYPE_FLOAT, 0, 0,
		unwrap_setgeom_planarmodethreshold, _T("setGeomPlanarThreshold"), 0, TYPE_VOID, 0, 1,
			_T("angle"), 0, TYPE_FLOAT,


		unwrap_getwindowx, _T("getWindowX"), 0, TYPE_INT, 0, 0,
		unwrap_getwindowy, _T("getWindowY"), 0, TYPE_INT, 0, 0,
		unwrap_getwindoww, _T("getWindowW"), 0, TYPE_INT, 0, 0,
		unwrap_getwindowh, _T("getWindowH"), 0, TYPE_INT, 0, 0,


		unwrap_getbackfacecull, _T("getIgnoreBackFaceCull"), 0, TYPE_BOOL, 0, 0,
		unwrap_setbackfacecull, _T("setIgnoreBackFaceCull"), 0, TYPE_VOID, 0, 1,
			_T("ignoreBackFaceCull"), 0, TYPE_BOOL,

		unwrap_getoldselmethod, _T("getOldSelMethod"), 0, TYPE_BOOL, 0, 0,
		unwrap_setoldselmethod, _T("setOldSelMethod"), 0, TYPE_VOID, 0, 1,
			_T("oldSelMethod"), 0, TYPE_BOOL,

		unwrap_selectbymatid, _T("selectByMatID"), 0, TYPE_VOID, 0, 1,
			_T("matID"), 0, TYPE_INT,

		unwrap_selectbysg, _T("selectBySG"), 0, TYPE_VOID, 0, 1,
			_T("sg"), 0, TYPE_INT,

		unwrap_gettvelementmode, _T("getTVElementMode"), 0, TYPE_BOOL, 0, 0,
		unwrap_settvelementmode, _T("setTVElementMode"), 0, TYPE_VOID, 0, 1,
			_T("mode"), 0, TYPE_BOOL,


		unwrap_geomexpandsel, _T("expandGeomFaceSelection"),0, TYPE_VOID, 0, 0,
		unwrap_geomcontractsel, _T("contractGeomFaceSelection"),0, TYPE_VOID, 0, 0,


		unwrap_getalwaysedit, _T("getAlwaysEdit"), 0, TYPE_BOOL, 0, 0,
		unwrap_setalwaysedit, _T("setAlwaysEdit"), 0, TYPE_VOID, 0, 1,
			_T("always"), 0, TYPE_BOOL,

		unwrap_getshowvertexconnectionlist, _T("getShowVertexConnections"), 0, TYPE_BOOL, 0, 0,
		unwrap_setshowvertexconnectionlist, _T("setShowVertexConnections"), 0, TYPE_VOID, 0, 1,
			_T("show"), 0, TYPE_BOOL,

		unwrap_getfilterselected, _T("getFilterSelected"), 0, TYPE_BOOL, 0, 0,
		unwrap_setfilterselected, _T("setFilterSelected"), 0, TYPE_VOID, 0, 1,
			_T("filter"), 0, TYPE_BOOL,

		unwrap_getsnap, _T("getSnap"), 0, TYPE_BOOL, 0, 0,
		unwrap_setsnap, _T("setSnap"), 0, TYPE_VOID, 0, 1,
			_T("snap"), 0, TYPE_BOOL,

		unwrap_getlock, _T("getLock"), 0, TYPE_BOOL, 0, 0,
		unwrap_setlock, _T("setLock"), 0, TYPE_VOID, 0, 1,
			_T("lock"), 0, TYPE_BOOL,

		unwrap_pack, _T("pack"), 0, TYPE_VOID, 0, 5,
			_T("method"), 0, TYPE_INT,
			_T("spacing"), 0, TYPE_FLOAT,
			_T("normalize"), 0, TYPE_BOOL,
			_T("rotate"), 0, TYPE_BOOL,
			_T("fillholes"), 0, TYPE_BOOL,

		unwrap_packnoparams, _T("packNoParams"), 0, TYPE_VOID, 0, 0,
		unwrap_packdialog, _T("packDialog"), 0, TYPE_VOID, 0, 0,


		unwrap_gettvsubobjectmode, _T("getTVSubObjectMode"), 0, TYPE_INT, 0, 0,
		unwrap_settvsubobjectmode, _T("setTVSubObjectMode"), 0, TYPE_VOID, 0, 1,
			_T("mode"), 0, TYPE_INT,
		

		unwrap_getselectedfaces, _T("getSelectedFaces"),0,TYPE_BITARRAY,0,0,
		unwrap_selectfaces, _T("selectFaces"),0,TYPE_VOID,0,1,
			_T("selection"),0,TYPE_BITARRAY,

		unwrap_isfaceselected, _T("isFaceSelected"),0,TYPE_BOOL,0,1,
			_T("index"),0,TYPE_INT,

		unwrap_getfillmode, _T("getFillMode"), 0, TYPE_INT, 0, 0,
		unwrap_setfillmode, _T("setFillMode"), 0, TYPE_VOID, 0, 1,
			_T("mode"), 0, TYPE_INT,

		unwrap_moveselected, _T("MoveSelected"),0,TYPE_VOID,0,1,
			_T("offset"),0,TYPE_POINT3,

		unwrap_rotateselectedc, _T("RotateSelectedCenter"),0,TYPE_VOID,0,1,
			_T("angle"),0,TYPE_FLOAT,
		unwrap_rotateselected, _T("RotateSelected"),0,TYPE_VOID,0,2,
			_T("angle"),0,TYPE_FLOAT,
			_T("axis"),0,TYPE_POINT3,
		unwrap_scaleselectedc, _T("ScaleSelectedCenter"),0,TYPE_VOID,0,2,
			_T("scale"),0,TYPE_FLOAT,
			_T("dir"),0,TYPE_INT,
		unwrap_scaleselected, _T("ScaleSelected"),0,TYPE_VOID,0,3,
			_T("scale"),0,TYPE_FLOAT,
			_T("dir"),0,TYPE_INT,
			_T("axis"),0,TYPE_POINT3,

		unwrap_getselectededges, _T("getSelectedEdges"),0,TYPE_BITARRAY,0,0,
		unwrap_selectedges, _T("selectEdges"),0,TYPE_VOID,0,1,
			_T("selection"),0,TYPE_BITARRAY,

		unwrap_isedgeselected, _T("isEdgeSelected"),0,TYPE_BOOL,0,1,
			_T("index"),0,TYPE_INT,


		unwrap_getdisplayopenedge, _T("getDisplayOpenEdges"), 0, TYPE_BOOL, 0, 0,
		unwrap_getdisplayopenedge, _T("setDisplayOpenEdges"), 0, TYPE_VOID, 0, 1,
			_T("displayOpenEdges"), 0, TYPE_BOOL,



		unwrap_getopenedgecolor, _T("getOpenEdgeColor"),0, TYPE_POINT3, 0, 0,
		unwrap_setopenedgecolor, _T("setOpenEdgeColor"),0, TYPE_VOID, 0, 1,
			_T("color"), 0, TYPE_POINT3,

		unwrap_getuvedgemode, _T("getUVEdgeMode"), 0, TYPE_BOOL, 0, 0,
		unwrap_setuvedgemode, _T("setUVEdgeMode"), 0, TYPE_VOID, 0, 1,
			_T("uvEdgeMode"), 0, TYPE_BOOL,

		unwrap_getopenedgemode, _T("getOpenEdgeMode"), 0, TYPE_BOOL, 0, 0,
		unwrap_setopenedgemode, _T("setOpenEdgeMode"), 0, TYPE_VOID, 0, 1,
			_T("uvOpenMode"), 0, TYPE_BOOL,
			

		unwrap_uvedgeselect, _T("uvEdgeSelect"), 0, TYPE_VOID, 0, 0,
		unwrap_openedgeselect, _T("openEdgeSelect"), 0, TYPE_VOID, 0, 0,


		unwrap_selectverttoedge, _T("vertToEdgeSelect"), 0, TYPE_VOID, 0, 0,
		unwrap_selectverttoface, _T("vertToFaceSelect"), 0, TYPE_VOID, 0, 0,

		unwrap_selectedgetovert, _T("edgeToVertSelect"), 0, TYPE_VOID, 0, 0,
		unwrap_selectedgetoface, _T("edgeToFaceSelect"), 0, TYPE_VOID, 0, 0,

		unwrap_selectfacetovert, _T("faceToVertSelect"), 0, TYPE_VOID, 0, 0,
		unwrap_selectfacetoedge, _T("faceToEdgeSelect"), 0, TYPE_VOID, 0, 0,

		unwrap_getdisplayhiddenedge, _T("getDisplayHiddenEdges"), 0, TYPE_BOOL, 0, 0,
		unwrap_setdisplayhiddenedge, _T("setDisplayHiddenEdges"), 0, TYPE_VOID, 0, 1,
			_T("displayHiddenEdges"), 0, TYPE_BOOL,

		unwrap_gethandlecolor, _T("getHandleColor"),0, TYPE_POINT3, 0, 0,
		unwrap_sethandlecolor, _T("setHandleColor"),0, TYPE_VOID, 0, 1,
		_T("color"), 0, TYPE_POINT3,

		unwrap_getfreeformmode, _T("getFreeFormMode"), 0, TYPE_BOOL, 0, 0,
		unwrap_setfreeformmode, _T("setFreeFormMode"), 0, TYPE_VOID, 0, 1,
			_T("freeFormMode"), 0, TYPE_BOOL,

		unwrap_getfreeformcolor, _T("getFreeFormColor"),0, TYPE_POINT3, 0, 0,
		unwrap_setfreeformcolor, _T("setFreeFormColor"),0, TYPE_VOID, 0, 1,
		_T("color"), 0, TYPE_POINT3,

		unwrap_scaleselectedxy, _T("ScaleSelectedXY"),0,TYPE_VOID,0,3,
			_T("scaleX"),0,TYPE_FLOAT,
			_T("scaleY"),0,TYPE_FLOAT,
			_T("axis"),0,TYPE_POINT3,

		unwrap_snappivot, _T("snapPivot"),0,TYPE_VOID,0,1,
			_T("pos"),0,TYPE_INT,

		unwrap_getpivotoffset, _T("getPivotOffset"),0, TYPE_POINT3, 0, 0,
		unwrap_setpivotoffset, _T("setPivotOffset"),0, TYPE_VOID, 0, 1,
			_T("offset"), 0, TYPE_POINT3,
		unwrap_getselcenter, _T("getSelCenter"),0, TYPE_POINT3, 0, 0,

		unwrap_sketch, _T("sketch"),0, TYPE_VOID, 0, 2,
			_T("indexList"),0,TYPE_INT_TAB,
			_T("positionList"),0,TYPE_POINT3_TAB,
		unwrap_sketchnoparams, _T("sketchNoParams"),0, TYPE_VOID, 0, 0,
		unwrap_sketchdialog, _T("sketchDialog"),0, TYPE_VOID, 0, 0,
		unwrap_sketchreverse, _T("sketchReverse"),0, TYPE_VOID, 0, 0,


		unwrap_gethitsize, _T("getHitSize"), 0, TYPE_INT, 0, 0,
		unwrap_sethitsize, _T("SetHitSize"), 0, TYPE_VOID, 0, 1,
			_T("size"), 0, TYPE_INT,


		unwrap_getresetpivotonsel, _T("getResetPivotOnSelection"), 0, TYPE_BOOL, 0, 0,
		unwrap_setresetpivotonsel, _T("SetResetPivotOnSelection"), 0, TYPE_VOID, 0, 1,
			_T("reset"), 0, TYPE_BOOL,

		unwrap_getpolymode, _T("getPolygonMode"), 0, TYPE_BOOL, 0, 0,
		unwrap_setpolymode, _T("setPolygonMode"), 0, TYPE_VOID, 0, 1,
			_T("mode"), 0, TYPE_BOOL,
		unwrap_polyselect, _T("PolygonSelect"), 0, TYPE_VOID, 0, 0,
			
		
		
		unwrap_getselectioninsidegizmo, _T("getAllowSelectionInsideGizmo"), 0, TYPE_BOOL, 0, 0,
		unwrap_setselectioninsidegizmo, _T("SetAllowSelectionInsideGizmo"), 0, TYPE_VOID, 0, 1,
			_T("select"), 0, TYPE_BOOL,



		unwrap_setasdefaults, _T("SaveCurrentSettingsAsDefault"), 0, TYPE_VOID, 0, 0,
		unwrap_loaddefaults, _T("LoadDefault"), 0, TYPE_VOID, 0, 0,


		
		unwrap_getshowshared, _T("getShowShared"), 0, TYPE_BOOL, 0, 0,
		unwrap_setshowshared, _T("setShowShared"), 0, TYPE_VOID, 0, 1,
			_T("select"), 0, TYPE_BOOL,


		unwrap_getsharedcolor, _T("getSharedColor"),0, TYPE_POINT3, 0, 0,
		unwrap_setsharedcolor, _T("setSharedColor"),0, TYPE_VOID, 0, 1,
			_T("color"), 0, TYPE_POINT3,


		unwrap_showicon, _T("showIcon"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("show"), 0, TYPE_BOOL,


		unwrap_getsyncselectionmode, _T("getSyncSelectionMode"), 0, TYPE_BOOL, 0, 0,
		unwrap_setsyncselectionmode, _T("setSyncSelectionMode"), 0, TYPE_VOID, 0, 1,
			_T("sync"), 0, TYPE_BOOL,


		unwrap_synctvselection, _T("syncTVSelection"), 0, TYPE_VOID, 0, 0,
		unwrap_syncgeomselection, _T("syncGeomSelection"), 0, TYPE_VOID, 0, 0,

		unwrap_getbackgroundcolor, _T("getBackgroundColor"),0, TYPE_POINT3, 0, 0,
		unwrap_setbackgroundcolor, _T("setBackgroundColor"),0, TYPE_VOID, 0, 1,
			_T("color"), 0, TYPE_POINT3,

		unwrap_updatemenubar, _T("updateMenuBar"),0, TYPE_VOID, 0, 0,


		unwrap_getbrightcentertile, _T("getBrightnessAffectsCenterTile"), 0, TYPE_BOOL, 0, 0,
		unwrap_setbrightcentertile, _T("setBrightnessAffectsCenterTile"), 0, TYPE_VOID, 0, 1,
			_T("bright"), 0, TYPE_BOOL,

		unwrap_getblendtoback, _T("getBlendTileToBackground"), 0, TYPE_BOOL, 0, 0,
		unwrap_setblendtoback, _T("setBlendTileToBackground"), 0, TYPE_VOID, 0, 1,
			_T("blend"), 0, TYPE_BOOL,

		unwrap_getpaintmode, _T("getPaintSelectMode"), 0, TYPE_BOOL, 0, 0,
		unwrap_setpaintmode, _T("setPaintSelectMode"), 0, TYPE_VOID, 0, 1,
			_T("paint"), 0, TYPE_BOOL,

		unwrap_getpaintsize, _T("getPaintSelectSize"), 0, TYPE_INT, 0, 0,
		unwrap_setpaintsize, _T("setPaintSelectSize"), 0, TYPE_VOID, 0, 1,
			_T("size"), 0, TYPE_INT,

		unwrap_incpaintsize, _T("PaintSelectIncSize"), 0, TYPE_VOID, 0, 0,
		unwrap_decpaintsize, _T("PaintSelectDecSize"), 0, TYPE_VOID, 0, 0,


		unwrap_getticksize, _T("getTickSize"), 0, TYPE_INT, 0, 0,
		unwrap_setticksize, _T("setTickSize"), 0, TYPE_VOID, 0, 1,
			_T("size"), 0, TYPE_INT,


//new
		unwrap_getgridsize, _T("getGridSize"), 0, TYPE_FLOAT, 0, 0,
		unwrap_setgridsize, _T("setGridSize"), 0, TYPE_VOID, 0, 1,
			_T("size"), 0, TYPE_FLOAT,

		unwrap_getgridsnap, _T("getGridSnap"), 0, TYPE_BOOL, 0, 0,
		unwrap_setgridsnap, _T("setGridSnap"), 0, TYPE_VOID, 0, 1,
			_T("snap"), 0, TYPE_BOOL,
		unwrap_getgridvisible, _T("getGridVisible"), 0, TYPE_BOOL, 0, 0,
		unwrap_setgridvisible, _T("setGridVisible"), 0, TYPE_VOID, 0, 1,
			_T("visible"), 0, TYPE_BOOL,

		unwrap_getgridcolor, _T("getGridColor"),0, TYPE_POINT3, 0, 0,
		unwrap_setgridcolor, _T("setGridColor"),0, TYPE_VOID, 0, 1,
			_T("color"), 0, TYPE_POINT3,

		unwrap_getgridstr, _T("getGridStr"), 0, TYPE_FLOAT, 0, 0,
		unwrap_setgridstr, _T("setGridStr"), 0, TYPE_VOID, 0, 1,
			_T("str"), 0, TYPE_FLOAT,

		unwrap_getautomap, _T("getAutoBackground"), 0, TYPE_BOOL, 0, 0,
		unwrap_setautomap, _T("setAutoBackground"), 0, TYPE_VOID, 0, 1,
			_T("enable"), 0, TYPE_BOOL,


		unwrap_getflattenangle, _T("getFlattenAngle"), 0, TYPE_FLOAT, 0, 0,
		unwrap_setflattenangle, _T("setFlattenAngle"), 0, TYPE_VOID, 0, 1,
			_T("angle"), 0, TYPE_FLOAT,

		unwrap_getflattenspacing, _T("getFlattenSpacing"), 0, TYPE_FLOAT, 0, 0,
		unwrap_setflattenspacing, _T("setFlattenSpacing"), 0, TYPE_VOID, 0, 1,
			_T("spacing"), 0, TYPE_FLOAT,

		unwrap_getflattennormalize, _T("getFlattenNormalize"), 0, TYPE_BOOL, 0, 0,
		unwrap_setflattennormalize, _T("setFlattenNormalize"), 0, TYPE_VOID, 0, 1,
			_T("normalize"), 0, TYPE_BOOL,

		unwrap_getflattenrotate, _T("getFlattenRotate"), 0, TYPE_BOOL, 0, 0,
		unwrap_setflattenrotate, _T("setFlattenRotate"), 0, TYPE_VOID, 0, 1,
			_T("rotate"), 0, TYPE_BOOL,

		unwrap_getflattenfillholes, _T("getFlattenFillHoles"), 0, TYPE_BOOL, 0, 0,
		unwrap_setflattenfillholes, _T("setFlattenFillHoles"), 0, TYPE_VOID, 0, 1,
			_T("fillHoles"), 0, TYPE_BOOL,

		unwrap_getpreventflattening, _T("getPreventFlattening"), 0, TYPE_BOOL, 0, 0,
		unwrap_setpreventflattening, _T("setPreventFlattening"), 0, TYPE_VOID, 0, 1,
			_T("preventFlattening"), 0, TYPE_BOOL,

		unwrap_getenablesoftselection, _T("getEnableSoftSelection"), 0, TYPE_BOOL, 0, 0,
		unwrap_setenablesoftselection, _T("setEnableSoftSelection"), 0, TYPE_VOID, 0, 1,
			_T("enable"), 0, TYPE_BOOL,

		unwrap_getapplytowholeobject, _T("getApplyToWholeObject"), 0, TYPE_BOOL, 0, 0,
		unwrap_setapplytowholeobject, _T("setApplyToWholeObject"), 0, TYPE_VOID, 0, 1,
			_T("whole"), 0, TYPE_BOOL,


		unwrap_setvertexposition2, _T("setVertexPosition2"),0,TYPE_VOID,0,5,
			_T("time"), 0, TYPE_TIMEVALUE,
			_T("index"), 0, TYPE_INT,
			_T("pos"), 0, TYPE_POINT3,
			_T("hold"), 0, TYPE_BOOL,
			_T("update"), 0, TYPE_BOOL,


		unwrap_relax, _T("relax"),0,TYPE_VOID,0,4,
			_T("iterations"), 0, TYPE_INT,
			_T("strength"), 0, TYPE_FLOAT,
			_T("lockEdges"), 0, TYPE_BOOL,
			_T("matchArea"), 0, TYPE_BOOL,

		unwrap_fitrelax, _T("fitRelax"),0,TYPE_VOID,0,2,
			_T("iterations"), 0, TYPE_INT,
			_T("strength"), 0, TYPE_FLOAT,


      end
      );

//5.1.05
static FPInterfaceDesc unwrap_interface3(
    UNWRAP_INTERFACE3, _T("unwrap3"), 0, &unwrapDesc, FP_MIXIN,

		//UNFOLD STUFF

		unwrap_getautobackground, _T("getAutoBackground"),0,TYPE_BOOL,0,0,
		unwrap_setautobackground, _T("setAutoBackground"), 0, TYPE_VOID, 0, 1,
			_T("autoBackground"), 0, TYPE_BOOL,

		unwrap_getrelaxamount, _T("getRelaxAmount"),0,TYPE_FLOAT,0,0,
		unwrap_setrelaxamount, _T("setRelaxAmount"), 0, TYPE_VOID, 0, 1,
			_T("amount"), 0, TYPE_FLOAT,

		unwrap_getrelaxiter, _T("getRelaxIteration"),0,TYPE_INT,0,0,
		unwrap_setrelaxiter, _T("setRelaxIteration"), 0, TYPE_VOID, 0, 1,
			_T("amount"), 0, TYPE_INT,


		unwrap_getrelaxboundary, _T("getRelaxBoundary"),0,TYPE_BOOL,0,0,
		unwrap_setrelaxboundary, _T("setRelaxBoundary"), 0, TYPE_VOID, 0, 1,
			_T("boundary"), 0, TYPE_BOOL,

		unwrap_getrelaxsaddle, _T("getRelaxSaddle"),0,TYPE_BOOL,0,0,
		unwrap_setrelaxsaddle, _T("setRelaxSaddle"), 0, TYPE_VOID, 0, 1,
			_T("saddle"), 0, TYPE_BOOL,


		unwrap_relax2, _T("relax2"), 0, TYPE_VOID, 0, 0,
		unwrap_relax2dialog, _T("relax2dialog"), 0, TYPE_VOID, 0, 0,

		end
		);
	
	
//  Get Descriptor method for Mixin Interface
//  *****************************************


void *UnwrapClassDesc::Create(BOOL loading)
		{
		AddInterface(&unwrap_interface);
		AddInterface(&unwrap_interface2);
		//5.1.05
		AddInterface(&unwrap_interface3);
		return new UnwrapMod;
		}

FPInterfaceDesc* IUnwrapMod::GetDesc()
	{
	 return &unwrap_interface;
	}

FPInterfaceDesc* IUnwrapMod2::GetDesc()
	{
	 return &unwrap_interface2;
	}
//5.1.05
FPInterfaceDesc* IUnwrapMod3::GetDesc()
	{
	 return &unwrap_interface3;
	}

void UnwrapMod::ActivateActionTable()
	{
	pCallback = new UnwrapActionCallback();
	pCallback->SetUnwrap(this);

	if  (GetCOREInterface()->GetActionManager()->ActivateActionTable(pCallback, kUnwrapActions) )
		{
		ActionTable *actionTable =  GetCOREInterface()->GetActionManager()->FindTable(kUnwrapActions);
		if (actionTable)
			{
			int actionCount = NumElements(spActions)/3;
			for (int i =0; i < actionCount; i++)
				{
				int id = spActions[i*3];
				UnwrapAction *action =  (UnwrapAction *) actionTable->GetAction(spActions[i*3]);
				if (action)
					{
					action->SetUnwrap(this);
					}

				}

			}
		}

	}

//action Table methods
void UnwrapMod::DeActivateActionTable()
	{
	ActionTable *actionTable =  GetCOREInterface()->GetActionManager()->FindTable(kUnwrapActions);
	if (actionTable)
		{
		int actionCount = NumElements(spActions)/3;
		for (int i =0; i < actionCount; i++)
			{
			UnwrapAction *action =  (UnwrapAction *) actionTable->GetAction(spActions[i*3]);
			if (action)
				{
				action->SetUnwrap(NULL);
				}
			}
		}
	GetCOREInterface()->GetActionManager()->DeactivateActionTable(pCallback, kUnwrapActions); 
	delete pCallback;
	pCallback = NULL;

	}




BOOL UnwrapMod::WtIsChecked(int id)
	{
	BOOL iret = FALSE;
	switch (id)
		{
		case ID_FREEFORMMODE:
			if (mode ==ID_FREEFORMMODE)
				{
				iret = TRUE; 
				}
			break;
		case ID_MOVE:
			if ((mode ==ID_UNWRAP_MOVE) || (mode ==  ID_TOOL_MOVE))
				{
				iret = TRUE; 
				}
			break;
		case ID_ROTATE:
			if ((mode ==ID_UNWRAP_ROTATE) || (mode ==  ID_TOOL_ROTATE))
				{
				iret = TRUE; 
				}
			break;
		case ID_SCALE:
			if ((mode ==ID_UNWRAP_SCALE) || (mode ==  ID_TOOL_SCALE))
				{
				iret = TRUE; 
				}
			break;
		case ID_WELD:
			if ((mode ==ID_UNWRAP_WELD) || (mode ==  ID_TOOL_WELD))
				{
				iret = TRUE; 
				}
			break;
		case ID_PAN:
			if ((mode ==ID_UNWRAP_PAN) || (mode ==  ID_TOOL_PAN))
				{
				iret = TRUE; 
				}
			break;
		case ID_ZOOM:
			if ((mode ==ID_UNWRAP_ZOOM) || (mode ==  ID_TOOL_ZOOM))
				{
				iret = TRUE; 
				}
			break;
		case ID_ZOOMREGION:
			if ((mode ==ID_UNWRAP_ZOOMREGION) || (mode ==  ID_TOOL_ZOOMREG))
				{
				iret = TRUE; 
				}
			break;

		case ID_LOCK:
			iret = lockSelected;
			break;
		case ID_FILTERSELECTED:
			iret = filterSelectedFaces;
			break;

		case ID_SHOWMAP:
			iret = showMap;
			break;

		case ID_SNAP:
			iret = pixelSnap;
			break;
		case ID_LIMITSOFTSEL:
			iret = limitSoftSel;
			break;
		case ID_GEOMELEMMODE:
			iret = geomElemMode;
			break;
		case ID_PLANARMODE:
			{
			iret = fnGetGeomPlanarMode();
			break;
			}
		case ID_IGNOREBACKFACE:
			{
			iret = fnGetBackFaceCull();
			break;
			}
		case ID_ELEMENTMODE:
			{
			iret = fnGetTVElementMode();
			break;
			}
		case ID_SHOWVERTCONNECT:
			{
			iret = fnGetShowConnection();
			break;
			}
		case ID_TV_VERTMODE:
			{
			if (fnGetTVSubMode() == TVVERTMODE)
				iret = TRUE;
			else iret = FALSE;
			break;
			}
		case ID_TV_EDGEMODE:
			{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;
			break;
			}
		case ID_TV_FACEMODE:
			{
			if (fnGetTVSubMode() == TVFACEMODE)
				iret = TRUE;
			else iret = FALSE;
			break;
			}
		case ID_UVEDGEMODE:
			{
			if (fnGetUVEdgeMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_OPENEDGEMODE:
			{
			if (fnGetOpenEdgeMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_DISPLAYHIDDENEDGES:
			{
			if (fnGetDisplayHiddenEdges())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_RESETPIVOTONSEL:
			{
			if (fnGetResetPivotOnSel())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_SKETCH:
			{
			if (mode == ID_SKETCHMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_SHOWHIDDENEDGES:
			{
			if (fnGetDisplayHiddenEdges())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_POLYGONMODE:
			{
			if (fnGetPolyMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_ALLOWSELECTIONINSIDEGIZMO:
			{
			if (fnGetAllowSelectionInsideGizmo())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_SHOWSHARED:
			{
			if (fnGetShowShared())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_ALWAYSEDIT:
			{
			if (fnGetAlwaysEdit())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_SYNCSELMODE:
			{
			if (fnGetSyncSelectionMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_SHOWOPENEDGES:
			{
			if (fnGetDisplayOpenEdges())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_BRIGHTCENTERTILE:
			{
			if (fnGetBrightCenterTile())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_BLENDTOBACK:
			{
			if (fnGetBlendToBack())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}

		case ID_PAINTSELECTMODE:
			{
			if (fnGetPaintMode())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}


		case ID_GRIDSNAP:
			{
			if (fnGetGridSnap())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}

		case ID_GRIDVISIBLE:
			{
			if (fnGetGridVisible())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_PREVENTREFLATTENING:
			{
			if (fnGetPreventFlattening())
				iret = TRUE;
			else iret = FALSE;			
			break;
			}

		}

	return iret;
	}
BOOL UnwrapMod::WtIsEnabled(int id)
	{
	BOOL iret = TRUE;

	switch (id)
		{
		case ID_PASTE:
			iret = copyPasteBuffer.CanPaste();
			break;
		case ID_PASTEINSTANCE:
			iret = copyPasteBuffer.CanPasteInstance(this);
			break;
		case ID_UVEDGEMODE:
			{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_OPENEDGEMODE:
			{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_UVEDGESELECT:
			{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_OPENEDGESELECT:
			{
			if (fnGetTVSubMode() == TVEDGEMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
			}

		case ID_SNAPCENTER:
		case ID_SNAPLOWERLEFT:
		case ID_SNAPLOWERCENTER:
		case ID_SNAPLOWERRIGHT:
		case ID_SNAPUPPERLEFT:
		case ID_SNAPUPPERCENTER:
		case ID_SNAPUPPERRIGHT:
		case ID_SNAPRIGHTCENTER:
		case ID_SNAPLEFTCENTER:

			{
			if (mode == ID_FREEFORMMODE)
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_SKETCH:
		case ID_SKETCHDIALOG:
			{
			if (fnGetTVSubMode() == TVVERTMODE) 
				iret = TRUE;
			else iret = FALSE;
			break;
			}
		case ID_SKETCHREVERSE:
			{
			if ((sketchSelMode == SKETCH_SELCURRENT) && (mode == ID_SKETCHMODE))
				iret = TRUE;
			else iret = FALSE;
			break;
			}
		case ID_POLYGONMODE:
		case ID_POLYGONSELECT:

			{
			if ((fnGetTVSubMode() == TVFACEMODE) && (objType == IS_MESH))
				iret = TRUE;
			else iret = FALSE;			
			break;
			}
		case ID_PAINTSELECTINC:
			{
			if (fnGetPaintMode() && (fnGetPaintSize() < 15))
				iret = TRUE;
			else iret = FALSE;
			break;
			}
		case ID_PAINTSELECTDEC:
			{
			if (fnGetPaintMode() && (fnGetPaintSize() > 1))
				iret = TRUE;
			else iret = FALSE;
			break;
			}
		case ID_FLATTENMAP:
		case ID_FLATTENMAPDIALOG:
			{
			if (fnGetPreventFlattening())
				iret = FALSE;
			else iret = TRUE;
			break;
			}
		case ID_WELD:
			if (fnGetTVSubMode() == TVFACEMODE) 
				iret = FALSE; 
			else iret = TRUE; 
			break;

		case ID_RELAX:
		case ID_RELAXDIALOG:
			if (fnGetTVSubMode() == TVVERTMODE) 
				iret = TRUE; 
			else iret = FALSE; 
			break;

		}		
	return iret;
	}
BOOL UnwrapMod::WtExecute(int id)
	{
	BOOL iret = FALSE;
	if (floaterWindowActive)
		{
		switch (id)
			{

			case ID_INCSEL:
				fnExpandSelection();
				iret = TRUE;
				break;
			case ID_DECSEL:
				fnContractSelection();
				iret = TRUE;
				break;


			case ID_LOCK:
				fnLock();
				iret = TRUE;
				break;			


			case ID_PAN:
				fnPan();
				iret = TRUE;
				break;			
			case ID_ZOOM:
				fnZoom();
				iret = TRUE;
				break;			
			case ID_ZOOMREGION:
				fnZoomRegion();
				iret = TRUE;
				break;			
			case ID_FIT:
				fnFit();
				iret = TRUE;
				break;			
			case ID_FITSELECTED:
				fnFitSelected();
				iret = TRUE;
				break;			


			}
		}
	switch (id)
		{
		case ID_PLANAR_MAP:
			fnPlanarMap();
			iret = TRUE;
			break;
		case ID_SAVE:
			fnSave();
			iret = TRUE;
			break;
		case ID_LOAD:
			fnLoad();
			iret = TRUE;
			break;
		case ID_RESET:
			fnReset();
			iret = TRUE;
			break;
		case ID_EDIT:
			fnEdit();
			iret = TRUE;
			break;
		case ID_PREVENTREFLATTENING:
			{
			if (fnGetPreventFlattening())
				fnSetPreventFlattening(FALSE);
			else fnSetPreventFlattening(TRUE);	
			iret = TRUE;
			break;
			}
		case ID_ZOOMTOGIZMO:
			fnZoomToGizmo(FALSE);
			iret = TRUE;
			break;
		case ID_FREEFORMMODE:
			fnSetFreeFormMode(TRUE);
			iret = TRUE;
			break;
		case ID_MOVE:
			fnMove();
			iret = TRUE;
			break;
		case ID_MOVEH:
			fnMoveH();
			iret = TRUE;
			break;
		case ID_MOVEV:
			fnMoveV();
			iret = TRUE;
			break;
		case ID_ROTATE:
			fnRotate();
			iret = TRUE;
			break;
		case ID_SCALE:
			fnScale();
			iret = TRUE;
			break;
		case ID_SCALEH:
			fnScaleH();
			iret = TRUE;
			break;
		case ID_SCALEV:
			fnScaleV();
			iret = TRUE;
			break;
		case ID_MIRRORH:
			fnMirrorH();
			iret = TRUE;
			break;
		case ID_MIRRORV:
			fnMirrorV();
			iret = TRUE;
			break;

		case ID_FLIPH:
			fnFlipH();
			iret = TRUE;
			break;
		case ID_FLIPV:
			fnFlipV();
			iret = TRUE;
			break;
		case ID_BREAK:
			fnBreakSelected();
			iret = TRUE;
			break;
		case ID_WELD:
			fnWeld();
			iret = TRUE;
			break;
		case ID_WELD_SELECTED:
			fnWeldSelected();
			iret = TRUE;
			break;
		case ID_UPDATEMAP:
			fnUpdatemap();
			iret = TRUE;
			break;			
		case ID_OPTIONS:
			fnOptions();
			iret = TRUE;
			break;	
		case ID_HIDE:
			fnHide();
			iret = TRUE;
			break;			
		case ID_UNHIDE:
			fnUnhide();
			iret = TRUE;
			break;			
		case ID_FREEZE:
			fnFreeze();
			iret = TRUE;
			break;			
		case ID_UNFREEZE:
			fnThaw();
			iret = TRUE;
			break;			
		case ID_FILTERSELECTED:
			fnFilterSelected();
			iret = TRUE;
			break;			
			
		case ID_SNAP:
			fnSnap();
			iret = TRUE;
			break;			
		case ID_FACETOVERTEX:
			iret = TRUE;
			fnGetSelectionFromFace();
			break;			

		case ID_DETACH:
			fnDetachEdgeVerts();
			iret = TRUE;
			break;


		case ID_GETFACESELFROMSTACK:
			fnGetFaceSelFromStack();
			iret = TRUE;
			break;
		case ID_STITCH:
			fnStitchVertsNoParams();
			iret = TRUE;
			break;
		case ID_STITCHDIALOG:
			fnStitchVertsDialog();
			iret = TRUE;
			break;
		case ID_SHOWMAP:
			fnShowMap();
			iret = TRUE;
			break;

		case ID_NORMALMAP:
			fnNormalMapNoParams();
			iret = TRUE;
			break;
		case ID_NORMALMAPDIALOG:
			fnNormalMapDialog();
			iret = TRUE;
			break;

		case ID_FLATTENMAP:
			fnFlattenMapNoParams();
			iret = TRUE;
			break;
		case ID_FLATTENMAPDIALOG:
			fnFlattenMapDialog();
			iret = TRUE;
			break;

		case ID_UNFOLDMAP:
			fnUnfoldSelectedPolygonsNoParams();
			iret = TRUE;
			break;
		case ID_UNFOLDMAPDIALOG:
			fnUnfoldSelectedPolygonsDialog();
			iret = TRUE;
			break;

		case ID_COPY:
			fnCopy();
			iret = TRUE;
			break;

		case ID_PASTE:
			fnPaste(TRUE);
			iret = TRUE;
			break;
		case ID_PASTEINSTANCE:
			fnPasteInstance();
			iret = TRUE;
			break;

		case ID_LIMITSOFTSEL:
			{
			BOOL limit = fnGetLimitSoftSel();

			if (limit)
				fnSetLimitSoftSel(FALSE);
			else fnSetLimitSoftSel(TRUE);
			iret = TRUE;
			break;
			}

		case ID_LIMITSOFTSEL1:
			fnSetLimitSoftSelRange(1);
			iret = TRUE;
			break;
		case ID_LIMITSOFTSEL2:
			fnSetLimitSoftSelRange(2);
			iret = TRUE;
			break;
		case ID_LIMITSOFTSEL3:
			fnSetLimitSoftSelRange(3);
			iret = TRUE;
			break;
		case ID_LIMITSOFTSEL4:
			fnSetLimitSoftSelRange(4);
			iret = TRUE;
			break;
		case ID_LIMITSOFTSEL5:
			fnSetLimitSoftSelRange(5);
			iret = TRUE;
			break;
		case ID_LIMITSOFTSEL6:
			fnSetLimitSoftSelRange(6);
			iret = TRUE;
			break;
		case ID_LIMITSOFTSEL7:
			fnSetLimitSoftSelRange(7);
			iret = TRUE;
			break;
		case ID_LIMITSOFTSEL8:
			fnSetLimitSoftSelRange(8);
			iret = TRUE;
			break;
		case ID_GEOMELEMMODE:
			{
			BOOL mode = fnGetGeomElemMode();
			if (mode)
				fnSetGeomElemMode(FALSE);
			else fnSetGeomElemMode(TRUE);
			iret = TRUE;
			break;
			}
		case ID_PLANARMODE:
			{
			BOOL pmode = fnGetGeomPlanarMode();

			if (pmode)
				fnSetGeomPlanarMode(FALSE);
			else fnSetGeomPlanarMode(TRUE);
			iret = TRUE;
			break;
			}
		case ID_IGNOREBACKFACE:
			{
			BOOL backface = fnGetBackFaceCull();

			if (backface)
				fnSetBackFaceCull(FALSE);
			else fnSetBackFaceCull(TRUE);
			iret = TRUE;
			break;
			}
		case ID_ELEMENTMODE:
			{
			BOOL mode = fnGetTVElementMode();

			if (mode)
				fnSetTVElementMode(FALSE);
			else fnSetTVElementMode(TRUE);
			iret = TRUE;
			break;
			}
		case ID_GEOMEXPANDFACESEL:
			fnGeomExpandFaceSel();
			iret = TRUE;
			break;
		case ID_GEOMCONTRACTFACESEL:
	 		fnGeomContractFaceSel();
			iret = TRUE;
			break;

		case ID_SHOWVERTCONNECT:
			{
			BOOL show = fnGetShowConnection();
			if (show)
				fnSetShowConnection(FALSE);
			else fnSetShowConnection(TRUE);
			iret = TRUE;
			break;
			}
		case ID_TV_VERTMODE:
			{
			fnSetTVSubMode(1);
			iret = TRUE;
			break;
			}
		case ID_TV_EDGEMODE:
			{
			fnSetTVSubMode(2);
			iret = TRUE;
			break;
			}
		case ID_TV_FACEMODE:
			{
			fnSetTVSubMode(3);
			iret = TRUE;
			break;
			}
		case ID_PACK:
			fnPackNoParams();
			iret = TRUE;
			break;
		case ID_PACKDIALOG:
			fnPackDialog();
			iret = TRUE;
			break;
		case ID_UVEDGEMODE:
			{
			if (fnGetUVEdgeMode())
				fnSetUVEdgeMode(FALSE);
			else fnSetUVEdgeMode(TRUE);
			iret = TRUE;
			break;
			}
		case ID_OPENEDGEMODE:
			{
			if (fnGetOpenEdgeMode())
				fnSetOpenEdgeMode(FALSE);
			else fnSetOpenEdgeMode(TRUE);
			iret = TRUE;
			break;
			}
		case ID_UVEDGESELECT:
			{
			fnUVEdgeSelect();
			iret = TRUE;
			break;
			}
		case ID_OPENEDGESELECT:
			{
			fnOpenEdgeSelect();
			iret = TRUE;
			break;
			}

		case ID_VERTTOEDGESELECT:
			{
			fnVertToEdgeSelect();
			fnSetTVSubMode(2);
			MoveScriptUI();
			iret = TRUE;
			break;
			}
		case ID_VERTTOFACESELECT:
			{
			fnVertToFaceSelect();
			fnSetTVSubMode(3);
			MoveScriptUI();
			iret = TRUE;
			break;
			}

		case ID_EDGETOVERTSELECT:
			{
			fnEdgeToVertSelect();
			fnSetTVSubMode(1);
			RebuildDistCache();
			MoveScriptUI();
			iret = TRUE;
			break;
			}

		case ID_EDGETOFACESELECT:
			{
			fnEdgeToFaceSelect();
			fnSetTVSubMode(3);
			MoveScriptUI();
			iret = TRUE;
			break;
			}

		case ID_FACETOVERTSELECT:
			{
			fnFaceToVertSelect();
			fnSetTVSubMode(1);
			RebuildDistCache();
			MoveScriptUI();
			iret = TRUE;
			break;
			}
		case ID_FACETOEDGESELECT:
			{
			fnFaceToEdgeSelect();
			fnSetTVSubMode(2);
			MoveScriptUI();
			iret = TRUE;
			break;
			}
		case ID_DISPLAYHIDDENEDGES:
			{
			if (fnGetDisplayHiddenEdges())
				fnSetDisplayHiddenEdges(FALSE);
			else fnSetDisplayHiddenEdges(TRUE);
			iret = TRUE;
			break;
			}
		case ID_SNAPCENTER:
			{
			fnSnapPivot(1);
			break;
			}
		case ID_SNAPLOWERLEFT:
			{
			fnSnapPivot(2);
			break;
			}
		case ID_SNAPLOWERCENTER:
			{
	 		fnSnapPivot(3);
			break;
			}
		case ID_SNAPLOWERRIGHT:
			{
			fnSnapPivot(4);
			break;
			}
		case ID_SNAPRIGHTCENTER:
			{
			fnSnapPivot(5);
			break;
			}

		case ID_SNAPUPPERLEFT:
			{
			fnSnapPivot(8);
			break;
			}
		case ID_SNAPUPPERCENTER:
			{
	 		fnSnapPivot(7);
			break;
			}
		case ID_SNAPUPPERRIGHT:
			{
			fnSnapPivot(6);
			break;
			}
		case ID_SNAPLEFTCENTER:
			{
			fnSnapPivot(9);
			break;
			}
		case ID_SKETCHREVERSE:
			{
			fnSketchReverse();
			break;
			}
		case ID_SKETCHDIALOG:
			{
			fnSketchDialog();
			iret = TRUE;
			break;
			}
		case ID_SKETCH:
			{
			fnSketchNoParams();
			iret = TRUE;
			break;
			}
		case ID_RESETPIVOTONSEL:
			{
			if (fnGetResetPivotOnSel())
				fnSetResetPivotOnSel(FALSE);
			else fnSetResetPivotOnSel(TRUE);
			iret = TRUE;
			break;
			}

		case ID_SHOWHIDDENEDGES:
			{
			if (fnGetDisplayHiddenEdges())
				fnSetDisplayHiddenEdges(FALSE);
			else fnSetDisplayHiddenEdges(TRUE);		
			iret = TRUE;
			break;
			}
		case ID_POLYGONMODE:
			{
			if (fnGetPolyMode())
				fnSetPolyMode(FALSE);
			else fnSetPolyMode(TRUE);		
			iret = TRUE;
			break;
			}
		case ID_POLYGONSELECT:
			{
			HoldSelection();
			fnPolySelect();
			break;
			}
		case ID_ALLOWSELECTIONINSIDEGIZMO:
			{
			if (fnGetAllowSelectionInsideGizmo())
				fnSetAllowSelectionInsideGizmo(FALSE);
			else fnSetAllowSelectionInsideGizmo(TRUE);		
			iret = TRUE;
			break;
			}
		case ID_SAVEASDEFAULT:
			{
			fnSetAsDefaults();
			iret = TRUE;
			break;
			}
		case ID_LOADDEFAULT:
			{
			fnLoadDefaults();
			iret = TRUE;
			break;
			}
		case ID_SHOWSHARED:
			{
			if (fnGetShowShared())
				fnSetShowShared(FALSE);
			else fnSetShowShared(TRUE);		
			iret = TRUE;
			break;
			}
		case ID_ALWAYSEDIT:
			{
			if (fnGetAlwaysEdit())
				fnSetAlwaysEdit(FALSE);
			else fnSetAlwaysEdit(TRUE);	
			iret = TRUE;
			break;
			}
		case ID_SYNCSELMODE:
			{
			if (fnGetSyncSelectionMode())
				fnSetSyncSelectionMode(FALSE);
			else fnSetSyncSelectionMode(TRUE);			
			iret = TRUE;
			break;
			}
		case ID_SYNCTVSELECTION:
			{
			fnSyncTVSelection();
			iret = TRUE;
			break;
			}
		case ID_SYNCGEOMSELECTION:
			{
			fnSyncGeomSelection();
			iret = TRUE;
			break;
			}
		case ID_SHOWOPENEDGES:
			{
			if (fnGetDisplayOpenEdges())
				fnSetDisplayOpenEdges(FALSE);
			else fnSetDisplayOpenEdges(TRUE);			
			iret = TRUE;
			break;
			}

		case ID_BRIGHTCENTERTILE:
			{
			if (fnGetBrightCenterTile())
				fnSetBrightCenterTile(FALSE);
			else fnSetBrightCenterTile(TRUE);			
			iret = TRUE;
			break;
			}
		case ID_BLENDTOBACK:
			{
			if (fnGetBlendToBack())
				fnSetBlendToBack(FALSE);
			else fnSetBlendToBack(TRUE);
			iret = TRUE;
			break;
			}

		case ID_PAINTSELECTMODE:
			{
			fnSetPaintMode(TRUE);
			iret = TRUE;
			break;
			}
		case ID_PAINTSELECTINC:
			{
			fnIncPaintSize();
			iret = TRUE;
			break;
			}
		case ID_PAINTSELECTDEC:
			{
			fnDecPaintSize();			
			iret = TRUE;
			break;
			}
		case ID_GRIDSNAP:
			{
			if (fnGetGridSnap())
				fnSetGridSnap(FALSE);
			else fnSetGridSnap(TRUE);			
			iret = TRUE;
			break;
			}

		case ID_GRIDVISIBLE:
			{
			if (fnGetGridVisible())
				fnSetGridVisible(FALSE);
			else fnSetGridVisible(TRUE);			
			iret = TRUE;
			break;
			}

		case ID_RELAX:
			{
			this->fnRelax2();
			break;
			}
			
		case ID_RELAXDIALOG:
			{
			this->fnRelax2Dialog();
			break;
			}


		}


	if ( (ip) &&(hWnd) && iret)
		{	
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();
		}

	return iret;
	}






void UnwrapMod::fnPlanarMap()
	{
//align to normals
	AlignMap();
//call fit

	flags |= CONTROL_FIT|CONTROL_CENTER|CONTROL_HOLD;
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	ApplyGizmo();
	CleanUpDeadVertices();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

void UnwrapMod::fnSave()
	{
	DragAcceptFiles(ip->GetMAXHWnd(), FALSE);
	SaveUVW(hWnd);
	DragAcceptFiles(ip->GetMAXHWnd(), TRUE);
	}
void UnwrapMod::fnLoad()
	{
	DragAcceptFiles(ip->GetMAXHWnd(), FALSE);
	LoadUVW(hWnd);
	DragAcceptFiles(ip->GetMAXHWnd(), TRUE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	InvalidateView();
	}

void UnwrapMod::fnEdit()
	{
	HWND hWnd = hParams;
	RegisterClasses();
	if (ip)
		{
		RebuildEdges();
		if (!this->hWnd) {
			HWND floaterHwnd = CreateDialogParam(
				hInstance,
				MAKEINTRESOURCE(IDD_UNWRAP_FLOATER),
	//			hWnd,
				ip->GetMAXHWnd (),
				UnwrapFloaterDlgProc,
				(LPARAM)this);
			LoadMaterials();

			IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
			assert(pContext);
			pContext->CreateWindowsMenu();
			SetMenu(floaterHwnd, pContext->GetCurWindowsMenu());
			DrawMenuBar(floaterHwnd);
			pContext->UpdateWindowsMenu();
			LaunchScriptUI();
			SetFocus(floaterHwnd);

			} 
		else 
			{
			SetForegroundWindow(this->hWnd);
			ShowWindow(this->hWnd,SW_RESTORE);
			}
		}
	}

void UnwrapMod::fnReset()
	{
	TSTR buf1 = GetString(IDS_RB_RESETUNWRAPUVWS);
	TSTR buf2 = GetString(IDS_RB_UNWRAPMOD);
	if (IDYES==MessageBox(ip->GetMAXHWnd(),buf1,buf2,MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL)) 
		{
		if (!theHold.Holding())
			theHold.Begin();
		Reset();
		theHold.Accept(GetString(IDS_RB_RESETUVWS));
		ip->RedrawViews(ip->GetTime());
		InvalidateView();

		}
	}


void UnwrapMod::fnSetMapChannel(int incChannel)
	{
	int tempChannel = incChannel;
	if (tempChannel == 1) tempChannel = 0;
	
	if (tempChannel != channel)
		{
		if (iMapID) iMapID->SetValue(incChannel,TRUE);

		theHold.Begin();
		Reset();
		channel = incChannel;
		if (channel == 1) channel = 0;
		theHold.Accept(GetString(IDS_RB_SETCHANNEL));					
		loadDefaults = FALSE;
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

		if (ip)
			{
			ip->RedrawViews(ip->GetTime());
			InvalidateView();
			}
		}

	}
int	UnwrapMod::fnGetMapChannel()
	{
	return channel;
	}

void UnwrapMod::fnSetProjectionType(int proj)
	{
	if (proj == 1)
		{
		CheckRadioButton(  hParams, IDC_RADIO1, IDC_RADIO4,IDC_RADIO1);
		SendMessage(hParams,WM_COMMAND,IDC_RADIO1,0);
		}
	else if (proj == 2)
		{
		CheckRadioButton(  hParams, IDC_RADIO1, IDC_RADIO4,IDC_RADIO2);
		SendMessage(hParams,WM_COMMAND,IDC_RADIO2,0);
		}	
	else if (proj == 3)
		{
		CheckRadioButton(  hParams, IDC_RADIO1, IDC_RADIO4,IDC_RADIO3);
		SendMessage(hParams,WM_COMMAND,IDC_RADIO3,0);
		}
	else if (proj == 4)
		{
		CheckRadioButton(  hParams, IDC_RADIO1, IDC_RADIO4,IDC_RADIO4);
		SendMessage(hParams,WM_COMMAND,IDC_RADIO4,0);
		}
	
	}
int	UnwrapMod::fnGetProjectionType()
	{
	return alignDir+1;
	}

void UnwrapMod::fnSetVC(BOOL vc)
	{
	if (vc)
		{
		CheckRadioButton(  hParams, IDC_MAP_CHAN1, IDC_MAP_CHAN2,IDC_MAP_CHAN2);
		SendMessage(hParams,WM_COMMAND,IDC_MAP_CHAN2,0);
		}
	else 
		{
		CheckRadioButton(  hParams, IDC_MAP_CHAN1, IDC_MAP_CHAN2,IDC_MAP_CHAN1);
		SendMessage(hParams,WM_COMMAND,IDC_MAP_CHAN1,0);
		}	

	}
BOOL UnwrapMod::fnGetVC()
	{
	if (channel == 1)
		return TRUE;
	else return FALSE;
	}


void UnwrapMod::fnMove()
	{
	if (iMove) 
		{
		iMove->SetCurFlyOff(0,TRUE);
		}
	else
		{
		move = 0;
		SetMode(ID_TOOL_MOVE);
		}
	}
void UnwrapMod::fnMoveH()
	{
	if (iMove) 
		{
		iMove->SetCurFlyOff(1,TRUE);
		}
	else
		{
		move = 1;
		SetMode(ID_TOOL_MOVE);
		}

	}
void UnwrapMod::fnMoveV()
	{
	if (iMove) 
		{
		iMove->SetCurFlyOff(2,TRUE);
		}
	else
		{
		move = 2;
		SetMode(ID_TOOL_MOVE);
		}

	}


void UnwrapMod::fnRotate()
	{
	if (iRot) SetMode(ID_TOOL_ROTATE);
	else
		{
		SetMode(ID_TOOL_ROTATE);
		}
	}	

void UnwrapMod::fnScale()
	{
	if (iScale) 
		iScale->SetCurFlyOff(0,TRUE);//SetMode(ID_TOOL_SCALE);
	else
		{
		scale = 0;
		SetMode(ID_TOOL_SCALE);
		}
	}
void UnwrapMod::fnScaleH()
	{
	if (iScale) 
		iScale->SetCurFlyOff(1,TRUE);//SetMode(ID_TOOL_SCALE);
	else
		{
		scale = 1;
		SetMode(ID_TOOL_SCALE);
		}
	}
void UnwrapMod::fnScaleV()
	{
	if (iScale) 
		iScale->SetCurFlyOff(2,TRUE);//SetMode(ID_TOOL_SCALE);
	else
		{
		scale = 2;
		SetMode(ID_TOOL_SCALE);
		}

	}



void UnwrapMod::fnMirrorH()
	{
	if (iMirror) 
		{
		iMirror->SetCurFlyOff(0,TRUE);
//		SendMessage(hWnd,WM_COMMAND,ID_TOOL_MIRROR,0);
		}
	else
		{
		mirror = 0;
		MirrorPoints(hWnd, mirror);
		}
	}

void UnwrapMod::fnMirrorV()
	{
	if (iMirror) 
		{
		iMirror->SetCurFlyOff(1,TRUE);

//		SendMessage(hWnd,WM_COMMAND,ID_TOOL_MIRROR,0);
		}
	else
		{
		mirror = 1;
		MirrorPoints(hWnd, mirror);

		}

	}


void UnwrapMod::fnExpandSelection()
 {

// if (iIncSelected) iIncSelected->SetCurFlyOff(0,TRUE);


	if ((fnGetTVSubMode() == TVEDGEMODE)  )
		{
		BOOL openEdgeSel = FALSE;
		int edgeCount = esel.GetSize();
		for (int i = 0; i < edgeCount; i++)
			{
			if (esel[i])
				{
				int ct = TVMaps.ePtrList[i]->faceList.Count();
				if (ct <= 1)
					{
					openEdgeSel = TRUE;
					i = edgeCount;
					}
				}
			}
		if (openEdgeSel)
			GrowSelectOpenEdge();
		GrowSelectUVEdge();
		}
	else ExpandSelection(0);

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	InvalidateView();
	UpdateWindow(hWnd);


 }


void UnwrapMod::fnContractSelection()
	{
//	if (iIncSelected) iIncSelected->SetCurFlyOff(1,TRUE);

	if ((fnGetTVSubMode() == TVEDGEMODE)  )
		ShrinkSelectOpenEdge();
	else ExpandSelection(1);

	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

	InvalidateView();
	UpdateWindow(hWnd);
	}

void UnwrapMod::fnSetFalloffType(int falloff)
	{
	if (iFalloff) 
		iFalloff->SetCurFlyOff(falloff-1,TRUE);
	else
		{
		this->falloff = falloff-1;
		RebuildDistCache();
		InvalidateView();
		}
	}
int	UnwrapMod::fnGetFalloffType()
	{
	if (iFalloff) 
		return iFalloff->GetCurFlyOff()+1;
	else
		{
		return falloff+1;
		}
	return -1;

	}
void UnwrapMod::fnSetFalloffSpace(int space)
	{
	if (iFalloffSpace) 
		iFalloffSpace->SetCurFlyOff(space-1,TRUE);
	else
		{
		falloffSpace = space -1;
		RebuildDistCache();
		InvalidateView();

		}
	}
int	UnwrapMod::fnGetFalloffSpace()
	{
	if (iFalloffSpace) 
		return iFalloffSpace->GetCurFlyOff()+1;
	else
		{
		return falloffSpace +1;
		}
	return -1;

	}

void UnwrapMod::fnSetFalloffDist(float dist)
	{
	if (iStr) iStr->SetValue(dist,TRUE);
	}
float UnwrapMod::fnGetFalloffDist()
	{
	if (iStr) return iStr->GetFVal();
	return -1.0f;
	}

void UnwrapMod::fnBreakSelected()
	{
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_BREAK,0);
	}
void UnwrapMod::fnWeld()
	{
	if ((mode == ID_UNWRAP_WELD)||(mode == ID_TOOL_WELD))
		SetMode(oldMode);
	else SetMode(ID_TOOL_WELD);
	}
void UnwrapMod::fnWeldSelected()
	{
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_WELD_SEL,0);
	}


void UnwrapMod::fnUpdatemap()
	{
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_UPDATE,0);
	}

void UnwrapMod::fnDisplaymap(BOOL update)
	{
	if (iShowMap)
		{
		iShowMap->SetCheck(update);
		SendMessage(hWnd,WM_COMMAND,ID_TOOL_SHOWMAP,0);
		}
	else
		{
		}
	
	}
BOOL UnwrapMod::fnIsMapDisplayed()
	{
	return showMap;
	}


void UnwrapMod::fnSetUVSpace(int space)
	{
	if (iUVW) 
		iUVW->SetCurFlyOff(space-1,TRUE);
	else
		{
		uvw = space -1;
		InvalidateView();
		}	
	}

int	UnwrapMod::fnGetUVSpace()
	{
	return uvw+1;
	}

void UnwrapMod::fnOptions()
	{
//	if (iProp) 
		SendMessage(hWnd,WM_COMMAND,ID_TOOL_PROP,0);
//	else
//		{
//		}	
	}


void UnwrapMod::fnLock()
	{
	if (iLockSelected)
		{
		if (iLockSelected->IsChecked())
			{
			iLockSelected->SetCheck(FALSE);
			lockSelected = FALSE;
			}
		else 
			{
			iLockSelected->SetCheck(TRUE);
			lockSelected = TRUE;
			}
		
		}
	else
		{
		lockSelected = !lockSelected;
		}
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_LOCKSELECTED,0);
	}

BOOL	UnwrapMod::fnGetLock()
	{
	return lockSelected;
	}
void	UnwrapMod::fnSetLock(BOOL lock)
	{
	lockSelected = lock;

	if (iLockSelected)
		{
		if (lockSelected)
			{
			iLockSelected->SetCheck(TRUE);
			}
		else 
			{
			iLockSelected->SetCheck(FALSE);
			}
		}
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_LOCKSELECTED,0);
	}

void UnwrapMod::fnHide()
	{
	if (iHide) 
		iHide->SetCurFlyOff(0,TRUE);
	else
		{
		HideSelected();
		InvalidateView();
		}
	}
void UnwrapMod::fnUnhide()
	{
	if (iHide) 
		iHide->SetCurFlyOff(1,TRUE);
	else
		{
		UnHideAll();
		InvalidateView();
		}
	}

void UnwrapMod::fnFreeze()
	{
	if (iFreeze) 
		iFreeze->SetCurFlyOff(0,TRUE);
	else
		{
		FreezeSelected();
		InvalidateView();
		}	

	
	}
void UnwrapMod::fnThaw()
	{
	if (iFreeze) 
		iFreeze->SetCurFlyOff(1,TRUE);
	else
		{
		UnFreezeAll();
		InvalidateView();
		}	
	}
void UnwrapMod::fnFilterSelected()
	{
	if (iFilterSelected)
		{
		if (iFilterSelected->IsChecked())
			{
			iFilterSelected->SetCheck(FALSE);
			filterSelectedFaces = FALSE;
			}
		else
			{
			iFilterSelected->SetCheck(TRUE);
			filterSelectedFaces = TRUE;
			}
		SendMessage(hWnd,WM_COMMAND,ID_TOOL_FILTER_SELECTEDFACES,0);
		}
	else
		{
		filterSelectedFaces = !filterSelectedFaces;
		SendMessage(hWnd,WM_COMMAND,ID_TOOL_FILTER_SELECTEDFACES,0);
		}
	}

BOOL	UnwrapMod::fnGetFilteredSelected()
	{
	return filterSelectedFaces;
	}
void	UnwrapMod::fnSetFilteredSelected(BOOL filter)
	{
	filterSelectedFaces = filter;
	if (iFilterSelected)
		{
		if (filterSelectedFaces)
			{
			iFilterSelected->SetCheck(TRUE);
			}
		else
			{
			iFilterSelected->SetCheck(FALSE);
			}
		
		}
	SendMessage(hWnd,WM_COMMAND,ID_TOOL_FILTER_SELECTEDFACES,0);
	}



void UnwrapMod::fnPan()
	{
//	if (iPan) 
	SetMode(ID_TOOL_PAN);
	}
void UnwrapMod::fnZoom()
	{
//	if (iWeld) 
	SetMode(ID_TOOL_ZOOM);
	}
void UnwrapMod::fnZoomRegion()
	{
//	if (iWeld) 
	SetMode(ID_TOOL_ZOOMREG);
	}
void UnwrapMod::fnFit()
	{
	if (iZoomExt) 		
		iZoomExt->SetCurFlyOff(0,TRUE);
	else
		{
		ZoomExtents();
		}
	}
void UnwrapMod::fnFitSelected()
	{
	if (iZoomExt) 
		iZoomExt->SetCurFlyOff(1,TRUE);
	else
		{
		ZoomSelected();
		}
	}

void UnwrapMod::fnSnap()
	{
	if (iSnap)
		{
		iSnap->SetCurFlyOff(1);
		if (iSnap->IsChecked())
			iSnap->SetCheck(FALSE);
		else iSnap->SetCheck(TRUE);
		}
	else
		{
		pixelSnap = !pixelSnap;
		}
	gridSnap = FALSE;

	SendMessage(hWnd,WM_COMMAND,ID_TOOL_SNAP,0);

	}

BOOL	UnwrapMod::fnGetSnap()
	{
	return pixelSnap;
	}
void	UnwrapMod::fnSetSnap(BOOL snap)
	{
	pixelSnap = snap;
	if (pixelSnap)
		gridSnap = FALSE;

	if (iSnap)
		{
		iSnap->SetCurFlyOff(1);
		if (pixelSnap)
			iSnap->SetCheck(TRUE);
		else iSnap->SetCheck(FALSE);
		}

	SendMessage(hWnd,WM_COMMAND,ID_TOOL_SNAP,0);

	}


int	UnwrapMod::fnGetCurrentMap()
	{
	return CurrentMap+1;
	}
void UnwrapMod::fnSetCurrentMap(int map)
	{
	map--;
	int ct = SendMessage( hTextures, CB_GETCOUNT, 0, 0 )-3;

	if ( (map < ct) && (CurrentMap!=map))
		{
		CurrentMap = map;
		SendMessage(hTextures, CB_SETCURSEL, map, 0 );
		SetupImage();
		}
	}	
int	UnwrapMod::fnNumberMaps()
	{
	int ct = SendMessage( hTextures, CB_GETCOUNT, 0, 0 )-3;
	return ct;
	}

Point3 *UnwrapMod::fnGetLineColor()
	{
	AColor c(lineColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
	}
void UnwrapMod::fnSetLineColor(Point3 color)
	{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	lineColor = c.toRGB();
	InvalidateView();
	}

Point3 *UnwrapMod::fnGetSelColor()
	{
	AColor c(selColor);
	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
	}

void UnwrapMod::fnSetSelColor(Point3 color)
	{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;

	selColor = c.toRGB();
	InvalidateView();
	}

void UnwrapMod::fnSetRenderWidth(int dist)
	{
	rendW = dist;
	if (rendW!=iw)
		SetupImage();
	InvalidateView();
	}
int UnwrapMod::fnGetRenderWidth()
	{
	return rendW;
	}
void UnwrapMod::fnSetRenderHeight(int dist)
	{
	rendH = dist;
	if (rendH!=ih)
		SetupImage();
	InvalidateView();
	}
int UnwrapMod::fnGetRenderHeight()
	{
	return rendH;
	}
		
void UnwrapMod::fnSetWeldThreshold(float dist)
	{
	weldThreshold = dist;
	}
float UnwrapMod::fnGetWeldThresold()
	{
	return weldThreshold;
	}

void UnwrapMod::fnSetUseBitmapRes(BOOL useBitmapRes)
	{
	BOOL change= FALSE;
	if (this->useBitmapRes != useBitmapRes)
		change = TRUE;
	this->useBitmapRes = useBitmapRes;
	if (change)
		SetupImage();
	InvalidateView();
	}
BOOL UnwrapMod::fnGetUseBitmapRes()
	{
	return useBitmapRes;
	}



BOOL UnwrapMod::fnGetConstantUpdate()
	{
	return update;
	}
void UnwrapMod::fnSetConstantUpdate(BOOL constantUpdates)
	{
	update = constantUpdates;
	}

BOOL UnwrapMod::fnGetShowSelectedVertices()
	{
	return showVerts;
	}
void UnwrapMod::fnSetShowSelectedVertices(BOOL show)
	{
	showVerts = show;
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();
	
	}

BOOL UnwrapMod::fnGetMidPixelSnape()
	{
	return midPixelSnap;
	}

void UnwrapMod::fnSetMidPixelSnape(BOOL midPixel)
	{
	midPixelSnap = midPixel;
	}


int	UnwrapMod::fnGetMatID()
	{
	return matid+2;
	}
void UnwrapMod::fnSetMatID(int mid)
	{
	mid--;
	int ct = SendMessage( hMatIDs, CB_GETCOUNT, 0, 0 );

	if ( (mid < ct) && (matid!=(mid-1)))
		{		
		SendMessage(hMatIDs, CB_SETCURSEL, mid, 0 );
		mid-=1;

		matid = mid;
		InvalidateView();
		}
	
	}
int	UnwrapMod::fnNumberMatIDs()
	{
	int ct = SendMessage( hMatIDs, CB_GETCOUNT, 0, 0 );
	return ct;
	}


void UnwrapMod::fnMoveSelectedVertices(Point3 offset)
	{
	Point2 pt;
	pt.x = offset.x;
	pt.y = offset.y;
	if (!theHold.Holding())
		theHold.Begin();
	MovePoints(pt);
	theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
	}
void UnwrapMod::fnRotateSelectedVertices(float angle, Point3 axis)
	{
	if (!theHold.Holding())
		theHold.Begin();
	centeron=TRUE;
	center.x = axis.x;
	center.y = axis.y;
//	center.z = axis.z;
	RotatePoints(tempHwnd, angle);
	theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));

	}
void UnwrapMod::fnRotateSelectedVertices(float angle)
	{
	if (!theHold.Holding())
		theHold.Begin();
	centeron=FALSE;
	RotatePoints(tempHwnd, angle);
	theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));
	}
void UnwrapMod::fnScaleSelectedVertices(float scale,int dir)
	{
	if (!theHold.Holding())
		theHold.Begin();
	centeron=FALSE;
	ScalePoints(tempHwnd, scale,dir);
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
	}


void UnwrapMod::fnScaleSelectedVertices(float scale,int dir, Point3 axis)
	{
	if (!theHold.Holding())
		theHold.Begin();
	centeron=TRUE;
	center.x = axis.x;
	center.y = axis.y;
	ScalePoints(tempHwnd, scale,dir);
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
	}


Point3* UnwrapMod::fnGetVertexPosition(TimeValue t,  int index)
	{
	index--;
	tempVert = GetPoint(t,index);
	return &tempVert;
	}

int	UnwrapMod::fnNumberVertices()
	{
	return TVMaps.v.Count();
	}


void UnwrapMod::fnMoveX(float p)
	{
	ChannelChanged(0, p);

	}
void UnwrapMod::fnMoveY(float p)
	{
	ChannelChanged(1, p);
	}
void UnwrapMod::fnMoveZ(float p)
	{
	ChannelChanged(2, p);
	}




class sMyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	};

int sMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);                 
			}
     return 0;              
	}

//new
MeshTopoData *UnwrapMod::GetModData()
	{
    sMyEnumProc dep;              
	EnumDependents(&dep);
	if (dep.Nodes.Count() > 0)
		{
		INode *node = dep.Nodes[0];
		Object* obj = node->GetObjectRef();
		SClass_ID		sc;
		IDerivedObject* dobj;
		Object *currentObject = obj;

		BOOL done = FALSE;
		if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
			{
			dobj = (IDerivedObject*)obj;
			while ((sc == GEN_DERIVOB_CLASS_ID) &&  (!done))
				{
				for (int j = 0; j < dobj->NumModifiers(); j++)
					{
					if (this == dobj->GetModifier(j) )
						{
						ModContext *mc = dobj->GetModContext(j);
						MeshTopoData *md = (MeshTopoData*)mc->localData;
						return md;
						}
					}
				dobj = (IDerivedObject*)dobj->GetObjRef();
				currentObject = (Object*) dobj;
				sc = dobj->SuperClassID();
				}
			}
		}
	return NULL;
	}


BitArray* UnwrapMod::fnGetSelectedPolygons()
	{
	ModContextList mcList;		
	INodeTab nodes;

	if (ip) 
		{
		ip->GetModContexts(mcList,nodes);

		int objects = mcList.Count();
		int ct = 0;
		if (objects != 0)
			{
			MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;
			if (md) return &md->faceSel;
			}
		}
	else
		{
		MeshTopoData *md = GetModData();
		if (md) return &md->faceSel;
		

		}
	return NULL;	

	}
void UnwrapMod::fnSelectPolygons(BitArray *sel)
	{
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();
	int ct = 0;
	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;
		if (md == NULL)
			return;

		md->faceSel.ClearAll();
		for (int i =0; i < md->faceSel.GetSize(); i++)
			{
			if (i < sel->GetSize())
				{
				if ((*sel)[i]) md->faceSel.Set(i);
				}
			}
		UpdateFaceSelection(md->faceSel);
		NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
		InvalidateView();
		
		}

	}
BOOL UnwrapMod::fnIsPolygonSelected(int index)
	{
	index--;
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return FALSE;
	ip->GetModContexts(mcList,nodes);

	int objects = mcList.Count();
	int ct = 0;
	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL)
			return FALSE;

		if (index< md->faceSel.GetSize())
			{
			return md->faceSel[index];
			}
		}
	return FALSE;	
	}


int	UnwrapMod::fnNumberPolygons()
	{
	ModContextList mcList;		
	INodeTab nodes;

	if (!ip) return 0;

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	
	if (objects != 0)
		{
		MeshTopoData *md = (MeshTopoData*)mcList[0]->localData;

		if (md == NULL)
			return 0;

		ct = md->faceSel.GetSize();
		}
	return ct;	
	}

void UnwrapMod::fnDetachEdgeVerts()
	{
	DetachEdgeVerts();
	}

void UnwrapMod::fnFlipH()
	{
	if (iMirror) 
		iMirror->SetCurFlyOff(2,TRUE);
	else
		{
		mirror = 2;
		FlipPoints(mirror-2);
		}

	}
void UnwrapMod::fnFlipV()
	{
	if (iMirror) iMirror->SetCurFlyOff(3,TRUE);
	else
		{
		mirror = 3;		
		FlipPoints(mirror-2);
		}
	}

BOOL UnwrapMod::fnGetLockAspect()
	{
	return lockAspect;
	}
void UnwrapMod::fnSetLockAspect(BOOL a)
	{
	lockAspect = a;
	InvalidateView();
	}


float UnwrapMod::fnGetMapScale()
	{
	return mapScale;
	}
void UnwrapMod::fnSetMapScale(float sc)
	{
	mapScale = sc;
	NotifyDependents(FOREVER,PART_SELECT,REFMSG_CHANGE);
	InvalidateView();

	}

void UnwrapMod::fnGetSelectionFromFace()
	{
	BitArray tempSel(vsel);
	tempSel.ClearAll();
	for (int i = 0; i < TVMaps.f.Count(); i++)
		{
		if (!(TVMaps.f[i]->flags & FLAG_DEAD))
			{
			if (TVMaps.f[i]->flags & FLAG_SELECTED)
				{
				for (int j = 0; j < TVMaps.f[i]->count; j++)
					{
					int id = TVMaps.f[i]->t[j];
					tempSel.Set(id,TRUE);
					}
				}
			}
		}
	if (fnGetTVSubMode() == TVVERTMODE)
		vsel = tempSel;
	else if (fnGetTVSubMode() == TVEDGEMODE)
		{
		BitArray holdSel(vsel);
		vsel = tempSel;
		GetEdgeSelFromVert(esel,FALSE);
		vsel = holdSel;
		}
	else if (fnGetTVSubMode() == TVFACEMODE)
		{
		BitArray holdSel(vsel);
		vsel = tempSel;
		GetFaceSelFromVert(fsel,FALSE);
		vsel = holdSel;
		}



	InvalidateView();
	}

void UnwrapMod::fnForceUpdate(BOOL update)
{
forceUpdate = update;
}

void UnwrapMod::fnZoomToGizmo(BOOL all)
{
if (ip)
	{
	if (!gizmoBounds.IsEmpty())
		ip->ZoomToBounds(all,gizmoBounds);
	}		
}


void UnwrapMod::fnSetVertexPosition(TimeValue t, int index, Point3 pos)
{
index --;
SetVertexPosition(t, index, pos);
}

void UnwrapMod::fnSetVertexPosition2(TimeValue t, int index, Point3 pos, BOOL hold, BOOL update)
{
index --;
SetVertexPosition(t, index, pos, hold, update);

}


void UnwrapMod::fnMarkAsDead(int index)
{
index--;
if ((index>=0) && (index < TVMaps.v.Count()))
	TVMaps.v[index].flags |= FLAG_DEAD;
}



int UnwrapMod::fnNumberPointsInFace(int index)
{
index--;
if ((index>=0) && (index < TVMaps.f.Count()))
	{
	return TVMaps.f[index]->count;
	}
return 0;

}
int UnwrapMod::fnGetVertexIndexFromFace(int index,int vertexIndex)
{
index--;
vertexIndex--;
if ((index>=0) && (index < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[index]->count;
	if ((vertexIndex >=0) && (vertexIndex <pcount))
		{
		return TVMaps.f[index]->t[vertexIndex]+1;
		}
	}

return 0;
}


int UnwrapMod::fnGetHandleIndexFromFace(int index,int vertexIndex)
{
index--;
vertexIndex--;
if ((index>=0) && (index < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[index]->count*2;
	if ((vertexIndex >=0) && (vertexIndex <pcount))
		{
		if ( (TVMaps.f[index]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[index]->vecs) )
			return TVMaps.f[index]->vecs->handles[vertexIndex]+1;
		}
	}

return 0;
}

int UnwrapMod::fnGetInteriorIndexFromFace(int index,int vertexIndex)
{
index--;
vertexIndex--;
if ((index>=0) && (index < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[index]->count;
	if ((vertexIndex >=0) && (vertexIndex <pcount))
		{
		if ( (TVMaps.f[index]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[index]->vecs) )
			{
			if (TVMaps.f[index]->flags & FLAG_INTERIOR)
				return TVMaps.f[index]->vecs->interiors[vertexIndex]+1;
			}	
		}
	}

return 0;
}



int UnwrapMod::fnGetVertexGIndexFromFace(int index,int vertexIndex)
{
index--;
vertexIndex--;
if ((index>=0) && (index < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[index]->count;
	if ((vertexIndex >=0) && (vertexIndex <pcount))
		{
		return TVMaps.f[index]->v[vertexIndex]+1;
		}
	}

return 0;
}


int UnwrapMod::fnGetHandleGIndexFromFace(int index,int vertexIndex)
{
index--;
vertexIndex--;
if ((index>=0) && (index < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[index]->count*2;
	if ((vertexIndex >=0) && (vertexIndex <pcount))
		{
		if ( (TVMaps.f[index]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[index]->vecs) )
			return TVMaps.f[index]->vecs->vhandles[vertexIndex]+1;
		}
	}

return 0;
}

int UnwrapMod::fnGetInteriorGIndexFromFace(int index,int vertexIndex)
{
index--;
vertexIndex--;
if ((index>=0) && (index < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[index]->count;
	if ((vertexIndex >=0) && (vertexIndex <pcount))
		{
		if ( (TVMaps.f[index]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[index]->vecs) )
			{
			if (TVMaps.f[index]->flags & FLAG_INTERIOR)
				return TVMaps.f[index]->vecs->vinteriors[vertexIndex]+1;
			}	
		}
	}

return 0;
}


void UnwrapMod::fnAddPoint(Point3 pos, int fIndex,int vIndex, BOOL sel)
{
fIndex--;
vIndex--;
if ((fIndex>=0) && (fIndex < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[fIndex]->count;
	if ((vIndex >=0) && (vIndex <pcount))
		{
		AddPoint(pos,fIndex,vIndex,sel);
		}
	}

}
void UnwrapMod::fnAddHandle(Point3 pos, int fIndex,int vIndex, BOOL sel)
{
fIndex--;
vIndex--;
if ((fIndex>=0) && (fIndex < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[fIndex]->count*2;
	if ((vIndex >=0) && (vIndex <pcount))
		{
		if ( (TVMaps.f[fIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[fIndex]->vecs) )
			AddHandle(pos,fIndex,vIndex,sel);
		}
	}

}
void UnwrapMod::fnAddInterior(Point3 pos, int fIndex,int vIndex, BOOL sel)
{
fIndex--;
vIndex--;
if ((fIndex>=0) && (fIndex < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[fIndex]->count;
	if ((vIndex >=0) && (vIndex <pcount))
		{
		if ( (TVMaps.f[fIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[fIndex]->vecs) )
			{
			if (TVMaps.f[fIndex]->flags & FLAG_INTERIOR)
				AddInterior(pos,fIndex,vIndex,sel);
			}
		}
	}
}



void UnwrapMod::fnSetFaceVertexIndex(int fIndex,int ithV, int vIndex)
{
fIndex--;
ithV--;
vIndex--;
if ((fIndex>=0) && (fIndex < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[fIndex]->count;
	if ((ithV >=0) && (ithV <pcount) && (vIndex >=0) && (vIndex<TVMaps.v.Count()))
		{
		TVMaps.f[fIndex]->t[ithV] = vIndex;
		}
	}

}
void UnwrapMod::fnSetFaceHandleIndex(int fIndex,int ithV, int vIndex)
{
fIndex--;
ithV--;
vIndex--;
if ((fIndex>=0) && (fIndex < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[fIndex]->count*2;
	if ((ithV >=0) && (ithV <pcount) && (vIndex >=0) && (vIndex<TVMaps.v.Count()))
		{
		if ( (TVMaps.f[fIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[fIndex]->vecs) )
			TVMaps.f[fIndex]->vecs->handles[ithV] = vIndex;
		}
	}

}
void UnwrapMod::fnSetFaceInteriorIndex(int fIndex,int ithV, int vIndex)
{
fIndex--;
ithV--;
vIndex--;
if ((fIndex>=0) && (fIndex < TVMaps.f.Count()))
	{
	int pcount = TVMaps.f[fIndex]->count;
	if ((ithV >=0) && (ithV <pcount) && (vIndex >=0) && (vIndex<TVMaps.v.Count()))
		{
		if ( (TVMaps.f[fIndex]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[fIndex]->vecs) )
			{
			if (TVMaps.f[fIndex]->flags & FLAG_INTERIOR)
				TVMaps.f[fIndex]->vecs->interiors[ithV] = vIndex;
			}
		}
	}

}

void UnwrapMod::fnUpdateViews()
{
NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
InvalidateView();
}

void UnwrapMod::fnGetFaceSelFromStack()
{
getFaceSelectionFromStack = TRUE;
NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
if (ip) ip->RedrawViews(ip->GetTime());
InvalidateView();
}

void UnwrapMod::fnSetDebugLevel(int level)
	{
	gDebugLevel = level;
	}


BOOL	UnwrapMod::fnGetTile()
	{
	return bTile;
	}
void	UnwrapMod::fnSetTile(BOOL tile)
	{
	bTile = tile;
	tileValid = FALSE;
	InvalidateView();
	}

int		UnwrapMod::fnGetTileLimit()
	{
	return iTileLimit;
	
	}
void	UnwrapMod::fnSetTileLimit(int limit)
	{
	iTileLimit = limit;
	if (iTileLimit < 0 ) iTileLimit = 0;
	if (iTileLimit > 50 ) iTileLimit = 50;
	tileValid = FALSE;
	InvalidateView();
	}

float	UnwrapMod::fnGetTileContrast()
	{
	return fContrast;
	}
void	UnwrapMod::fnSetTileContrast(float fcontrast)
	{
	this->fContrast = fcontrast;
	tileValid = FALSE;
	InvalidateView();
	}



BOOL	UnwrapMod::fnGetShowMap()
	{
	return showMap;
	}	

void	UnwrapMod::fnSetShowMap(BOOL smap)
	{
	showMap = smap;
	if (iShowMap) 
		iShowMap->SetCheck(smap);
	else InvalidateView();
	}

void	UnwrapMod::fnShowMap()
	{
	if (iShowMap)
		{
		if (iShowMap->IsChecked())
			iShowMap->SetCheck(FALSE);
		else iShowMap->SetCheck(TRUE);
		SendMessage(hWnd,WM_COMMAND,ID_TOOL_SHOWMAP,0);
		}
	else
		{
		showMap = !showMap;
		InvalidateView();

		}
	}

#pragma warning (disable  : 4530)

void	UnwrapMod::InitScriptUI()
	{
	if (executedStartUIScript==FALSE)
		{
		TSTR scriptUI;

		scriptUI.printf("mcrfile = openFile   \"UI\\MacroScripts\\Macro_UnwrapUI.mcr\" ; execute mcrfile");


		init_thread_locals();
		push_alloc_frame();
		one_typed_value_local(StringStream* util_script);

		vl.util_script = new StringStream (scriptUI);

		save_current_frames();
		try 
			{
	   		vl.util_script->execute_vf(NULL, 0);
			}
		catch (MAXScriptException& e)
			{
			restore_current_frames();
			MAXScript_signals = 0;
			error_message_box(e, _T("Unwrap UI Script Macro_UnwrapUI.mcr not found"));
			}
		catch (...)
			{
			restore_current_frames();
			error_message_box(UnknownSystemException (), _T("Unwrap UI Script Macro_UnwrapUI.mcr not found"));
			}
		vl.util_script->close();
		pop_value_locals();
		pop_alloc_frame();
		executedStartUIScript=TRUE;
		}
	
	}

void	UnwrapMod::LaunchScriptUI()
{
	TSTR scriptUI;

	scriptUI.printf("macros.run \"UVW Unwrap\" \"OpenUnwrapUI\" ");


	init_thread_locals();
	push_alloc_frame();
	one_typed_value_local(StringStream* util_script);

	vl.util_script = new StringStream (scriptUI);

	save_current_frames();
	try 
	{
	   	vl.util_script->execute_vf(NULL, 0);
	}
	catch (MAXScriptException& e)
	{
		restore_current_frames();
		MAXScript_signals = 0;
		error_message_box(e, _T("Unwrap UI Script OpenUnwrapUI Macro not found"));
	}
	catch (...)
	{
		restore_current_frames();
		error_message_box(UnknownSystemException (), _T("Unwrap UI Script  OpenUnwrapUI Macro not found"));
	}
	vl.util_script->close();
	pop_value_locals();
	pop_alloc_frame();
}

void	UnwrapMod::EndScriptUI()
{

	TSTR scriptUI;

	scriptUI.printf("macros.run \"UVW Unwrap\" \"CloseUnwrapUI\" ");

	init_thread_locals();
	push_alloc_frame();
	one_typed_value_local(StringStream* util_script);

	vl.util_script = new StringStream (scriptUI);

	save_current_frames();
	try 
	{
	   	vl.util_script->execute_vf(NULL, 0);
	}
	catch (MAXScriptException& e)
	{
		restore_current_frames();
		MAXScript_signals = 0;
		error_message_box(e, _T("Unwrap UI Script CloseUnwrapUI Macro not found"));
	}
	catch (...)
	{
		restore_current_frames();
		error_message_box(UnknownSystemException (), _T("Unwrap UI Script  CloseUnwrapUI Macro not found"));
	}
	vl.util_script->close();
	pop_value_locals();
	pop_alloc_frame();


}

void	UnwrapMod::MoveScriptUI()
{

	TSTR scriptUI;

	scriptUI.printf("macros.run \"UVW Unwrap\" \"MoveUnwrapUI\" ");

	init_thread_locals();
	push_alloc_frame();
	one_typed_value_local(StringStream* util_script);

	vl.util_script = new StringStream (scriptUI);

	save_current_frames();
	try 
	{
	   	vl.util_script->execute_vf(NULL, 0);
	}
	catch (MAXScriptException& e)
	{
		restore_current_frames();
		MAXScript_signals = 0;
		error_message_box(e, _T("Unwrap UI Script MoveUnwrapUI Macro not found"));
	}
	catch (...)
	{
		restore_current_frames();
		error_message_box(UnknownSystemException (), _T("Unwrap UI Script  MoveUnwrapUI Macro not found"));
	}
	vl.util_script->close();
	pop_value_locals();
	pop_alloc_frame();


}


#pragma warning (default  : 4530)

int		UnwrapMod::fnGetWindowX()
	{
	WINDOWPLACEMENT floaterPos;
	GetWindowPlacement(hWnd,&floaterPos);
	
	
	
	WINDOWPLACEMENT maxPos;
	Interface *ip = GetCOREInterface();
	HWND maxHwnd = ip->GetMAXHWnd();
	GetWindowPlacement(maxHwnd,&maxPos);
	
	RECT rect;
	GetWindowRect(  hWnd ,&rect);

	if (floaterPos.showCmd & SW_MINIMIZE)
		return 0;
	else return rect.left;//return floaterPos.rcNormalPosition.left;
	}
int		UnwrapMod::fnGetWindowY()
	{
	WINDOWPLACEMENT floaterPos;
	GetWindowPlacement(hWnd,&floaterPos);
	
	RECT rect;
	GetWindowRect(  hWnd ,&rect);	
	
	if (floaterPos.showCmd & SW_MINIMIZE)
		return 0;
	else return rect.top;//floaterPos.rcNormalPosition.top;
	}
int		UnwrapMod::fnGetWindowW()
	{
	WINDOWPLACEMENT floaterPos;
	GetWindowPlacement(hWnd,&floaterPos);
	if (floaterPos.showCmd & SW_MINIMIZE)
		return 0;
	else return (floaterPos.rcNormalPosition.right-floaterPos.rcNormalPosition.left);

	}
int		UnwrapMod::fnGetWindowH()
	{
	WINDOWPLACEMENT floaterPos;
	GetWindowPlacement(hWnd,&floaterPos);
	if (floaterPos.showCmd & SW_MINIMIZE)
		return 0;
	else return (floaterPos.rcNormalPosition.bottom-floaterPos.rcNormalPosition.top);
	}


BOOL	UnwrapMod::fnGetBackFaceCull()
	{
	return ignoreBackFaceCull;
	}	

void	UnwrapMod::fnSetBackFaceCull(BOOL backFaceCull)
	{
	ignoreBackFaceCull = backFaceCull;
//update UI
	CheckDlgButton(hSelRollup,IDC_IGNOREBACKFACING_CHECK,ignoreBackFaceCull);
	}

BOOL	UnwrapMod::fnGetOldSelMethod()
	{
	return oldSelMethod;
	}
void	UnwrapMod::fnSetOldSelMethod(BOOL oldSelMethod)
	{
	this->oldSelMethod = oldSelMethod;
	}

BOOL	UnwrapMod::fnGetAlwaysEdit()
	{
	return alwaysEdit;
	}
void	UnwrapMod::fnSetAlwaysEdit(BOOL always)
	{
	this->alwaysEdit = always;
	}


int		UnwrapMod::fnGetTVSubMode()
	{
	return TVSubObjectMode+1;
	}
void	UnwrapMod::fnSetTVSubMode(int mode)
	{
	TVSubObjectMode = mode-1;
	if (fnGetSyncSelectionMode())
		{
		theHold.Suspend();
 		fnSyncGeomSelection();
		theHold.Resume();
		}

	if ( (ip) &&(hWnd) )
		{	
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();
		}

	InvalidateView();
	}

int		UnwrapMod::fnGetFillMode()
	{
	return fillMode;
	}

void	UnwrapMod::fnSetFillMode(int mode)
	{
	fillMode = mode;
	InvalidateView();
	}

void UnwrapMod::fnMoveSelected(Point3 offset)
	{
	Point2 pt;
	pt.x = offset.x;
	pt.y = offset.y;
	TransferSelectionStart();
	if (!theHold.Holding())
		theHold.Begin();
	MovePoints(pt);
	theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
	TransferSelectionEnd(FALSE,FALSE);
	}
void UnwrapMod::fnRotateSelected(float angle, Point3 axis)
	{
	TransferSelectionStart();
	if (!theHold.Holding())
		theHold.Begin();
	centeron=TRUE;
	center.x = axis.x;
	center.y = axis.y;
//	center.z = axis.z;
	RotatePoints(tempHwnd, angle);
	theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));
	TransferSelectionEnd(FALSE,FALSE);

	}
void UnwrapMod::fnRotateSelected(float angle)
	{
	TransferSelectionStart();
	if (!theHold.Holding())
		theHold.Begin();
	centeron=FALSE;
	RotatePoints(tempHwnd, angle);
	theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));
	TransferSelectionEnd(FALSE,FALSE);
	}
void UnwrapMod::fnScaleSelected(float scale,int dir)
	{
	TransferSelectionStart();
	if (!theHold.Holding())
		theHold.Begin();
	centeron=FALSE;
	ScalePoints(tempHwnd, scale,dir);
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
	TransferSelectionEnd(FALSE,FALSE);
	}

void UnwrapMod::fnScaleSelected(float scale,int dir, Point3 axis)
	{
	TransferSelectionStart();
	if (!theHold.Holding())
		theHold.Begin();
	centeron=TRUE;
	center.x = axis.x;
	center.y = axis.y;
	ScalePoints(tempHwnd, scale,dir);
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
	TransferSelectionEnd(FALSE,FALSE);
	}

void UnwrapMod::fnScaleSelectedXY(float scaleX,float scaleY, Point3 axis)
	{
	TransferSelectionStart();
	if (!theHold.Holding())
		theHold.Begin();
	center.x = axis.x;
	center.y = axis.y;
	ScalePointsXY(tempHwnd, scaleX,scaleY);
	theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
	TransferSelectionEnd(FALSE,FALSE);
	}


BOOL	UnwrapMod::fnGetDisplayOpenEdges()
	{
	return displayOpenEdges;
	}
void	UnwrapMod::fnSetDisplayOpenEdges(BOOL openEdgeDisplay)
	{
	displayOpenEdges = openEdgeDisplay;
	InvalidateView();
	}
		


Point3 *UnwrapMod::fnGetOpenEdgeColor()
	{
	AColor c(openEdgeColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
	}
void UnwrapMod::fnSetOpenEdgeColor(Point3 color)
	{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	openEdgeColor = c.toRGB();
	InvalidateView();
	}

BOOL	 UnwrapMod::fnGetDisplayHiddenEdges()
	{
	return displayHiddenEdges;
	}
void	 UnwrapMod::fnSetDisplayHiddenEdges(BOOL hiddenEdgeDisplay)
	{
	displayHiddenEdges = hiddenEdgeDisplay;
	InvalidateView();
	}



Point3 *UnwrapMod::fnGetHandleColor()
	{
	AColor c(handleColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
	}
void UnwrapMod::fnSetHandleColor(Point3 color)
	{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	handleColor = c.toRGB();
	InvalidateView();
	}


BOOL	UnwrapMod::fnGetFreeFormMode()
	{
	if (mode ==ID_FREEFORMMODE)
		return TRUE; 
	else return FALSE; 

	}
void	UnwrapMod::fnSetFreeFormMode(BOOL freeFormMode)
	{
	if (freeFormMode)
		{
		SetMode(ID_FREEFORMMODE);
		}
	}


Point3 *UnwrapMod::fnGetFreeFormColor()
	{
	AColor c(freeFormColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
	}
void UnwrapMod::fnSetFreeFormColor(Point3 color)
	{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	freeFormColor = c.toRGB();
	InvalidateView();
	}

void	UnwrapMod::fnSnapPivot(int pos)
	{
//snap to center
	if (pos == 1)
		freeFormPivotOffset = Point3(0.0f,0.0f,0.0f);
	else if (pos == 2)
		freeFormPivotOffset = selCenter - freeFormCorners[3];
	else if (pos == 3)
		freeFormPivotOffset = selCenter - freeFormEdges[2];
	else if (pos == 4)
		freeFormPivotOffset = selCenter - freeFormCorners[2];
	else if (pos == 5)
		freeFormPivotOffset = selCenter - freeFormEdges[3];
	else if (pos == 6)
		freeFormPivotOffset = selCenter - freeFormCorners[0];
	else if (pos == 7)
		freeFormPivotOffset = selCenter - freeFormEdges[0];
	else if (pos == 8)
		freeFormPivotOffset = selCenter - freeFormCorners[1];
	else if (pos == 9)
		freeFormPivotOffset = selCenter - freeFormEdges[1];

	InvalidateView();

	}

Point3*	UnwrapMod::fnGetPivotOffset()
	{
	return &freeFormPivotOffset;
	}
void	UnwrapMod::fnSetPivotOffset(Point3 offset)
	{
	freeFormPivotOffset = offset;
	InvalidateView();
	}
Point3*	UnwrapMod::fnGetSelCenter()
	{
	return &selCenter;
	}

BOOL	UnwrapMod::fnGetResetPivotOnSel()
	{
	return resetPivotOnSel;
	}
void	UnwrapMod::fnSetResetPivotOnSel(BOOL reset)
	{
	resetPivotOnSel = reset;
	}

BOOL	UnwrapMod::fnGetAllowSelectionInsideGizmo()
	{
	return allowSelectionInsideGizmo;
	}
void	UnwrapMod::fnSetAllowSelectionInsideGizmo(BOOL select)
	{
	allowSelectionInsideGizmo = select;
	}


void	UnwrapMod::fnSetSharedColor(Point3 color)
	{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	sharedColor = c.toRGB();
	InvalidateView();
	}

Point3*	UnwrapMod::fnGetSharedColor()
	{
	AColor c(sharedColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
	}

BOOL	UnwrapMod::fnGetShowShared()
	{
	return showShared;
	}

void	UnwrapMod::fnSetShowShared(BOOL share)
	{
	showShared = share;
	InvalidateView();
	}

void	UnwrapMod::fnShowIcon(int id,BOOL show)
	{
	if ((id > 0) && (id < 30))
		showIconList.Set(id,show);
	}


Point3* UnwrapMod::fnGetBackgroundColor()
	{
	AColor c(backgroundColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
	}

void UnwrapMod::fnSetBackgroundColor(Point3 color)
	{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	backgroundColor = c.toRGB();
	if (iBuf) iBuf->SetBkColor(backgroundColor);
	if (iTileBuf) iTileBuf->SetBkColor(backgroundColor);
	ColorMan()->SetColor(BACKGROUNDCOLORID,  backgroundColor);
	InvalidateView();
	}

void	UnwrapMod::fnUpdateMenuBar()
	{
	if ( (ip) &&(hWnd) )
		{
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();

		}
	}

BOOL	UnwrapMod::fnGetBrightCenterTile()
	{
	return brightCenterTile;
	}

void	UnwrapMod::fnSetBrightCenterTile(BOOL bright)
	{
	brightCenterTile = bright;
	tileValid = FALSE;
	InvalidateView();
	}

BOOL	UnwrapMod::fnGetBlendToBack()
	{
	return blendTileToBackGround;
	}

void	UnwrapMod::fnSetBlendToBack(BOOL blend)
	{
	blendTileToBackGround = blend;
	tileValid = FALSE;
	InvalidateView();
	}

int		UnwrapMod::fnGetTickSize()
	{
	return tickSize;
	}
void	UnwrapMod::fnSetTickSize(int size)
	{
	tickSize = size;
	if (tickSize <1 ) tickSize = 1;
	InvalidateView();
	}


//new


float	UnwrapMod::fnGetGridSize()
	{
	return gridSize;
	}
void	UnwrapMod::fnSetGridSize(float size)
	{
	gridSize = size;
	InvalidateView();
	}

BOOL	UnwrapMod::fnGetGridSnap()
	{
	return gridSnap;
	}
void	UnwrapMod::fnSetGridSnap(BOOL snap)
	{
	gridSnap = snap;

	if (gridSnap)
		pixelSnap = FALSE;
	
	if (iSnap)
		{
		iSnap->SetCurFlyOff(0);
		if (gridSnap)		
			iSnap->SetCheck(TRUE);
		else iSnap->SetCheck(FALSE);
		}
	}
BOOL	UnwrapMod::fnGetGridVisible()
	{
	return gridVisible;

	}
void	UnwrapMod::fnSetGridVisible(BOOL visible)
	{
	gridVisible = visible;
	InvalidateView();
	}

void	UnwrapMod::fnSetGridColor(Point3 color)
	{
	AColor c;
	c.r = color.x;
	c.g = color.y;
	c.b = color.z;
	gridColor = c.toRGB();
	if (gridVisible) InvalidateView();
	}

Point3*	UnwrapMod::fnGetGridColor()
	{
	AColor c(gridColor);

	lColor.x = c.r;
	lColor.y = c.g;
	lColor.z = c.b;
	return &lColor;
	}

float	UnwrapMod::fnGetGridStr()
	{
	
	return gridStr * 2.0f;
	}
void	UnwrapMod::fnSetGridStr(float str)
	{

	str = str *0.5f;

	if (str < 0.0f) str = 0.0f;
	if (str > 0.5f) str = 0.5f;

	gridStr = str;
	}


BOOL	UnwrapMod::fnGetAutoMap()
	{
	return autoMap;

	}
void	UnwrapMod::fnSetAutoMap(BOOL autoMap)
	{
	this->autoMap = autoMap;
	}




float	UnwrapMod::fnGetFlattenAngle()
{
	return flattenAngleThreshold;
}
void	UnwrapMod::fnSetFlattenAngle(float angle)
{
	if (angle < 0.0f) angle = 0.0f;
	if (angle > 180.0f) angle = 180.0f;

	flattenAngleThreshold = angle;
	loadDefaults = FALSE;
}

float	UnwrapMod::fnGetFlattenSpacing()
{
	return flattenSpacing;
}
void	UnwrapMod::fnSetFlattenSpacing(float spacing)
{
	if (spacing < 0.0f) spacing = 0.0f;
	if (spacing > 1.0f) spacing = 1.0f;

	flattenSpacing = spacing;
	loadDefaults = FALSE;
}

BOOL	UnwrapMod::fnGetFlattenNormalize()
{
	return flattenNormalize;
}
void	UnwrapMod::fnSetFlattenNormalize(BOOL normalize)
{
	flattenNormalize = normalize;
	loadDefaults = FALSE;
}

BOOL	UnwrapMod::fnGetFlattenRotate()
{
	return flattenRotate;
}
void	UnwrapMod::fnSetFlattenRotate(BOOL rotate)
{
	flattenRotate = rotate;
	loadDefaults = FALSE;
}

BOOL	UnwrapMod::fnGetFlattenFillHoles()
{
	return flattenCollapse;
}
void	UnwrapMod::fnSetFlattenFillHoles(BOOL fillHoles)
{
	flattenCollapse = fillHoles;
	loadDefaults = FALSE;
}


BOOL	UnwrapMod::fnGetPreventFlattening()
{
	return preventFlattening;
}
void	UnwrapMod::fnSetPreventFlattening(BOOL preventFlattening)
{
	this->preventFlattening = preventFlattening;

	if ( (ip) &&(hWnd) )
		{	
		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();
		//update UI

		}
	CheckDlgButton(hSelRollup,IDC_DONOTREFLATTEN_CHECK,preventFlattening);
	loadDefaults = FALSE;

}


BOOL	UnwrapMod::fnGetEnableSoftSelection()
{
	return enableSoftSelection;
}

void	UnwrapMod::fnSetEnableSoftSelection(BOOL enable)
{
	enableSoftSelection = enable;
	RebuildDistCache();
	InvalidateView();

}


BOOL	UnwrapMod::fnGetApplyToWholeObject()
{
	return applyToWholeObject;
}

void	UnwrapMod::fnSetApplyToWholeObject(BOOL whole)
{
	applyToWholeObject = whole;

}






//5.1.05
BOOL	UnwrapMod::fnGetAutoBackground()
{
	return this->autoBackground;
}
void	UnwrapMod::fnSetAutoBackground(BOOL autoBackground)
{
	this->autoBackground = autoBackground;
}

