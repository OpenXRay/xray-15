/**********************************************************************
 *<
	FILE: PolyEdit.cpp

	DESCRIPTION:   Editable Polygon Mesh Object

	CREATED BY: Steve Anderson

	HISTORY: created November 1999

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "PolyEdit.h"
#include "MeshDLib.h"
#include "macrorec.h"
#include "setkeymode.h"

//--- Class vars -------------------------------

Interface *			EditPolyObject::ip              = NULL;
EditPolyObject *	EditPolyObject::editObj			= NULL;
IParamMap2 *		EditPolyObject::pSelect			= NULL;
IParamMap2 *		EditPolyObject::pSoftsel		= NULL;
IParamMap2 *		EditPolyObject::pGeom			= NULL;
IParamMap2 *		EditPolyObject::pSubdivision	= NULL;	
IParamMap2 *		EditPolyObject::pSurface		= NULL;
IParamMap2 *		EditPolyObject::pDisplacement = NULL;
IParamMap2 *		EditPolyObject::pOperationSettings = NULL;
IParamMap2 *		EditPolyObject::pSubobjControls = NULL;
bool				EditPolyObject::rsSelect		= false;
bool				EditPolyObject::rsSoftsel		= true;
bool				EditPolyObject::rsGeom			= false;
bool				EditPolyObject::rsSubdivision	= false;
bool				EditPolyObject::rsSurface		= false;
bool				EditPolyObject::rsDisplacement = true;
bool				EditPolyObject::rsSubobjControls = false;
bool				EditPolyObject::inExtrude    = false;
bool				EditPolyObject::inBevel = false;
bool				EditPolyObject::inChamfer = false;
bool				EditPolyObject::inInset = false;

MoveModBoxCMode*    EditPolyObject::moveMode        = NULL;
RotateModBoxCMode*  EditPolyObject::rotMode			= NULL;
UScaleModBoxCMode*  EditPolyObject::uscaleMode      = NULL;
NUScaleModBoxCMode* EditPolyObject::nuscaleMode     = NULL;
SquashModBoxCMode*  EditPolyObject::squashMode      = NULL;
SelectModBoxCMode*  EditPolyObject::selectMode      = NULL;
CreateVertCMode*	EditPolyObject::createVertMode  = NULL;
CreateEdgeCMode*	EditPolyObject::createEdgeMode	= NULL;
CreateFaceCMode*	EditPolyObject::createFaceMode	= NULL;
AttachPickMode*		EditPolyObject::attachPickMode	= NULL;
ShapePickMode *	EditPolyObject::shapePickMode = NULL;
DivideEdgeCMode*	EditPolyObject::divideEdgeMode	= NULL;
DivideFaceCMode *	EditPolyObject::divideFaceMode	= NULL;
ExtrudeCMode *		EditPolyObject::extrudeMode		= NULL;
ExtrudeVECMode *	EditPolyObject::extrudeVEMode = NULL;
ChamferCMode *		EditPolyObject::chamferMode		= NULL;
BevelCMode*			EditPolyObject::bevelMode		= NULL;
InsetCMode*			EditPolyObject::insetMode = NULL;
OutlineCMode*		EditPolyObject::outlineMode = NULL;
CutCMode *			EditPolyObject::cutMode		= NULL;
QuickSliceCMode *	EditPolyObject::quickSliceMode = NULL;
WeldCMode *			EditPolyObject::weldMode		= NULL;
LiftFromEdgeCMode * EditPolyObject::liftFromEdgeMode = NULL;
PickLiftEdgeCMode * EditPolyObject::pickLiftEdgeMode = NULL;
EditTriCMode *		EditPolyObject::editTriMode		= NULL;
int EditPolyObject::attachMat = 0;
int EditPolyObject::condenseMat = true;
Quat				EditPolyObject::sliceRot(0.0f,0.0f,0.0f,1.0f);
Point3				EditPolyObject::sliceCenter(0.0f,0.0f,0.0f);
float				EditPolyObject::sliceSize = 100.0f;
bool				EditPolyObject::sliceMode		= false;
EPolyBackspaceUser EditPolyObject::backspacer;
EPolyActionCB *EditPolyObject::mpEPolyActions = NULL;

TempMoveRestore * EditPolyObject::tempMove = NULL;

//--- EditPolyObject methods ----------------------------------

EditPolyObject::EditPolyObject() {
	tempData = NULL;
	pblock = NULL;
	masterCont = NULL;
	selLevel = EP_SL_OBJECT;
	arValid.SetEmpty();
	ePolyFlags = 0;
	sliceInitialized = false;
	GetEditablePolyDesc()->MakeAutoParamBlocks(this);

	mPreviewOn = false;
	mPreviewValid = false;
	mPreviewDrag = false;
	mPreviewSuspend = false;
	mPreviewOperation = epop_null;
	mpPreviewTemp = NULL;

	mLastOperation = epop_null;

	mHitLevelOverride = 0x0;
	mDispLevelOverride = 0x0;
	mForceIgnoreBackfacing = false;
	mSuspendConstraints = false;

	theHold.Suspend();
	MakeRefByID (FOREVER, EPOLY_MASTER_CONTROL_REF, NewDefaultMasterPointController());
	theHold.Resume();
}

EditPolyObject::~EditPolyObject() {
	if (tempData) delete tempData;

	theHold.Suspend ();
	DeleteAllRefsFromMe ();
	theHold.Resume ();
}

void EditPolyObject::ResetClassParams (BOOL fileReset) {
	EditPolyObject::rsSelect		= FALSE;
	EditPolyObject::rsSoftsel		= TRUE;
	EditPolyObject::rsGeom			= FALSE;
	EditPolyObject::rsSubdivision		= TRUE;
	EditPolyObject::rsSurface		= FALSE;
	EditPolyObject::inExtrude    = FALSE;
	EditPolyObject::inBevel = FALSE;
	EditPolyObject::inChamfer = FALSE;
	EditPolyObject::inInset = false;
	EditPolyObject::attachMat = 0;
	EditPolyObject::condenseMat = true;
	EditPolyObject::sliceRot = Quat(0.0f,0.0f,0.0f,1.0f);
	EditPolyObject::sliceCenter = Point3(0.0f,0.0f,0.0f);
	EditPolyObject::sliceSize = 100.0f;
	EditPolyObject::sliceMode		= false;
}

RefTargetHandle EditPolyObject::Clone(RemapDir& remap) {
	EditPolyObject *npol = new EditPolyObject;
	npol->mm = mm;
	npol->ReplaceReference(EPOLY_PBLOCK,pblock->Clone(remap));
	for (int i=0; i<cont.Count(); i++) {
		if (cont[i] == NULL) continue;
		npol->MakeRefByID (FOREVER, EPOLY_VERT_BASE_REF + i, cont[i]->Clone (remap));
	}
	npol->SetDisplacementParams ();
	for (i=0; i<3; i++) npol->selSet[i] = selSet[i];
	BaseClone(this, npol, remap);
	return npol;
}

BOOL EditPolyObject::IsSubClassOf(Class_ID classID) {
	return classID==ClassID() || classID==polyObjectClassID;	
}

Object *EditPolyObject::ConvertToType (TimeValue t, Class_ID obtype) {
	UpdateEverything (t);

	int useSubdiv=false;
	if (pblock) pblock->GetValue (ep_surf_subdivide, t, useSubdiv, FOREVER);
	if (!useSubdiv && !EpPreviewOn()) return PolyObject::ConvertToType (t, obtype);

	PolyObject *pobj = new PolyObject;
	if (useSubdiv) pobj->mm = subdivResult;
	else pobj->mm = mPreviewMesh;

	pobj->SetChannelValidity (GEOM_CHAN_NUM, geomValid);
	pobj->SetChannelValidity (TOPO_CHAN_NUM, topoValid);
	pobj->SetChannelValidity (TEXMAP_CHAN_NUM, texmapValid);
	pobj->SetChannelValidity (VERT_COLOR_CHAN_NUM, vcolorValid);

	pobj->SetDisplacementDisable (GetDisplacementDisable ());
	pobj->SetDisplacementSplit (GetDisplacementSplit());
	pobj->SetDisplacementParameters (GetDisplacementParameters());
	pobj->SetDisplacement (GetDisplacement ());
	pobj->mm.dispFlags = mm.dispFlags;

	Object *ret = pobj->ConvertToType (t, obtype);
	if (ret != pobj) delete pobj;
	return ret;
}

int EditPolyObject::RenderBegin(TimeValue t, ULONG flags) {
	SetFlag(EPOLY_IN_RENDER);
	if (!pblock) return 0;
	int update, recalc;
#ifndef NO_OUTPUTRENDERER
	int useRIter, useRSharp;
#endif
	pblock->GetValue (ep_surf_update, t, update, FOREVER);
	recalc = (update==1);
#ifndef NO_OUTPUTRENDERER
	if (!recalc) {
		pblock->GetValue (ep_surf_use_riter, t, useRIter, FOREVER);
		pblock->GetValue (ep_surf_use_rthresh, t, useRSharp, FOREVER);
		if (useRIter || useRSharp) recalc = 1;
	}
#endif
	if (recalc) {
		subdivValid.SetEmpty ();
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

int EditPolyObject::RenderEnd(TimeValue t) {
	ClearFlag(EPOLY_IN_RENDER);
	if (!pblock) return 0;
	int update, recalc=FALSE;
#ifndef NO_OUTPUTRENDERER
	int useRIter, useRSharp;
#endif
	pblock->GetValue (ep_surf_update, t, update, FOREVER);
	recalc = (update==1);
#ifndef NO_OUTPUTRENDERER
	if (!recalc) {
		pblock->GetValue (ep_surf_use_riter, t, useRIter, FOREVER);
		pblock->GetValue (ep_surf_use_rthresh, t, useRSharp, FOREVER);
		if (useRIter || useRSharp) recalc = true;
	}
#endif
	if (recalc) {
		subdivValid.SetEmpty ();
		if (update==1) SetFlag (EPOLY_FORCE);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	return 0;
}

Mesh* EditPolyObject::GetRenderMesh(TimeValue t, INode *inode, View &view,  BOOL& needDelete) {
	UpdateEverything (t);

	int useSubdiv=false;
	if (pblock) pblock->GetValue (ep_surf_subdivide, t, useSubdiv, FOREVER);
	if (!useSubdiv && !EpPreviewOn()) return PolyObject::GetRenderMesh (t, inode, view, needDelete);

	MNMesh *meshToRender;
	if (useSubdiv) meshToRender = &subdivResult;
	else meshToRender = &mPreviewMesh;

	needDelete = TRUE;
	Mesh *ret;
	BOOL needDisp = FALSE;
	if (!GetDisplacementDisable() && (view.flags & RENDER_MESH_DISPLACEMENT_MAP)) {
		// Find out if we need displacement mapping:
		// Get the material
		Mtl* pMtl = inode ? inode->GetMtl() : NULL;

		if (pMtl) {
			for (int i = 0; i< meshToRender->numf; i++) {
				if (pMtl->Requirements(meshToRender->f[i].material)&MTLREQ_DISPLACEMAP) {
					needDisp = TRUE;
					break;
				}
			}
		}
	}

	if (!needDisp) {
		ret = new class Mesh;
		meshToRender->OutToTri (*ret);
		return ret;
	}

	// Otherwise we have displacement mapping.
	// Create a TriObject from this, and get its render mesh.
	TriObject *tobj = (TriObject *) ConvertToType (t, triObjectClassID);
	if (!tobj) {
		// shouldn't happen
		DbgAssert(0);
		ret = new class Mesh;
		meshToRender->OutToTri (*ret);
		return ret;
	}
	BOOL subNeedDelete;
	ret = tobj->GetRenderMesh (t, inode, view, subNeedDelete);
	if (!subNeedDelete) {
		// Shouldn't happen, because of displacement parameters.
		Mesh *ret2 = ret;
		ret = new Mesh (*ret2);
	}
	delete tobj;
	return ret;
}

Interval EditPolyObject::ChannelValidity (TimeValue t, int nchan) {
	if (nchan != DISP_APPROX_CHANNEL) return PolyObject::ChannelValidity (t, nchan);
	return displacementSettingsValid;
}

int EditPolyObject::GetSubobjectLevel() {
	return selLevel;
}

void EditPolyObject::SetSubobjectLevel(int level) {
	int oldSelLevel = selLevel;
	selLevel = level;
	mm.selLevel = meshSelLevel[level];
	subdivResult.selLevel = meshSelLevel[level];
	UpdateDisplayFlags ();

	InvalidateTempData (PART_SUBSEL_TYPE);
	if (editObj==this) UpdateUIToSelectionLevel (oldSelLevel);
	InvalidateNumberSelected ();
	NotifyDependents (FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);
	if (ip) ip->RedrawViews (ip->GetTime());
}

bool CheckNodeSelection (Interface *ip, INode *inode) {
	if (!ip) return FALSE;
	if (!inode) return FALSE;
	int i, nct = ip->GetSelNodeCount();
	for (i=0; i<nct; i++) if (ip->GetSelNode (i) == inode) return TRUE;
	return FALSE;
}

// Indicates whether the Show Cage checkbox is relevant:
bool EditPolyObject::ShowGizmoConditions () {
	if (!ip) return FALSE;
	if (editObj != this) return FALSE;
	if (selLevel == EP_SL_OBJECT) return false;
	if (ip->GetShowEndResult()) return TRUE;
	int subdiv;
	pblock->GetValue (ep_surf_subdivide, TimeValue(0), subdiv, FOREVER);
	if (subdiv) return TRUE;
	return FALSE;
}

bool EditPolyObject::ShowGizmo () {
	if (!ip) return FALSE;
	if (editObj != this) return FALSE;
	if (selLevel == EP_SL_OBJECT) return false;
	if (pblock == NULL) return false;
	int showCage;
	pblock->GetValue (ep_show_cage, TimeValue(0), showCage, FOREVER);
	if (!showCage) return false;
	if (ip->GetShowEndResult()) return true;
	int subdiv;
	pblock->GetValue (ep_surf_subdivide, TimeValue(0), subdiv, FOREVER);
	if (subdiv) return true;
	return false;
}

void EditPolyObject::UpdateDisplayFlags () {
	mm.dispFlags = epLevelToDispFlags[selLevel];

	// If we're in a command mode that wants something shown, show it.
	if (mDispLevelOverride) mm.SetDispFlag (mDispLevelOverride);

	// If we're showing the gizmo, always suspend showing the end result vertices.
	if (ShowGizmo()) mm.ClearDispFlag (MNDISP_SELVERTS|MNDISP_VERTTICKS);

	// Copy the results to the dependent meshes:
	subdivResult.dispFlags = mm.dispFlags;
	mPreviewMesh.dispFlags = mm.dispFlags;

	// CAL-03/20/03: clear/set the MNDISP_HIDE_SUBDIVISION_INTERIORS flag to show/hide interior edges
	if (pblock && pblock->GetInt(ep_surf_isoline_display))
		subdivResult.SetDispFlag(MNDISP_HIDE_SUBDIVISION_INTERIORS);
	else
		subdivResult.ClearDispFlag(MNDISP_HIDE_SUBDIVISION_INTERIORS);
}

int EditPolyObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) {
	if (!ShowGizmo()) return 0;

	// Set up GW
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 tm = inode->GetObjectTM(t);

	DWORD savedLimits = gw->getRndLimits();
	gw->setRndLimits((savedLimits & ~GW_ILLUM) | GW_ALL_EDGES);
	gw->setTransform(tm);

	if (sliceMode) DisplaySlicePlane (gw);
	if (ip->GetCommandMode () == createFaceMode) createFaceMode->Display (gw);

	// Display gizmo of preview mesh if appropriate.
	UpdateEverything (t);
	MNMesh *pMeshToDisplay = &mm;
	MNTempData *pTempData = TempData();
	if (EpPreviewOn()) {
		pMeshToDisplay = &mPreviewMesh;
		pTempData = PreviewTempData ();
	}

#ifdef MESH_CAGE_BACKFACE_CULLING
	if (savedLimits & GW_BACKCULL) pMeshToDisplay->UpdateBackfacing (gw, true);
#endif

	// We need to draw a "gizmo" version of the polymesh:
	Point3 colSel=GetSubSelColor();
	Point3 colTicks=GetUIColor (COLOR_VERT_TICKS);
	Point3 colGiz=GetUIColor(COLOR_GIZMOS);
	Point3 colGizSel=GetUIColor(COLOR_SEL_GIZMOS);
	gw->setColor (LINE_COLOR, colGiz);
	Point3 rp[3];
	int i;
	int es[3];
	bool edgeLev = (meshSelLevel[selLevel] == MNM_SL_EDGE);
	bool faceLev = (meshSelLevel[selLevel] == MNM_SL_FACE);
	bool diagonals = (ip->GetCommandMode() == editTriMode);
	if (diagonals) {
		for (i=0; i<pMeshToDisplay->numf; i++) {
			MNFace *pFace = pMeshToDisplay->F(i);
			if (pFace->GetFlag (MN_DEAD)) continue;
			if (pFace->GetFlag (MN_HIDDEN)) continue;
			if (pFace->deg < 4) continue;
#ifdef MESH_CAGE_BACKFACE_CULLING
			if ((savedLimits & GW_BACKCULL) && pFace->GetFlag (MN_BACKFACING)) continue;
#endif
			es[0] = GW_EDGE_INVIS;
			if (pFace->GetFlag (MN_SEL)) gw->setColor (LINE_COLOR, colGizSel);
			else gw->setColor (LINE_COLOR, colGiz);

			for (int j=0; j<pFace->deg-3; j++) {
				int jj = j*2;
				rp[0] = pMeshToDisplay->v[pFace->vtx[pFace->diag[jj]]].p;
				rp[1] = pMeshToDisplay->v[pFace->vtx[pFace->diag[jj+1]]].p;
			}
			gw->polyline (2, rp, NULL, NULL, FALSE, es);
		}
	}
	for (i=0; i<pMeshToDisplay->nume; i++) {
		MNEdge *pEdge = pMeshToDisplay->E(i);
		if (pEdge->GetFlag (MN_DEAD)) continue;
		if (pMeshToDisplay->f[pEdge->f1].GetFlag (MN_HIDDEN) && 
			((pEdge->f2 < 0) || pMeshToDisplay->f[pEdge->f2].GetFlag (MN_HIDDEN))) continue;
#ifdef MESH_CAGE_BACKFACE_CULLING
		if ((savedLimits & GW_BACKCULL) && pEdge->GetFlag (MN_BACKFACING)) continue;
#endif
		if (pEdge->GetFlag (MN_EDGE_INVIS)) {
			if (!edgeLev) continue;
			es[0] = GW_EDGE_INVIS;
		} else {
			es[0] = GW_EDGE_VIS;
		}
		if (!sliceMode) {
			if (edgeLev) {
				if (pEdge->GetFlag (MN_SEL)) gw->setColor (LINE_COLOR, colGizSel);
				else gw->setColor (LINE_COLOR, colGiz);
			}
			if (faceLev) {
				if (pMeshToDisplay->f[pEdge->f1].GetFlag (MN_SEL) ||
					((pEdge->f2 >= 0) && pMeshToDisplay->f[pEdge->f2].GetFlag (MN_SEL)))
					gw->setColor (LINE_COLOR, colGizSel);
				else gw->setColor (LINE_COLOR, colGiz);
			}
		}
		rp[0] = pMeshToDisplay->v[pEdge->v1].p;
		rp[1] = pMeshToDisplay->v[pEdge->v2].p;
		gw->polyline (2, rp, NULL, NULL, FALSE, es);
	}

	if ((ip->GetCommandMode()==createFaceMode) ||
		(ip->GetCommandMode()==createEdgeMode) ||
		(ip->GetCommandMode()==editTriMode) ||
		(selLevel == EP_SL_VERTEX)) {
		int softSel, useEdgeDist, edgeIts, affectBack;
		float falloff, pinch, bubble;
		Interval frvr = FOREVER;
		pblock->GetValue (ep_ss_use, t, softSel, frvr);
		if (softSel) {
			pblock->GetValue (ep_ss_use, t, useEdgeDist, frvr);
			if (useEdgeDist) pblock->GetValue (ep_ss_use, t, edgeIts, frvr);
			pblock->GetValue (ep_ss_use, t, affectBack, frvr);
			pblock->GetValue (ep_ss_falloff, t, falloff, frvr);
			pblock->GetValue (ep_ss_pinch, t, pinch, frvr);
			pblock->GetValue (ep_ss_bubble, t, bubble, frvr);
		}
		float *ourvw = NULL;
		if (softSel) {
			ourvw = pTempData->VSWeight (useEdgeDist, edgeIts, !affectBack, falloff, pinch, bubble)->Addr(0);
		}
		for (i=0; i<pMeshToDisplay->numv; i++) {
			MNVert *pVert = pMeshToDisplay->V(i);
			if (pVert->GetFlag (MN_HIDDEN)) continue;
#ifdef MESH_CAGE_BACKFACE_CULLING
			if ((savedLimits & GW_BACKCULL) && !getDisplayBackFaceVertices()) {
				if (pVert->GetFlag(MN_BACKFACING)) continue;
			}
#endif
			if (pVert->GetFlag (MN_SEL)) gw->setColor (LINE_COLOR, colSel);
			else {
				if (ourvw) gw->setColor (LINE_COLOR, SoftSelectionColor(ourvw[i]));
				else gw->setColor (LINE_COLOR, colTicks);
			}

			if(getUseVertexDots()) gw->marker (&(pVert->p), VERTEX_DOT_MARKER(getVertexDotType()));
			else gw->marker (&(pVert->p), PLUS_SIGN_MRKR);
		}
	}

	gw->setRndLimits(savedLimits);
	return 0;	
}

void EditPolyObject::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {
	box.Init();
	if (!ShowGizmo()) return;
	UpdateEverything (t);
	Matrix3 tm = inode->GetObjectTM(t);
	// We need to draw a "gizmo" version of the mesh:
	box = mm.getBoundingBox (&tm);
	if (!CheckNodeSelection (ip, inode)) return;

	if (sliceMode) GetSlicePlaneBoundingBox (box, &tm);
}

int EditPolyObject::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags) {
	UpdateEverything (t);

	int useSubdiv=false;
	if (pblock) pblock->GetValue (ep_surf_subdivide, t, useSubdiv, FOREVER);

	MNMesh *pMeshToDisplay = NULL;
	if (EpPreviewOn()) pMeshToDisplay = &mPreviewMesh;
	if (useSubdiv) pMeshToDisplay = &subdivResult;

	if (pMeshToDisplay) {
		float *vsw = NULL;
		DWORD oldRndLimits, newRndLimits;
		Tab<UVVert> vertexSelColors;
		Tab<MNMapFace> vertexSelFaces;

		// NOTE: code below based on PolyObject::Display.
		GraphicsWindow *gw = vpt->getGW();
		gw->setTransform(inode->GetObjectTM(t));

		newRndLimits = oldRndLimits = gw->getRndLimits();

		// CAL-11/16/01: Use soft selection colors when soft selection is turned on in sub-object mode.
		// The condition is satisfied if the display of vertex/edge/face is requested and
		// the vertex selection weights are set.
		// CAL-12/02/01: Add one more checking to the condition to check if the display of soft
		// selection using wireframe or vertex color shading is requested.
		// CAL-11/16/01: (TODO) To imporvement the performance of this function,
		// vertexSelColors & vertexSelFaces tables can be cached (in MNMesh) and used as long as
		// the vertex selection weights are not changed.

		// Note - in the following line, we use subobject level as a test instead of the display flags
		// because the vertex-level display flags are usually turned off when the subdivResult is engaged.
		bool dspSoftSelVert = (selLevel == EP_SL_VERTEX) ? true : false;
		bool dspSoftSelEdFc = ((pMeshToDisplay->dispFlags&MNDISP_SELEDGES) || (pMeshToDisplay->dispFlags&MNDISP_SELFACES)) &&
							  (flags&DISP_SHOWSUBOBJECT);
		bool dspWireSoftSel = (oldRndLimits&GW_WIREFRAME) ||
							  ((oldRndLimits&GW_COLOR_VERTS) && (inode->GetVertexColorType() == nvct_soft_select));

		if ((dspSoftSelVert || dspSoftSelEdFc) && dspWireSoftSel &&
			(vsw = pMeshToDisplay->getVSelectionWeights()) && pMeshToDisplay->numv) {

			Point3 clr = GetUIColor(COLOR_VERT_TICKS);
			
			vertexSelColors.SetCount(pMeshToDisplay->VNum());
			vertexSelFaces.SetCount(pMeshToDisplay->FNum());

			// Create the array of colors, one per vertex:
			for (int i=0; i<pMeshToDisplay->VNum(); i++) {
				// (Note we may want a different color - this gives the appropriate vertex-tick
				// color, which we may not want to use on faces.  Fades from blue to red.)
				vertexSelColors[i] = (vsw[i]) ? SoftSelectionColor(vsw[i]) : clr;
			}

			// Copy over the face topology exactly to the map face topology:
			for (i=0; i<pMeshToDisplay->FNum(); i++) {
				vertexSelFaces[i].Init();
				vertexSelFaces[i] = pMeshToDisplay->f[i];
			}

			// CAL-05/21/02: make sure there's data before accessing it.
			// Set the mesh to use these colors:
			pMeshToDisplay->SetDisplayVertexColors ((vertexSelColors.Count() > 0) ? vertexSelColors.Addr(0) : NULL,
									   (vertexSelFaces.Count() > 0) ? vertexSelFaces.Addr(0) : NULL);

			// Turn on vertex color mode
			if (oldRndLimits&GW_WIREFRAME) {
				newRndLimits |= GW_COLOR_VERTS;				// turn on vertex colors
				newRndLimits &= ~GW_SHADE_CVERTS;			// turn off vertex color shading
				newRndLimits |= GW_ILLUM;					// turn on lit wire frame
				if (mm.dispFlags&MNDISP_DIAGONALS)
					newRndLimits |= GW_ALL_EDGES;			// turn on diagonal display
				gw->setRndLimits(newRndLimits);
			}

		} else {
			switch (inode->GetVertexColorType()) {
			case nvct_color:
				pMeshToDisplay->SetDisplayVertexColors (0);
				break;
			case nvct_illumination:
				pMeshToDisplay->SetDisplayVertexColors (MAP_SHADING);
				break;
			case nvct_alpha:
				pMeshToDisplay->SetDisplayVertexColors (MAP_ALPHA);
				break;
			// CAL-06/15/03: add a new option to view map channel as vertex color. (FID #1926)
			case nvct_map_channel:
				pMeshToDisplay->SetDisplayVertexColors (inode->GetVertexColorMapChannel());
				break;
			case nvct_soft_select:
				// Turn off vertex color if soft selection is not on.
				if (oldRndLimits&GW_COLOR_VERTS) {
					newRndLimits &= ~GW_COLOR_VERTS;
					gw->setRndLimits(newRndLimits);
				}
				break;
			}
		}
		pMeshToDisplay->render (gw, inode->Mtls(),
			(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, 
			COMP_ALL | ((flags&DISP_SHOWSUBOBJECT)?COMP_OBJSELECTED:0),
			inode->NumMtls());

		// Clear out our temporary data:
		if ( vsw ) {
			for (int i=0; i<vertexSelFaces.Count(); i++)
				vertexSelFaces[i].Clear();
			mm.SetDisplayVertexColors (NULL, NULL);
		}

		// Don't forget the gizmo - if this node is selected, that is:
		if (useSubdiv && inode->Selected ()) Display (t, inode, vpt, flags, NULL);
	} else {
		// Commented out 4/26/01 - was causing vertex ticks to show up on unselected nodes.
		// GraphicsWindow *gw = vpt->getGW();
		// DWORD oldLimits = gw->getRndLimits ();
		//if (mm.GetDispFlag (MNDISP_VERTTICKS)) {
			//gw->setRndLimits (oldLimits | GW_VERT_TICKS);
		//}
		PolyObject::Display (t, inode, vpt, flags);
		//gw->setRndLimits (oldLimits);
	}
	if (!CheckNodeSelection (ip, inode)) return 0;
	GraphicsWindow *gw = vpt->getGW();
	Matrix3 tm = inode->GetObjectTM(t);
	int savedLimits;

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);

	if (sliceMode) DisplaySlicePlane (gw);
	if (ip->GetCommandMode () == createFaceMode) createFaceMode->Display (gw);

	gw->setRndLimits(savedLimits);
	return 0;
}

// World and local bounding boxes are unaffected by subdivision, since the
// subdivided mesh is always (?) within the "convex hull" of the original mesh.
// However, previews can definitely affect the bounding box.
void EditPolyObject::GetWorldBoundBox (TimeValue t, INode* inode, ViewExp* vp, Box3& box ) {
	UpdateEverything (t);

	if (EpPreviewOn()) {
		Box3 meshBox;
		mPreviewMesh.BBox (meshBox);
		if (meshBox.IsEmpty()) box = meshBox;
		else {
			Matrix3 mat = inode->GetObjectTM(t);
			box.Init();
			for (int i = 0; i < 8; i++) box += mat * meshBox[i];
		}
	} else {
		PolyObject::GetWorldBoundBox (t, inode, vp, box);
	}

	if (!CheckNodeSelection (ip, inode)) return;

	Matrix3 tm = inode->GetObjectTM(t);

	if (sliceMode) GetSlicePlaneBoundingBox (box, &tm);
}

void EditPolyObject::GetLocalBoundBox (TimeValue t, INode* inode, ViewExp* vp, Box3& box ) {
	UpdateEverything (t);

	if (EpPreviewOn()) {
		mPreviewMesh.BBox (box);
	} else {
		PolyObject::GetLocalBoundBox (t, inode, vp, box);
	}

	if (!CheckNodeSelection (ip, inode)) return;

	if (sliceMode) GetSlicePlaneBoundingBox (box);
}

// What subobject level should we be hit-testing on?
DWORD EditPolyObject::CurrentHitLevel (int *selByVert) {
	// Under normal circumstances, pretty much the level we're at.
	DWORD hitLev = hitLevel[selLevel];

	// But we might be selecting by vertex...
	int sbv;
	if (selByVert == NULL) selByVert = &sbv;
	pblock->GetValue (ep_by_vertex, TimeValue(0), *selByVert, FOREVER);
	if (*selByVert) {
		hitLev = SUBHIT_MNVERTS;
		if (selLevel != EP_SL_VERTEX) hitLev |= SUBHIT_MNUSECURRENTSEL;
		if (selLevel == EP_SL_BORDER) hitLev |= SUBHIT_OPENONLY;
	}

	// And if we're in a command mode, it may override the current hit level:
	if (mHitLevelOverride != 0x0) hitLev = mHitLevelOverride;
	return hitLev;
}

void MakeMyHitRegion(HitRegion& hr, int type, int crossing, int epsi, IPoint2 *p){
	IPoint2 v;
	hr.epsilon = epsi;
	hr.crossing = crossing;

	if(type == HITTYPE_SOLID || type == HITTYPE_POINT)
		hr.dir = RGN_DIR_UNDEF;
	else {
		if(p[0].x < p[1].x)
			hr.dir = RGN_DIR_RIGHT;
		else
			hr.dir = RGN_DIR_LEFT;
	}

	switch ( type ) {
	case HITTYPE_SOLID:
	case HITTYPE_POINT:
		hr.type = POINT_RGN;
		hr.pt.x = p[0].x;
		hr.pt.y = p[0].y;
		break;
	
	case HITTYPE_BOX:
		hr.type        = RECT_RGN;
		hr.rect.left   = p[0].x;
		hr.rect.right  = p[1].x;
		hr.rect.top    = p[0].y;
		hr.rect.bottom = p[1].y;
		((Box2*)(&hr.rect))->Rectify();
		break;

	case HITTYPE_CIRCLE:
		v = p[0] - p[1];
		hr.type			= CIRCLE_RGN;
		hr.circle.x		= p[0].x;
		hr.circle.y		= p[0].y;
		hr.circle.r		= max(abs(v.x), abs(v.y));	// this is how the circle is drawn!! DB 3/2
		break;

	case HITTYPE_LASSO:
	case HITTYPE_FENCE:
		int ct;
		ct = 0;
		while (p[ct].x != -100 && p[ct].y != -100) ct++;
		hr.type			= FENCE_RGN;
		hr.epsilon		= ct;
		hr.pts			= (POINT *)p;
		break;
	}
}

int EditPolyObject::HitTest (TimeValue t, INode *inode, int type, int crossing, int flags,
							 IPoint2 *p, ViewExp *vpt)
{
	UpdateEverything (t);

	int useSubdiv=false;
	if (pblock) pblock->GetValue (ep_surf_subdivide, t, useSubdiv, FOREVER);

	MNMesh *pMeshToDisplay = &mm;
	if (EpPreviewOn()) pMeshToDisplay = &mPreviewMesh;
	if (useSubdiv) pMeshToDisplay = &subdivResult;

	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();
	MakeMyHitRegion (hitRegion,type,crossing,4,p);	
	gw->setTransform(inode->GetObjectTM(t));
	return pMeshToDisplay->select( gw, inode->Mtls(), &hitRegion, flags & HIT_ABORTONHIT, inode->NumMtls() );
}

int EditPolyObject::HitTest (TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	Interval valid;
	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	
	// Setup GW
	MakeHitRegion(hr,type, crossing, DEF_PICKBOX_SIZE, p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);

	int ignoreBack;
	pblock->GetValue (ep_ignore_backfacing, t, ignoreBack, FOREVER);
	if (mForceIgnoreBackfacing) ignoreBack = true;
	if (ignoreBack) gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
	else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
	gw->clearHitCode();	

	// Update selection converter to respect latest selection settings:
	mSelConv.SetFlag (MNM_SELCONV_REQUIRE_ALL,
		(hr.crossing||(type==HITTYPE_POINT)) ? false : true);

	if (sliceMode && CheckNodeSelection (ip, inode)) {
		DisplaySlicePlane (gw);
		if (gw->checkHitCode()) {
			vpt->LogHit (inode, mc, gw->getHitDistance(), 0, NULL);
			res = 1;
		}
		gw->setRndLimits (savedLimits);
		return res;
	}

	SubObjHitList hitList;
	MeshSubHitRec *rec;
	int selByVert;
	DWORD hitLev = CurrentHitLevel (&selByVert);
	DWORD thisFlags = hitLev | flags;
	res = mm.SubObjectHitTest(gw, gw->getMaterial(), &hr, thisFlags, hitList);

	rec = hitList.First();
	while (rec) {
		vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
		rec = rec->Next();
	}

	gw->setRndLimits(savedLimits);	
	return res;
}

void EditPolyObject::SelectSubComponent (HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert) {
	if (selLevel == EP_SL_OBJECT) return;
	if (!ip) return;
	if (sliceMode) return;
	ip->ClearCurNamedSelSet ();
	TimeValue t = ip->GetTime();

	int selByVert;
	pblock->GetValue (ep_by_vertex, t, selByVert, FOREVER);

	// Compose hits into a single bitarray:
	BitArray hitArray;
	if (selByVert || (selLevel == EP_SL_VERTEX)) {
		hitArray.SetSize (mm.numv);
	} else {
		if (meshSelLevel[selLevel] == MNM_SL_EDGE) {
			hitArray.SetSize (mm.nume);
		} else {
			hitArray.SetSize (mm.numf);
		}
	}

	HitRecord *hr;
	hitArray.ClearAll ();
	for (hr = hitRec; hr!=NULL; hr=hr->Next()) {
		hitArray.Set (hr->hitInfo);
		if (!all) break;
	}

	// Use that hitArray to create new selection:
	BitArray *newSel;
	BitArray currentSel, intermediateSel, nsel;
	switch (selLevel) {
	case EP_SL_VERTEX:
		mm.getVertexSel (currentSel);
		if (invert) currentSel ^= hitArray;
		else {
			if (selected) currentSel |= hitArray;
			else currentSel &= ~hitArray;
		}
		SetVertSel (currentSel, this, t);
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Vertex"), mr_bitarray, &vsel);
		break;

	case EP_SL_EDGE:
		mm.getEdgeSel (currentSel);
		if (selByVert) {
			mSelConv.VertexToEdge (mm, hitArray, nsel);
			newSel = &nsel;
		} else newSel = &hitArray;

		if (invert) currentSel ^= *newSel;
		else {
			if (selected) currentSel |= *newSel;
			else currentSel &= ~(*newSel);
		}

		SetEdgeSel (currentSel, this, t);
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Edge"), mr_bitarray, &esel);
		break;

	case EP_SL_BORDER:
		mm.getEdgeSel (currentSel);
		if (selByVert) {
			mSelConv.VertexToEdge (mm, hitArray, intermediateSel);
			mSelConv.EdgeToBorder (mm, intermediateSel, nsel);
		} else {
			mSelConv.EdgeToBorder (mm, hitArray, nsel);
		}

		if (invert) currentSel ^= nsel;
		else {
			if (selected) currentSel |= nsel;
			else currentSel &= ~nsel;
		}

		SetEdgeSel (currentSel, this, t);
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Edge"), mr_bitarray, &esel);
		break;

	case EP_SL_FACE:
		mm.getFaceSel (currentSel);
		if (selByVert) {
			mSelConv.VertexToFace (mm, hitArray, nsel);
			newSel = &nsel;
		} else newSel = &hitArray;

		if (invert) currentSel ^= *newSel;
		else {
			if (selected) currentSel |= *newSel;
			else currentSel &= ~(*newSel);
		}

		SetFaceSel (currentSel, this, t);
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Face"), mr_bitarray, &fsel);
		break;

	case EP_SL_ELEMENT:
		if (selByVert) {
			mSelConv.VertexToFace (mm, hitArray, intermediateSel);
			mSelConv.FaceToElement (mm, intermediateSel, nsel);
		} else {
			mSelConv.FaceToElement (mm, hitArray, nsel);
		}

		mm.getFaceSel (currentSel);
		if (invert) currentSel ^= nsel;
		else {
			if (selected) currentSel |= nsel;
			else currentSel &= ~nsel;
		}
		SetFaceSel (currentSel, this, t);

		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Face"), mr_bitarray, &fsel);
		break;
	}
	LocalDataChanged(PART_SELECT);
	RefreshScreen ();
}

void EditPolyObject::ClearSelection(int sl) {
	if (sl == EP_SL_CURRENT) sl = selLevel;
	if (sliceMode) return;
	BitArray sel;
	switch (sl) {
	case EP_SL_OBJECT: return;
	case EP_SL_VERTEX:
		sel.SetSize (mm.numv);
		sel.ClearAll ();
		SetVertSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Vertex"), mr_bitarray, &vsel);
		break;
	case EP_SL_EDGE:
	case EP_SL_BORDER:
		sel.SetSize (mm.nume);
		sel.ClearAll();
		SetEdgeSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Edge"), mr_bitarray, &esel);
		break;
	default:
		sel.SetSize (mm.numf);
		sel.ClearAll ();
		SetFaceSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Face"), mr_bitarray, &fsel);
		break;
	}
	LocalDataChanged (PART_SELECT);
	RefreshScreen ();
}

void EditPolyObject::SelectAll(int sl) {
	if (sl == EP_SL_CURRENT) sl = selLevel;
	if (sl == EP_SL_OBJECT) return;
	if (sliceMode) return;
	BitArray sel;
	int i;
	switch (sl) {
	case EP_SL_VERTEX: 
		sel.SetSize (mm.numv);
		sel.SetAll();
		SetVertSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Vertex"), mr_bitarray, &vsel);
		break;
	case EP_SL_EDGE:
		sel.SetSize (mm.nume);
		sel.SetAll();
		SetEdgeSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Edge"), mr_bitarray, &esel);
		break;
	case EP_SL_BORDER:
		sel.SetSize (mm.nume);
		for (i=0; i<mm.nume; i++) sel.Set (i, mm.e[i].f2<0);
		SetEdgeSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Edge"), mr_bitarray, &esel);
		break;
	default:
		sel.SetSize (mm.numf);
		sel.SetAll(); 
		SetFaceSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Face"), mr_bitarray, &fsel);
		break;
	}
	LocalDataChanged (PART_SELECT);
	RefreshScreen ();
}

void EditPolyObject::InvertSelection(int sl) {
	if (sl == EP_SL_CURRENT) sl = selLevel;
	if (sl == EP_SL_OBJECT) return;
	if (sliceMode) return;
	BitArray sel;
	switch (sl) {
	case EP_SL_VERTEX:
		mm.getVertexSel (sel);
		sel = ~sel;
		SetVertSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Vertex"), mr_bitarray, &vsel);
		break;
	case EP_SL_EDGE:
	case EP_SL_BORDER:
		mm.getEdgeSel (sel);
		sel = ~sel;
		if (sl == EP_SL_BORDER) {
			for (int i=0; i<mm.nume; i++) if (mm.e[i].f2>-1) sel.Clear(i);
		}
		SetEdgeSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Edge"), mr_bitarray, &esel);
		break;
	default:
		mm.getFaceSel (sel);
		sel = ~sel;
		SetFaceSel (sel, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Face"), mr_bitarray, &fsel);
		break;
	}
	LocalDataChanged (PART_SELECT);
	RefreshScreen ();
}

void EditPolyObject::EpfnGrowSelection (int mnSelLevel) {
	if (mnSelLevel == MNM_SL_CURRENT) mnSelLevel = meshSelLevel[selLevel];
	DbgAssert (mm.GetFlag (MN_MESH_FILLED_IN));
	if (!mm.GetFlag (MN_MESH_FILLED_IN)) return;
	BitArray newSel;
	int i;
	switch (mnSelLevel) {
	case MNM_SL_VERTEX:
		mm.ClearEFlags (MN_USER);
		mm.PropegateComponentFlags (MNM_SL_EDGE, MN_USER, mnSelLevel, MN_SEL);
		newSel.SetSize (mm.numv);
		for (i=0; i<mm.nume; i++) {
			if (mm.e[i].GetFlag (MN_USER)) {
				newSel.Set (mm.e[i].v1);
				newSel.Set (mm.e[i].v2);
			}
		}
		SetVertSel (newSel, this, TimeValue(0));
		break;

	case MNM_SL_EDGE:
		mm.ClearVFlags (MN_USER);
		mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL);
		newSel.SetSize (mm.nume);
		for (i=0; i<mm.nume; i++) {
			if (mm.v[mm.e[i].v1].GetFlag (MN_USER) || mm.v[mm.e[i].v2].GetFlag (MN_USER))
				newSel.Set (i);
		}
		SetEdgeSel (newSel, this, TimeValue(0));
		break;

	case MNM_SL_FACE:
		mm.ClearVFlags (MN_USER);
		mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL);
		newSel.SetSize (mm.numf);
		for (i=0; i<mm.numf; i++) {
			for (int j=0; j<mm.f[i].deg; j++) {
				if (mm.v[mm.f[i].vtx[j]].GetFlag (MN_USER)) break;
			}
			if (j<mm.f[i].deg) newSel.Set (i);
		}
		SetFaceSel (newSel, this, TimeValue(0));
		break;
	}
	macroRecorder->FunctionCall(_T("$.EditablePoly.GrowSelection"), 0, 0);
	macroRecorder->EmitScript ();
	LocalDataChanged (PART_SELECT);
}

void EditPolyObject::EpfnShrinkSelection (int mnSelLevel) {
	if (mnSelLevel == MNM_SL_CURRENT) mnSelLevel = meshSelLevel[selLevel];
	DbgAssert (mm.GetFlag (MN_MESH_FILLED_IN));
	if (!mm.GetFlag (MN_MESH_FILLED_IN)) return;
	BitArray newSel;
	int i;
	switch (mnSelLevel) {
	case MNM_SL_VERTEX:
		// Find the edges between two selected vertices.
		mm.ClearEFlags (MN_USER);
		mm.PropegateComponentFlags (MNM_SL_EDGE, MN_USER, mnSelLevel, MN_SEL, true);
		if (vsel.GetSize() != mm.numv) mm.getVertexSel (vsel);
		newSel = vsel;
		// De-select all the vertices touching edges to unselected vertices:
		for (i=0; i<mm.nume; i++) {
			if (!mm.e[i].GetFlag (MN_USER)) {
				newSel.Clear (mm.e[i].v1);
				newSel.Clear (mm.e[i].v2);
			}
		}
		SetVertSel (newSel, this, TimeValue(0));
		break;

	case MNM_SL_EDGE:
		// Find all vertices used by only selected edges:
		mm.ClearVFlags (MN_USER);
		mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL, true);
		if (esel.GetSize() != mm.nume) mm.getEdgeSel (esel);
		newSel = esel;
		for (i=0; i<mm.nume; i++) {
			// Deselect edges with at least one vertex touching a nonselected edge:
			if (!mm.v[mm.e[i].v1].GetFlag (MN_USER) || !mm.v[mm.e[i].v2].GetFlag (MN_USER))
				newSel.Clear (i);
		}
		SetEdgeSel (newSel, this, TimeValue(0));
		break;

	case MNM_SL_FACE:
		// Find all vertices used by only selected faces:
		mm.ClearVFlags (MN_USER);
		mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, mnSelLevel, MN_SEL, true);
		if (fsel.GetSize() != mm.numf) mm.getFaceSel (fsel);
		newSel = fsel;
		for (i=0; i<mm.numf; i++) {
			for (int j=0; j<mm.f[i].deg; j++) {
				if (!mm.v[mm.f[i].vtx[j]].GetFlag (MN_USER)) break;
			}
			// Deselect faces with at least one vertex touching a nonselected face:
			if (j<mm.f[i].deg) newSel.Clear (i);
		}
		SetFaceSel (newSel, this, TimeValue(0));
		break;
	}
	macroRecorder->FunctionCall(_T("$.EditablePoly.ShrinkSelection"), 0, 0);
	macroRecorder->EmitScript ();
	LocalDataChanged (PART_SELECT);
}

static TCHAR *selLevelNames[] = {_T("Object"), _T("Vertex"), _T("Edge"), _T("Border"), _T("Face"), _T("Element") };

int EditPolyObject::EpfnConvertSelection (int epSelLevelFrom, int epSelLevelTo, bool requireAll) {
	DbgAssert (mm.GetFlag (MN_MESH_FILLED_IN));
	if (!mm.GetFlag (MN_MESH_FILLED_IN)) return 0;

	if (epSelLevelFrom == EP_SL_CURRENT) epSelLevelFrom = selLevel;
	if (epSelLevelTo == EP_SL_CURRENT) epSelLevelTo = selLevel;
	if (epSelLevelFrom == epSelLevelTo) return 0;

	// We don't select the whole object here.
	if (epSelLevelTo == EP_SL_OBJECT) return 0;

	// If 'from' is the whole object, we select all.
	int numComponentsSelected = 0;
	BitArray sel;
	int i;
	if (epSelLevelFrom == EP_SL_OBJECT) {
		switch (epSelLevelTo) {
		case EP_SL_VERTEX: 
			sel.SetSize (mm.numv);
			sel.SetAll();
			SetVertSel (sel, this, ip->GetTime());
			break;
		case EP_SL_EDGE:
			sel.SetSize (mm.nume);
			sel.SetAll();
			SetEdgeSel (sel, this, ip->GetTime());
			break;
		case EP_SL_BORDER:
			sel.SetSize (mm.nume);
			for (i=0; i<mm.nume; i++) sel.Set (i, mm.e[i].f2<0);
			SetEdgeSel (sel, this, ip->GetTime());
			break;
		default:
			sel.SetSize (mm.numf);
			sel.SetAll(); 
			SetFaceSel (sel, this, ip->GetTime());
			break;
		}
		numComponentsSelected = sel.NumberSet();
	} else {
		int mslFrom = meshSelLevel[epSelLevelFrom];
		int mslTo = meshSelLevel[epSelLevelTo];

		if (theHold.Holding()) theHold.Put (new ComponentFlagRestore (this, mslTo));

		// Clear the selection first
		switch (mslTo) {
		case MNM_SL_VERTEX:
			mm.ClearVFlags (MN_SEL);
			break;
		case MNM_SL_EDGE:
			mm.ClearEFlags (MN_SEL);
			break;
		case MNM_SL_FACE:
			mm.ClearFFlags (MN_SEL);
			break;
		}

		// Then propegate flags using the convenient MNMesh method:
		mm.PropegateComponentFlags (mslTo, MN_SEL, mslFrom, MN_SEL, requireAll);

		// Then either update the EPoly selection BitArrays, or convert to Border/Element selections:
		switch (epSelLevelTo) {
		case EP_SL_VERTEX:
			mm.getVertexSel (vsel);
			numComponentsSelected = vsel.NumberSet();
			break;
		case EP_SL_EDGE:
			mm.getEdgeSel (esel);
			numComponentsSelected = esel.NumberSet();
			break;
		case EP_SL_FACE:
			mm.getFaceSel (fsel);
			numComponentsSelected = fsel.NumberSet();
			break;

		case EP_SL_BORDER:
			sel.SetSize(mm.nume);
			sel.ClearAll();

			for (i=0; i<sel.GetSize(); i++) {
				if (!mm.e[i].GetFlag (MN_SEL)) continue;
				if (sel[i]) continue;
				if (mm.e[i].f2 > -1) continue;
				mm.BorderFromEdge (i, sel);
			}
			SetEdgeSel (sel, this, TimeValue(0));
			numComponentsSelected = esel.NumberSet();
			break;

		case EP_SL_ELEMENT:
			sel.SetSize(mm.numf);
			sel.ClearAll();

			for (int i=0; i<mm.numf; i++) {
				if (!mm.f[i].GetFlag (MN_SEL)) continue;
				if (sel[i]) continue;
				mm.ElementFromFace (i, sel);
			}
			SetFaceSel (sel, this, TimeValue(0));
			numComponentsSelected = fsel.NumberSet();
			break;
		}
	}

	if (requireAll) {
		macroRecorder->FunctionCall (_T("$.EditablePoly.ConvertSelection"), 2, 1,
			mr_name, selLevelNames[epSelLevelFrom],
			mr_name, selLevelNames[epSelLevelTo],
			_T("requireAll"), mr_bool, true);
	} else {
		macroRecorder->FunctionCall (_T("$.EditablePoly.ConvertSelection"), 2, 0,
			mr_name, selLevelNames[epSelLevelFrom],
			mr_name, selLevelNames[epSelLevelTo]);
	}
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_SELECT);
	return numComponentsSelected;
}

void EditPolyObject::EpfnSelectElement () {
	BitArray nsel;
	nsel.SetSize(mm.numf);
	nsel.ClearAll();

	for (int i=0; i<nsel.GetSize(); i++) {
		if (!mm.f[i].GetFlag (MN_SEL)) continue;
		if (nsel[i]) continue;
		mm.ElementFromFace (i, nsel);
	}
	SetFaceSel (nsel, this, TimeValue(0));

	macroRecorder->FunctionCall (_T("$.EditablePoly.SelectElement"), 0, 0);
	macroRecorder->EmitScript ();
	LocalDataChanged (PART_SELECT);
}

void EditPolyObject::EpfnSelectBorder () {
	BitArray nsel;
	nsel.SetSize(mm.nume);
	nsel.ClearAll();

	for (int i=0; i<nsel.GetSize(); i++) {
		if (!mm.e[i].GetFlag (MN_SEL)) continue;
		if (nsel[i]) continue;
		if (mm.e[i].f2 > -1) continue;
		mm.BorderFromEdge (i, nsel);
	}
	SetEdgeSel (nsel, this, TimeValue(0));

	macroRecorder->FunctionCall (_T("$.EditablePoly.SelectBorder"), 0, 0);
	macroRecorder->EmitScript ();
	LocalDataChanged (PART_SELECT);
}

void EditPolyObject::EpfnSelectEdgeRing () {
	BitArray nsel = GetEdgeSel ();
	mm.SelectEdgeRing (nsel);
	SetEdgeSel (nsel, this, TimeValue(0));
	macroRecorder->FunctionCall (_T("$.EditablePoly.SelectEdgeRing"), 0, 0);
	macroRecorder->EmitScript ();
	LocalDataChanged (PART_SELECT);
}

void EditPolyObject::EpfnSelectEdgeLoop () {
	BitArray nsel = GetEdgeSel ();
	mm.SelectEdgeLoop (nsel);
	SetEdgeSel (nsel, this, TimeValue(0));
	macroRecorder->FunctionCall (_T("$.EditablePoly.SelectEdgeLoop"), 0, 0);
	macroRecorder->EmitScript ();
	LocalDataChanged (PART_SELECT);
}

BOOL EditPolyObject::SelectSubAnim(int subNum) {
	if (subNum<2) return FALSE;	// cannot select master point controller or pblock.
	subNum -= 2;
	if (subNum >= mm.numv) return FALSE;

	BOOL add = GetKeyState(VK_CONTROL)<0;
	BOOL sub = GetKeyState(VK_MENU)<0;
	BitArray nvs;

	if (add || sub) {
		mm.getVertexSel (nvs);
		if (sub) nvs.Clear (subNum);
		else nvs.Set (subNum);
	} else {
		nvs.SetSize (mm.numv);
		nvs.ClearAll ();
		nvs.Set (subNum);
	}

	if (ip) SetVertSel (nvs, this, ip->GetTime());
	else SetVertSel (nvs, this, TimeValue(0));
	macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
		mr_name, _T("Vertex"), mr_bitarray, &vsel);
	LocalDataChanged (PART_SELECT);
	RefreshScreen ();
	return TRUE;
}

void EditPolyObject::ActivateSubobjSel (int level, XFormModes& modes) {
	// Register or unregister delete, backspace key notification
	if (ip) {
		if (selLevel==EP_SL_OBJECT && level!=EP_SL_OBJECT) {
			ip->RegisterDeleteUser(this);
			backspacer.SetEPoly (this);
			backspaceRouter.Register (&backspacer);
		}
		if (selLevel!=EP_SL_OBJECT && level==EP_SL_OBJECT) {
			ip->UnRegisterDeleteUser(this);
			backspacer.SetEPoly (NULL);
			backspaceRouter.UnRegister (&backspacer);
		}
	}

	// Cancel any popup dialog operations:
	EpfnClosePopupDialog ();

	if (selLevel != level) {
		ExitAllCommandModes (level == EP_SL_OBJECT, level == EP_SL_OBJECT);
	}

	// Set the mesh's level
	SetSubobjectLevel(level);

	// Fill in modes with our sub-object modes
	if (level!=EP_SL_OBJECT)
		modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,selectMode);

	// Setup named selection sets
	if (ip && (level != EP_SL_OBJECT)) {
		GenericNamedSelSetList &set = GetSelSet(namedSetLevel[level]);
		ip->ClearSubObjectNamedSelSets();
		for (int i=0; i<set.Count(); i++) {
			ip->AppendSubObjectNamedSelSet(*(set.names[i]));
		}
	}
	if (ip) 
	{
		ip->PipeSelLevelChanged();
		UpdateNamedSelDropDown ();
	}

	InvalidateTempData (PART_TOPO|PART_GEOM|PART_SELECT|PART_SUBSEL_TYPE);
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
}

GenericNamedSelSetList &EditPolyObject::GetSelSet(int namedSelLevel) {
	return selSet[namedSelLevel];
}

void EditPolyObject::GetSubObjectCenters(SubObjAxisCallback *cb,
										TimeValue t, INode *node,ModContext *mc) {
	Matrix3 tm = node->GetObjectTM(t);

	if (sliceMode) {
		cb->Center (sliceCenter*tm, 0);
		return;
	}

	if (selLevel == EP_SL_OBJECT) return;
	if (selLevel == EP_SL_VERTEX) {
		BitArray sel = mm.VertexTempSel();
		Point3 cent(0,0,0);
		int ct=0;
		for (int i=0; i<mm.numv; i++) {
			if (sel[i]) {
				cent += mm.v[i].p;
				ct++;
			}
		}
		if (ct) {
			cent /= float(ct);			
			cb->Center(cent*tm,0);
		}
		return;
	}

	Tab<Point3> *centers = TempData()->ClusterCenters(meshSelLevel[selLevel]);
	DbgAssert (centers);
	if (!centers) return;
	for (int i=0; i<centers->Count(); i++) cb->Center((*centers)[i]*tm,i);
}

void EditPolyObject::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc) {
	Matrix3 tm, otm = node->GetObjectTM(t);

	if (sliceMode) {
		Matrix3 rotMatrix(1);
		sliceRot.MakeMatrix (rotMatrix);
		rotMatrix.SetTrans (sliceCenter);
		rotMatrix *= otm;
		cb->TM (rotMatrix, 0);
		return;
	}

	switch (selLevel) {
	case EP_SL_OBJECT:
		break;

	case EP_SL_VERTEX:
		if (ip->GetCommandMode()->ID()==CID_SUBOBJMOVE) {
			if (!mm.GetFlag (MN_MESH_NO_BAD_VERTS)) mm.EliminateBadVerts ();
			Matrix3 otm = node->GetObjectTM(t);
			for (int i=0; i<mm.numv; i++) {
				if (!mm.v[i].FlagMatch (MN_SEL|MN_DEAD, MN_SEL)) continue;
				mm.GetVertexSpace (i, tm);
				tm = tm * otm;
				tm.SetTrans(mm.v[i].p*otm);
				cb->TM(tm,i);
			}
		} else {
			for (int i=0; i<mm.numv; i++) if (mm.v[i].FlagMatch (MN_SEL|MN_DEAD, MN_SEL)) break;
			if (i >= mm.numv) return;
			Point3 norm(0,0,0), *nptr=TempData()->VertexNormals ()->Addr(0);
			Point3 cent(0,0,0);
			int ct=0;

			// Compute average face normal
			for (; i<mm.numv; i++) {
				if (!mm.v[i].FlagMatch (MN_SEL|MN_DEAD, MN_SEL)) continue;
				cent += mm.v[i].p;
				norm += nptr[i];
				ct++;
			}
			cent /= float(ct);
			norm = Normalize (norm/float(ct));

			cent = cent * otm;
			norm = Normalize(VectorTransform(otm,norm));
			Matrix3 mat;
			MatrixFromNormal(norm,mat);
			mat.SetTrans(cent);
			cb->TM(mat,0);
		}
		break;

	case EP_SL_EDGE:
	case EP_SL_BORDER:
		int i, ct;
		ct = TempData()->ClusterNormals(MNM_SL_EDGE)->Count();
		for (i=0; i<ct; i++) {
			tm = TempData()->ClusterTM (i) * otm;
			cb->TM(tm,i);
		}
		break;

	default:
		ct = TempData()->ClusterNormals(MNM_SL_FACE)->Count();
		for (i=0; i<ct; i++) {
			tm = TempData()->ClusterTM (i) * otm;
			cb->TM(tm,i);
		}
		break;
	}
}

void EditPolyObject::ShowEndResultChanged (BOOL showEndResult) {
	if ((!ip) || (editObj != this)) return;
	UpdateDisplayFlags ();
	UpdateCageCheckboxEnable ();
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

//--- Saving/Loading --------------------------------

#define VERTCOUNT_CHUNKID		0x3002
#define GENSELSET_ID_CHUNK     0x3003
#define GENSELSET_CHUNK     0x3004
#define EPOBJ_SELLEVEL_CHUNK 0x4038

IOResult EditPolyObject::Load(ILoad *iload) {
	ulong nb;
	IOResult res;
	bool selLevLoaded = FALSE;

	while (iload->PeekNextChunkID() == GENSELSET_ID_CHUNK) {
		int which;
		iload->OpenChunk();
		res = iload->Read(&which, sizeof(int), &nb);
		iload->CloseChunk ();
		if (res!=IO_OK) return res;

		if (iload->PeekNextChunkID() != GENSELSET_CHUNK) break;
		iload->OpenChunk ();
		res = selSet[which].Load(iload);
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}

	if (iload->PeekNextChunkID()==VERTCOUNT_CHUNKID) {		
		int ct;
		iload->OpenChunk();
		iload->Read(&ct,sizeof(ct),&nb);
		iload->CloseChunk();
		AllocContArray(ct);
		for (int i=0; i<ct; i++) SetPtCont(i, NULL);
	}

	if (iload->PeekNextChunkID() == EPOBJ_SELLEVEL_CHUNK) {
		iload->OpenChunk ();
		res = iload->Read (&selLevel, sizeof(DWORD), &nb);
		if (res != IO_OK) return res;
		iload->CloseChunk ();
		selLevLoaded = TRUE;
	}

	IOResult ret = PolyObject::Load (iload);
	if (!selLevLoaded) {
		switch (mm.selLevel) {
		case MNM_SL_OBJECT: selLevel = EP_SL_OBJECT; break;
		case MNM_SL_VERTEX: selLevel = EP_SL_VERTEX; break;
		case MNM_SL_EDGE: selLevel = EP_SL_EDGE; break;
		case MNM_SL_FACE: selLevel = EP_SL_FACE; break;
		}
	} else {
		// Otherwise, we consider the EPoly selection level to be "dominant".
		switch (selLevel)
		{
		case EP_SL_OBJECT: mm.selLevel = MNM_SL_OBJECT; break;
		case EP_SL_VERTEX: mm.selLevel = MNM_SL_VERTEX; break;
		case EP_SL_EDGE:
		case EP_SL_BORDER: mm.selLevel = MNM_SL_EDGE; break;
		case EP_SL_FACE:
		case EP_SL_ELEMENT: mm.selLevel = MNM_SL_FACE; break;
		}
	}

	// Updating these guys, who aren't loaded:
	mm.getVertexSel (vsel);
	mm.getEdgeSel (esel);
	mm.getFaceSel (fsel);
	return ret;
}

IOResult EditPolyObject::Save(ISave *isave) {	
	int ct = cont.Count();
	ulong nb;

	for (int j=0; j<3; j++) {
		if (!selSet[j].Count()) continue;
		isave->BeginChunk(GENSELSET_ID_CHUNK);
		isave->Write (&j, sizeof(j), &nb);
		isave->EndChunk ();
		isave->BeginChunk (GENSELSET_CHUNK);
		selSet[j].Save(isave);
		isave->EndChunk();
	}
	
	if (ct) {
		isave->BeginChunk(VERTCOUNT_CHUNKID);
		isave->Write(&ct,sizeof(ct),&nb);
		isave->EndChunk();
	}

	isave->BeginChunk (EPOBJ_SELLEVEL_CHUNK);
	isave->Write (&selLevel, sizeof(DWORD), &nb);
	isave->EndChunk ();

	return PolyObject::Save(isave);
}

void EditPolyObject::ActivateSubSelSet(TSTR &setName) {
	if (selLevel == EP_SL_OBJECT) return;

	BitArray *sset;
	int nsl = namedSetLevel[selLevel];
	sset = GetSelSet(nsl).GetSet(setName);
	if (sset==NULL) return;
	theHold.Begin ();
	SetSel (nsl, *sset, this, ip->GetTime());
	theHold.Accept (GetString (IDS_SELECT));
	LocalDataChanged ();
	RefreshScreen ();
}

void EditPolyObject::NewSetFromCurSel(TSTR &setName) {
	if (selLevel == EP_SL_OBJECT) return;
	BitArray *sset;
	int nsl = namedSetLevel[selLevel];
	sset = GetSelSet(nsl).GetSet(setName);
	if (sset) *sset = GetSel (nsl);
	else {
		BitArray sel = GetSel(nsl);
		GetSelSet(nsl).AppendSet (sel, 0, setName);
	}
}

void EditPolyObject::RemoveSubSelSet(TSTR &setName) {
	GenericNamedSelSetList &set = GetSelSet(namedSetLevel[selLevel]);
	BitArray *ssel = set.GetSet(setName);
	if (ssel) {
		if (theHold.Holding()) theHold.Put (new EPDeleteSetRestore (setName, &set, this));
		set.RemoveSet (setName);
	}
	ip->ClearCurNamedSelSet();
}

void EditPolyObject::SetupNamedSelDropDown() {
	// Setup named selection sets
	if (selLevel == EP_SL_OBJECT) return;
	GenericNamedSelSetList &set = GetSelSet(namedSetLevel[selLevel]);
	ip->ClearSubObjectNamedSelSets();
	for (int i=0; i<set.Count(); i++) ip->AppendSubObjectNamedSelSet(*(set.names[i]));
}

void EditPolyObject::UpdateNamedSelDropDown () {
	if (!ip) return;
	DWORD nsl = namedSetLevel[selLevel];
	GenericNamedSelSetList & ns = GetSelSet(nsl);
	for (int i=0; i<ns.Count(); i++) {
		if (*(ns.sets[i]) == GetSel (nsl)) break;
	}
	if (i<ns.Count()) ip->SetCurNamedSelSet (*(ns.names[i]));
	else ip->ClearCurNamedSelSet ();
}

int EditPolyObject::NumNamedSelSets() {
	GenericNamedSelSetList &set = GetSelSet(namedSetLevel[selLevel]);
	return set.Count();
}

TSTR EditPolyObject::GetNamedSelSetName(int i) {
	GenericNamedSelSetList &set = GetSelSet(namedSetLevel[selLevel]);
	return *set.names[i];
}

void EditPolyObject::SetNamedSelSetName(int i,TSTR &newName) {
	GenericNamedSelSetList &set = GetSelSet(namedSetLevel[selLevel]);
	if (theHold.Holding()) theHold.Put(new EPSetNameRestore(i,&set,this));
	*set.names[i] = newName;
}

void EditPolyObject::NewSetByOperator (TSTR &newName,Tab<int> &sets,int op) {
	GenericNamedSelSetList &set = GetSelSet(namedSetLevel[selLevel]);
	BitArray bits = *set.sets[sets[0]];

	for (int i=1; i<sets.Count(); i++) {
		switch (op) {
		case NEWSET_MERGE:
			bits |= *set.sets[sets[i]];
			break;

		case NEWSET_INTERSECTION:
			bits &= *set.sets[sets[i]];
			break;

		case NEWSET_SUBTRACT:
			bits &= ~(*set.sets[sets[i]]);
			break;
		}
	}
	
	set.AppendSet(bits,0,newName);
	if (theHold.Holding()) theHold.Put(new AppendSetRestore(&set,this));
	if (!bits.NumberSet()) RemoveSubSelSet(newName);
}

void EditPolyObject::EpfnNamedSelectionCopy (TSTR setName) {
	GenericNamedSelSetList & selSet = GetSelSet(namedSetLevel[selLevel]);
	int i;
	for (i=0; i<selSet.names.Count(); i++) {
		if (*selSet.names[i] == setName) break;
	}
	if (i>=selSet.names.Count()) return;
	NamedSelectionCopy (i);
}

void EditPolyObject::NamedSelectionCopy(int index) {
	if (selLevel == EP_SL_OBJECT) return;
	if (index<0) return;
	int nsl = namedSetLevel[selLevel];
	GenericNamedSelSetList & setList = GetSelSet(nsl);
	MeshNamedSelClip *clip = new MeshNamedSelClip(*(setList.names[index]));
	BitArray *bits = new BitArray(*setList.sets[index]);
	clip->sets.Append(1,&bits);
	SetMeshNamedSelClip (clip, namedClipLevel[selLevel]);

	// Enable the paste button
	HWND hGeom = GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but;
		but = GetICustButton(GetDlgItem(hGeom, IDC_PASTE_NS));
		but->Enable();
		ReleaseICustButton(but);
	}
}

void EditPolyObject::EpfnNamedSelectionPaste(bool useDlgToRename) {
	if (selLevel==EP_SL_OBJECT) return;
	int nsl = namedSetLevel[selLevel];
	MeshNamedSelClip *clip = GetMeshNamedSelClip(namedClipLevel[selLevel]);
	if (!clip) return;
	TSTR name = clip->name;
	if (!GetUniqueSetName(name,useDlgToRename)) return;

	theHold.Begin ();
	GenericNamedSelSetList & setList = GetSelSet(nsl);
	setList.AppendSet (*clip->sets[0], 0, name);
	if (theHold.Holding()) theHold.Put(new AppendSetRestore(&setList,this));
	ActivateSubSelSet(name);
	theHold.Accept (GetString (IDS_PASTE_NS));
	ip->SetCurNamedSelSet(name);
	SetupNamedSelDropDown();
}

static INT_PTR CALLBACK PickSetNameDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name;
	ICustEdit *edit;
	TCHAR buf[256];

	switch (msg) {
	case WM_INITDIALOG:
		name = (TSTR*)lParam;
		edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
		edit->SetText(*name);
		ReleaseICustEdit(edit);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			edit =GetICustEdit(GetDlgItem(hWnd,IDC_SET_NAME));
			edit->GetText(buf,256);
			*name = TSTR(buf);
			ReleaseICustEdit(edit);
			EndDialog(hWnd,1);
			break;

		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

BOOL EditPolyObject::GetUniqueSetName(TSTR &name, bool useDlg) {
	TSTR startname = name;
	TCHAR buffer[80];
	for (int loopCount=0; loopCount<100; loopCount++) {
		GenericNamedSelSetList & setList = selSet[namedSetLevel[selLevel]];

		BOOL unique = TRUE;
		for (int i=0; i<setList.Count(); i++) {
			if (name==*setList.names[i]) {
				unique = FALSE;
				break;
			}
		}
		if (unique) break;

		if (useDlg) {
			if (!ip) return FALSE;
			if (!DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NAMEDSET_PASTE),
				ip->GetMAXHWnd(), PickSetNameDlgProc, (LPARAM)&name)) return FALSE;
		} else {
			sprintf (buffer, "%s%d", startname, loopCount);
			name = TSTR(buffer);
		}
	}
	return TRUE;
}

static INT_PTR CALLBACK PickSetDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:	{
		GenericNamedSelSetList *setList = (GenericNamedSelSetList *)lParam;
		for (int i=0; i<setList->Count(); i++) {
			int pos  = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_ADDSTRING,0,
				(LPARAM)(TCHAR*)*(setList->names[i]));
			SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_SETITEMDATA,pos,i);
		}
		break;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NS_LIST:
			if (HIWORD(wParam)!=LBN_DBLCLK) break;
			// fall through

		case IDOK:
			int sel;
			sel = SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETCURSEL,0,0);
			if (sel!=LB_ERR) {
				int res =SendDlgItemMessage(hWnd,IDC_NS_LIST,LB_GETITEMDATA,sel,0);
				EndDialog(hWnd,res);
				break;
			}
			// fall through

		case IDCANCEL:
			EndDialog(hWnd,-1);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

int EditPolyObject::SelectNamedSet() {
	GenericNamedSelSetList & setList = selSet[namedSetLevel[selLevel]];
	return DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NAMEDSET_SEL),
		ip->GetMAXHWnd(), PickSetDlgProc, (LPARAM)&setList);
}

class NamedSetSizeIncrease : public RestoreObj {
	EditPolyObject *mpEditPoly;
	int nsl;
	int oldsize, increase;

public:
	NamedSetSizeIncrease () : mpEditPoly(NULL) { }
	NamedSetSizeIncrease (EditPolyObject *e, int n, int old, int inc) : mpEditPoly(e), nsl(n), oldsize(old), increase(inc) { }
	void Restore (int isUndo) { mpEditPoly->GetSelSet(nsl).SetSize (oldsize); }
	void Redo () { mpEditPoly->GetSelSet(nsl).SetSize (oldsize+increase); }
	int GetSize () { return 3*sizeof(int) + sizeof (void *); }
	TSTR Description () { return _T("Named Selection Set Size Increase"); }
};

void EditPolyObject::IncreaseNamedSetSize (int nsl, int oldsize, int increase) {
	if (increase == 0) return;
	if (GetSelSet(nsl).Count() == 0) return;
	if (theHold.Holding())
		theHold.Put (new NamedSetSizeIncrease (this, nsl, oldsize, increase));
	GetSelSet(nsl).SetSize (oldsize + increase);
}

class NamedSetDelete : public RestoreObj {
	EditPolyObject *mpEditPoly;
	int nsl;
	Tab<BitArray *> oldSets;
	BitArray del;

public:
	NamedSetDelete (EditPolyObject *e, int n, BitArray &d);
	~NamedSetDelete ();
	void Restore (int isUndo);
	void Redo () { mpEditPoly->GetSelSet(nsl).DeleteSetElements (del, (nsl==NS_EDGE) ? 3 : 1); }
	int GetSize () { return 3*sizeof(int) + sizeof (void *); }
	TSTR Description () { return _T("Named Selection Set Subset Deletion"); }
};

NamedSetDelete::NamedSetDelete (EditPolyObject *e, int n, BitArray &d) : mpEditPoly(e), nsl(n), del(d) {
	oldSets.SetCount (mpEditPoly->GetSelSet(nsl).Count());
	for (int i=0; i<mpEditPoly->GetSelSet(nsl).Count(); i++) {
		oldSets[i] = new BitArray;
		(*oldSets[i]) = (*(mpEditPoly->GetSelSet(nsl).sets[i]));
	}
}

NamedSetDelete::~NamedSetDelete () {
	for (int i=0; i<oldSets.Count(); i++) delete oldSets[i];
}

void NamedSetDelete::Restore (int isUndo) {
	int i, max = oldSets.Count();
	if (mpEditPoly->GetSelSet(nsl).Count() < max) max = mpEditPoly->GetSelSet(nsl).Count();
	for (i=0; i<max; i++) *(mpEditPoly->GetSelSet(nsl).sets[i]) = *(oldSets[i]);
}

void EditPolyObject::DeleteNamedSetArray (int nsl, BitArray &del) {
	if (del.NumberSet() == 0) return;
	if (GetSelSet(nsl).Count() == 0) return;
	GetSelSet(nsl).Alphabetize ();
	if (theHold.Holding()) 
		theHold.Put (new NamedSetDelete (this, nsl, del));
	GetSelSet(nsl).DeleteSetElements (del, (nsl==NS_EDGE) ? 3 : 1);
}

BitArray *EditPolyObject::GetNamedSelSet (int nsl, int setIndex)
{
	if ((nsl<0) || (nsl>2)) return NULL;
	if ((setIndex<0) || (setIndex>=selSet[nsl].Count())) return NULL;
	return selSet[nsl].sets[setIndex];
}

void EditPolyObject::SetNamedSelSet (int nsl, int setIndex, BitArray *newSet)
{
	if ((nsl<0) || (nsl>2)) return;
	if ((setIndex<0) || (setIndex>selSet[nsl].Count())) return;
	if (theHold.Holding ()) theHold.Put (new NamedSetChangeRestore (nsl, setIndex, this));
	*(selSet[nsl].sets[setIndex]) = *(newSet);
}

void EditPolyObject::CreateContArray() {
	if (cont.Count()) return;
	AllocContArray (mm.numv);
	for (int i=0; i<cont.Count(); i++) SetPtCont(i, NULL);
}

void EditPolyObject::SynchContArray(int nv) {
	int i, cct = cont.Count();
	if (!cct) return;
	if (cct == nv) return;
	if (masterCont) masterCont->SetNumSubControllers(nv, TRUE);
	if (cct>nv) {
		cont.Delete (nv, cct-nv);
		return;
	}
	cont.Resize (nv);
	Control *dummy=NULL;
	for (i=cct; i<nv; i++) cont.Append (1, &dummy);
}

void EditPolyObject::AllocContArray(int count) {
	cont.SetCount (count);
	if (masterCont) masterCont->SetNumSubControllers (count);
}

void EditPolyObject::ReplaceContArray(Tab<Control *> &nc) {
	AllocContArray (nc.Count());
	for(int i=0; i<nc.Count(); i++) SetPtCont (i, nc[i]);
}

RefTargetHandle EditPolyObject::GetReference(int i) {
	if (i == EPOLY_PBLOCK) return pblock;
	if (i == EPOLY_MASTER_CONTROL_REF) return masterCont;
	if (i >= (cont.Count() + EPOLY_VERT_BASE_REF)) return NULL;
	return cont[i - EPOLY_VERT_BASE_REF];
}

void EditPolyObject::SetReference(int i, RefTargetHandle rtarg) {
	if (i == EPOLY_PBLOCK) {
		pblock = (IParamBlock2 *) rtarg;
		return;
	}

	if(i == EPOLY_MASTER_CONTROL_REF) {
		masterCont = (MasterPointControl*)rtarg;
		if (masterCont) masterCont->SetNumSubControllers(cont.Count());
		return;
	}

	if(i < (mm.numv + EPOLY_VERT_BASE_REF)) {
		if (!cont.Count()) CreateContArray();
		SetPtCont(i - EPOLY_VERT_BASE_REF, (Control*)rtarg); 
	}
}

TSTR EditPolyObject::SubAnimName(int i) {
	if (i == EPOLY_PBLOCK) return GetString (IDS_PBLOCK);
	if (i == EPOLY_MASTER_CONTROL_REF) return GetString(IDS_MASTERCONT);
	TSTR buf;
	if(i < (cont.Count() + EPOLY_VERT_BASE_REF))
		buf.printf(GetString(IDS_WHICH_POINT), i+1-EPOLY_VERT_BASE_REF);
	return buf;
}

void EditPolyObject::DeletePointConts(BitArray &set) {
	if (!cont.Count()) return;

	BOOL deleted = FALSE;
	Tab<Control*> nc;
	nc.SetCount(cont.Count());
	int ix=0;
	for (int i=0; i<cont.Count(); i++) {
		if (!set[i]) nc[ix++] = cont[i];
		else deleted = TRUE;
	}
	nc.SetCount(ix);
	nc.Shrink();
	ReplaceContArray(nc);
}

void EditPolyObject::PlugControllersSel(TimeValue t,BitArray &set) {
	BOOL res = FALSE;
	SetKeyModeInterface *ski = GetSetKeyModeInterface(GetCOREInterface());
	if( !ski || !ski->TestSKFlag(SETKEY_SETTING_KEYS) ) {
		if(!AreWeKeying(t)) { return; }
	}
	for (int i=0; i<mm.numv; i++) if (set[i] && PlugControl(t,i)) res = TRUE;
	if (res) NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

BOOL EditPolyObject::PlugControl(TimeValue t,int i) {
	if (!AreWeKeying(t)) return FALSE;
	if (!cont.Count()) CreateContArray();
	
#ifndef WEBVERSION // fix for bug #415902 - not a problem in luna
	if (cont[i]) return FALSE;
#else  // WEBVERSION
	if ( i >= cont.Count() ) { 
		return FALSE;
	}
#endif // WEBVERSION



	MakeRefByID (FOREVER, i + EPOLY_VERT_BASE_REF, NewDefaultPoint3Controller());
	SuspendAnimate(); 
	AnimateOff();  
	SuspendSetKeyMode(); // AF (5/13/03)
	theHold.Suspend ();
	if( tempMove) cont[i]->SetValue (t, tempMove->init[i]);
	else
		cont[i]->SetValue (t, &mm.v[i].p);
	ResumeSetKeyMode();
	cont[i]->SetValue (t, &mm.v[i].p);
	theHold.Resume ();

	ResumeAnimate (); 
	masterCont->SetSubController (i, cont[i]);
	return TRUE;
}

void EditPolyObject::SetPtCont (int i, Control *c) {
	cont[i]=c;
	if (masterCont) masterCont->SetSubController(i, c);
}

void EditPolyObject::InvalidateDistanceCache () {
	InvalidateSoftSelectionCache ();
	if (!tempData) return;
	tempData->InvalidateDistances ();
}

void EditPolyObject::InvalidateSoftSelectionCache () {
	arValid = NEVER;
	if (!tempData) return;
	tempData->InvalidateSoftSelection ();
	int affectRegion;
	pblock->GetValue (ep_ss_use, TimeValue(0), affectRegion, FOREVER);
	if (affectRegion) NotifyDependents (FOREVER, PART_SELECT, REFMSG_CHANGE);
}

void EditPolyObject::RescaleWorldUnits (float f) {
	PolyObject::RescaleWorldUnits (f);
	LocalDataChanged (PART_GEOM);
}

void EditPolyObject::UpdateGeometry (TimeValue time) {
	arValid = NEVER;
	subdivValid = NEVER;
	mPreviewValid = false;
	localGeomValid = FOREVER;
	for (int i=0; i<cont.Count(); i++) {
		if (cont[i]) cont[i]->GetValue (time, mm.P(i), localGeomValid);
	}
	mm.InvalidateGeomCache ();
	mm.freeRVerts ();
	InvalidateTempData (PART_GEOM);
}

void EditPolyObject::UpdateSoftSelection (TimeValue time) {
	if (!localGeomValid.InInterval (time)) UpdateGeometry (time);

	arValid = localGeomValid;

	int useSoftSel, useEdgeDist, edgeIts, affectBack;
	float falloff, pinch, bubble;
	pblock->GetValue (ep_ss_use, time, useSoftSel, arValid);
	if (!useSoftSel) {
		mm.freeVSelectionWeights ();
		return;
	}

	pblock->GetValue (ep_ss_edist_use, time, useEdgeDist, arValid);
	if (useEdgeDist) {
		pblock->GetValue (ep_ss_edist, time, edgeIts, arValid);
	}
	pblock->GetValue (ep_ss_affect_back, time, affectBack, arValid);
	pblock->GetValue (ep_ss_falloff, time, falloff, arValid);
	pblock->GetValue (ep_ss_pinch, time, pinch, arValid);
	pblock->GetValue (ep_ss_bubble, time, bubble, arValid);

	mm.SupportVSelectionWeights ();
	if (!mm.numv) return;

	float *vsw = mm.getVSelectionWeights();
	TempData()->InvalidateSoftSelection ();	// Otherwise we'll just use the last cached value.
	Tab<float> *myVSWTable = TempData()->VSWeight (useEdgeDist, edgeIts, !affectBack,
		falloff, pinch, bubble);
	float *myVSW = (myVSWTable && myVSWTable->Count()) ? myVSWTable->Addr(0) : NULL;
	memcpy (vsw, myVSW, mm.numv*sizeof(float));
}

class Epomop : public MeshOpProgress {
	EditPolyObject *mpEditPoly;
	bool testEscape;
	HCURSOR originalCursor;
public:
	Epomop () : mpEditPoly(NULL), testEscape(false) { }
	Epomop (EditPolyObject *e, int order) : mpEditPoly(e) { SuperInit (order); }
	~Epomop () { SuperFinish(); }
	void SuperInit (int order);	// My localized init
	void Init (int numSteps) {}
	BOOL Progress (int step);
	void SuperFinish ();
};

class EscapeChecker {
	DWORD mMaxProcessID;
public:
	EscapeChecker () { mMaxProcessID = 0; }
	void Setup ();
	bool Check ();
};

void EscapeChecker::Setup () {
	GetAsyncKeyState (VK_ESCAPE);	// Clear any previous presses.
	mMaxProcessID = GetCurrentProcessId();
}

bool EscapeChecker::Check () {
	if (!GetAsyncKeyState (VK_ESCAPE)) return false;
	DWORD processID=0;
	HWND hForeground = GetForegroundWindow ();
	if (hForeground) GetWindowThreadProcessId (hForeground, &processID);
	return (processID == mMaxProcessID) ? true : false;
}

static EscapeChecker theEscapeChecker;

void Epomop::SuperInit (int total) {
	if (mpEditPoly) mpEditPoly->ClearFlag (EPOLY_ABORTED);
	if (total<kMNNurmsHourglassLimit) { testEscape=false; return; }
	testEscape = true;
	originalCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
	theEscapeChecker.Setup ();
}

BOOL Epomop::Progress(int p) {
	if (!testEscape) return TRUE;
	if (theEscapeChecker.Check ()) {
		if (mpEditPoly) mpEditPoly->SetFlag(EPOLY_ABORTED);
		return FALSE;
	}
	else return TRUE;
}

void Epomop::SuperFinish () {
	if (!testEscape) return;
	if (originalCursor) SetCursor (originalCursor);
	else SetCursor (LoadCursor (NULL, IDC_ARROW));
}

void EditPolyObject::UpdateSubdivResult (TimeValue t) {
	BOOL useSubdiv=false;
	if (pblock) pblock->GetValue (ep_surf_subdivide, t, useSubdiv, FOREVER);
	if (!useSubdiv) {
		subdivValid.SetInfinite();
		return;
	}

	if (subdivValid.InInterval (t)) {
		subdivResult.selLevel = mm.selLevel;
		return;
	}

	MNMesh *subdivStarterMesh = &mm;
	if (EpPreviewOn()) subdivStarterMesh = &mPreviewMesh;

	// Check if we should be updating the subdiv result:
	bool inRender = GetFlag (EPOLY_IN_RENDER);
	bool forceUpdate = GetFlag (EPOLY_FORCE);
	int update;
	pblock->GetValue (ep_surf_update, t, update, FOREVER);
	if (!forceUpdate && ((update==2) || ((update==1) && !inRender))) return;

	if (!subdivStarterMesh->GetFlag (MN_MESH_FILLED_IN)) {
		// Shouldn't happen.
		DbgAssert(0);
		subdivStarterMesh->FillInMesh ();
	}

	subdivResult = *subdivStarterMesh;
	subdivValid = FOREVER;

	BOOL useRIter=false, useRThresh=false, sepSmooth, sepMat, smoothResult;
	int iter;
	float thresh;
	pblock->GetValue (ep_surf_subdiv_smooth, t, smoothResult, subdivValid);
#ifndef NO_OUTPUTRENDERER
	if (inRender) {
		pblock->GetValue (ep_surf_use_riter, t, useRIter, subdivValid);
		pblock->GetValue (ep_surf_use_rthresh, t, useRThresh, subdivValid);
	}
	if (useRIter) pblock->GetValue (ep_surf_riter, t, iter, subdivValid);
	else pblock->GetValue (ep_surf_iter, t, iter, subdivValid);
	if (useRThresh) pblock->GetValue (ep_surf_rthresh, t, thresh, subdivValid);
	else pblock->GetValue (ep_surf_thresh, t, thresh, subdivValid);
#else
	pblock->GetValue (ep_surf_iter, t, iter, subdivValid);
	pblock->GetValue (ep_surf_thresh, t, thresh, subdivValid);
#endif  // NO_OUTPUTRENDERER

	pblock->GetValue (ep_surf_sep_smooth, t, sepSmooth, subdivValid);
	pblock->GetValue (ep_surf_sep_mat, t, sepMat, subdivValid);

	ClearFlag (EPOLY_FORCE);

	// CAL-04/07/03: set the subdivision corner flag on all the vertices in the subdivResult.
	for (int i=0; i<subdivResult.numv; i++)
		if (!subdivResult.v[i].GetFlag(MN_DEAD))
			subdivResult.v[i].SetFlag(MN_VERT_SUBDIVISION_CORNER);
	// CAL-03/20/03: set the subdivision boundary flag on all the edges in the subdivResult.
	for (i=0; i<subdivResult.nume; i++)
		if (!subdivResult.e[i].GetFlag(MN_DEAD))
			subdivResult.e[i].SetFlag(MN_EDGE_SUBDIVISION_BOUNDARY);
	// CAL-03/20/03: clear/set the MNDISP_HIDE_SUBDIVISION_INTERIORS flag to show/hide interior edges
	if (pblock->GetInt(ep_surf_isoline_display, t))
		subdivResult.SetDispFlag(MNDISP_HIDE_SUBDIVISION_INTERIORS);
	else
		subdivResult.ClearDispFlag(MNDISP_HIDE_SUBDIVISION_INTERIORS);

	subdivResult.EliminateBadVerts ();	// don't trust NO_BAD_VERTS flag.
	subdivResult.SplitFacesUsingBothSidesOfEdge (0);
	subdivResult.SetMapSeamFlags ();
	subdivResult.FenceOneSidedEdges ();
	if (sepMat) subdivResult.FenceMaterials ();
	if (sepSmooth) subdivResult.FenceSmGroups ();

	DWORD subdivFlags = MN_SUBDIV_NEWMAP;
	int nstart = subdivResult.TargetVertsBySelection (MNM_SL_OBJECT);

	// Order-of-magnitude calculation:
	// We approximately multiply the number of active components by four each iteration.
	int order = nstart;
	for (i=0; i<iter; i++) order *= 4;
	Epomop epomop(this, order);

	int oldfnum = subdivResult.numf;
	for (i=0; i<iter; i++) {
		if (!epomop.Progress(i)) break;
		// Can following line be removed?
		subdivResult.OrderVerts ();
		if (thresh<1) subdivResult.DetargetVertsBySharpness (1.0f-thresh);
		if (!epomop.Progress(i)) break;

		subdivResult.CubicNURMS (&epomop, NULL, subdivFlags);
		subdivResult.CollapseDeadFaces ();
		// Can following line be removed?
		// Certainly it should go after FillInMesh below...  But are either needed?
		subdivResult.EliminateBadVerts();

		if (GetFlag (EPOLY_ABORTED)) break;

		if (!subdivResult.GetFlag (MN_MESH_FILLED_IN)) subdivResult.FillInMesh ();
		subdivResult.SetMapSeamFlags ();
		subdivResult.FenceOneSidedEdges ();
		if (sepMat) subdivResult.FenceMaterials ();
		if (sepSmooth) subdivResult.FenceSmGroups ();
	}

	if (GetFlag (EPOLY_ABORTED)) {
		subdivValid.SetInstant(t);
		if (!GetFlag (EPOLY_IN_RENDER)) pblock->SetValue (ep_surf_update, t, 2);	// Set to manual update.
	} else {
		// Apply smoothing:
		if (smoothResult) {
			subdivResult.ClearEFlags (MN_USER);
			subdivResult.PropegateComponentFlags (MNM_SL_EDGE, MN_USER,
				MNM_SL_EDGE, MN_EDGE_NOCROSS);
			float *ec;
			if (ec = subdivResult.edgeFloat (EDATA_CREASE)) {
				for (i=0; i<subdivResult.nume; i++) {
					if (ec[i]) subdivResult.e[i].SetFlag (MN_USER);
				}
			}
			subdivResult.SmoothByCreases (MN_USER);
		}
	}
}

void EditPolyObject::UpdateEverything (TimeValue time) {
	bool updated = false;
	if (!mm.GetFlag (MN_MESH_FILLED_IN)) {
		mm.FillInMesh ();
		updated = true;
	}
	if (!localGeomValid.InInterval(time)) {
		UpdateGeometry (time);
		updated = true;
		arValid.SetEmpty ();
	}
	if (!arValid.InInterval (time)) {
		UpdateSoftSelection (time);
		updated = true;
		// Need to update both of these if Soft Selection has changed:
		// (otherwise preview / subdivision result won't be correct.)
		EpPreviewInvalidate ();
		subdivValid.SetEmpty ();
	}
	if (mPreviewOn && !mPreviewValid) {
		UpdatePreviewMesh ();
		updated = true;
	}

	int useSubdiv;
	pblock->GetValue (ep_surf_subdivide, time, useSubdiv, FOREVER);
	if (useSubdiv && !subdivValid.InInterval(time)) {
		UpdateSubdivResult (time);
		updated = true;
	}

	if (!displacementSettingsValid.InInterval (time)) {
		SetDisplacementParams (time);
		updated = true;
	}

	if (!updated) return;

	if (useSubdiv) {
		geomValid = subdivValid & arValid & localGeomValid;
		topoValid = subdivValid;
		texmapValid = subdivValid;
		selectValid = subdivValid;
		vcolorValid = subdivValid;
	} else {
		geomValid = arValid & localGeomValid;
		topoValid   = FOREVER;
		texmapValid = FOREVER;
		selectValid = FOREVER;
		vcolorValid = FOREVER;
	}
}

ObjectState EditPolyObject::Eval(TimeValue time) {
	UpdateEverything (time);
	return ObjectState(this);
}

RefResult EditPolyObject::NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget,
										   PartID& partID, RefMessage message) {
	switch (message) {
	case REFMSG_CHANGE:
		if (hTarget == pblock) {
			// if this was caused by a NotifyDependents from pblock, LastNotifyParamID()
			// will contain ID to update, else it will be -1 => inval whole rollout
			int pid = pblock->LastNotifyParamID ();
			if (editObj==this) InvalidateDialogElement (pid);
			if ((pid >= ep_surf_subdivide) && (pid <= ep_surf_sep_mat))
				subdivValid.SetEmpty ();
			if (pid==ep_surf_subdiv_smooth) subdivValid.SetEmpty ();
			if (pid == -1) subdivValid.SetEmpty();
			switch (pid) {
			case ep_show_cage:
				UpdateDisplayFlags ();
				break;
			case ep_surf_subdivide:
				if (editObj == this) UpdateCageCheckboxEnable ();
				UpdateDisplayFlags ();
				break;
			case ep_surf_isoline_display:		// CAL-03/20/03:
				UpdateDisplayFlags ();
				break;
			case ep_vert_color_selby:
				if (editObj==this) InvalidateSurfaceUI();
				break;
			case ep_ss_use:
			case ep_ss_edist_use:
			case ep_ss_edist:
				InvalidateDistanceCache ();
				break;
			case ep_ss_affect_back:
			case ep_ss_falloff:
			case ep_ss_pinch:
			case ep_ss_bubble:
				InvalidateSoftSelectionCache ();
				break;
			case ep_surf_update:
			case ep_surf_use_riter:
			case ep_surf_use_rthresh:
				InvalidateSubdivisionUI ();
				break;
			case ep_sd_use:
			case ep_sd_split_mesh:
			case ep_sd_method:
			case ep_sd_tess_steps:
			case ep_sd_tess_edge:
			case ep_sd_tess_distance:
			case ep_sd_tess_angle:
			case ep_sd_view_dependent:
			case ep_asd_style:
			case ep_asd_min_iters:
			case ep_asd_max_iters:
			case ep_asd_max_tris:
				displacementSettingsValid.SetEmpty ();
				break;
		
			// These parameters could affect the preview mode:
			case ep_extrusion_type:
			case ep_bevel_type:
			case ep_face_extrude_height:
			case ep_vertex_extrude_height:
			case ep_edge_extrude_height:
			case ep_vertex_extrude_width:
			case ep_edge_extrude_width:
			case ep_bevel_height:
			case ep_bevel_outline:
			case ep_outline:
			case ep_inset:
			case ep_inset_type:
			case ep_vertex_chamfer:
			case ep_edge_chamfer:
			case ep_weld_threshold:
			case ep_edge_weld_threshold:
			case ep_ms_smoothness:
			case ep_ms_sep_smooth:
			case ep_ms_sep_mat:
			case ep_tess_type:
			case ep_tess_tension:
			case ep_connect_edge_segments:
			case ep_extrude_spline_node:
			case ep_extrude_spline_segments:
			case ep_extrude_spline_taper:
			case ep_extrude_spline_taper_curve:
			case ep_extrude_spline_twist:
			case ep_extrude_spline_rotation:
			case ep_extrude_spline_align:
			case ep_lift_angle:
			case ep_lift_edge:
			case ep_lift_segments:
			case ep_cut_start_level:
			case ep_cut_start_index:
			case ep_cut_start_coords:
			case ep_cut_end_coords:
			case ep_cut_normal:
			case ep_interactive_full:
				if (EpPreviewOn()) EpPreviewInvalidate ();
				break;
			}
			if (!killRefmsg.DistributeRefmsg())
				return REF_STOP;
		} else {
			// Must be one of our point controller references.
			localGeomValid.SetEmpty();
		}
		break;
	}
	return REF_SUCCEED;
}

BOOL EditPolyObject::AssignController (Animatable *control, int subAnim) {
	ReplaceReference (subAnim, (Control*)control);
	if (subAnim==EPOLY_MASTER_CONTROL_REF) {
		int n = cont.Count();
		masterCont->SetNumSubControllers(n);
		for (int i=0; i<n; i++) if (cont[i]) masterCont->SetSubController(i,cont[i]);
	}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	return TRUE;
}

#ifdef _SUBMTLASSIGNMENT

// CCJ 1/19/98
// Return the sub object material assignment interface
// This is used by the node when assigning materials.
// If an open face selection mode is active, the material
// will be assigned to the selected faces only.
// A multi/sub-object material is created and the material
// is assigned to the matierial ID created for the selected
// faces.
void* EditPolyObject::GetInterface(ULONG id) {
	switch (id) {
	case I_SUBMTLAPI: return (ISubMtlAPI*)this;
	case I_MESHSELECT: return (IMeshSelect*)this;
	case I_MESHSELECTDATA: return (IMeshSelectData*)this;
	}
	//JH 3/8/99
	//This previously called Object"s implementation
	return Object::GetInterface(id);
}

// Return a material ID that is currently not used by the object.
// If the current face selection share once single MtlID that is not
// used by any other faces, you should use it.
MtlID EditPolyObject::GetNextAvailMtlID(ModContext* mc) {
	int mtlID = GetSelFaceUniqueMtlID(mc);

	if (mtlID == -1) {
		int i;
		BitArray b;
		mtlID = mm.numf;
		b.SetSize (mm.numf, FALSE);
		b.ClearAll ();
		for (i=0; i<mm.numf; i++) {
			int mid = mm.f[i].material;
			if (mid < mm.numf) b.Set(mid);
		}

		for (i=0; i<mm.numf; i++) {
			if (!b[i]) {
				mtlID = i;
				break;
			}
		}
	}

	return (MtlID)mtlID;
}

// Indicate if you are active in the modifier panel and have an 
// active face selection
BOOL EditPolyObject::HasFaceSelection(ModContext* mc) {
	// Are we the edited object?
	if (ip == NULL)  return FALSE;
	// Is Face selection active?
	if (selLevel < EP_SL_FACE) return FALSE;
	// Do we have any selected faces?
	for (int i=0; i<mm.numf; i++) {
		if (mm.f[i].FlagMatch (MN_SEL|MN_DEAD, MN_SEL)) break;
	}
	return (i<mm.numf);
}

// Set the selected faces to the specified material ID.
// If bResetUnsel is TRUE, then you should set the remaining
// faces material ID's to 0
void EditPolyObject::SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel) {
	if (theHold.Holding() && !TestAFlag(A_HELD))
		theHold.Put(new MtlIDRestore(this));

	for (int i=0; i<mm.numf; i++) {
		if (mm.f[i].GetFlag (MN_DEAD)) continue;
		if (mm.f[i].GetFlag (MN_SEL)) mm.f[i].material = id;
		else if (bResetUnsel) mm.f[i].material = 0;
	}

	LocalDataChanged (PART_TOPO);
}

// Return the material ID of the selected face(s).
// If multiple faces are selected they should all have the same MtlID -
// otherwise you should return -1.
// If faces other than the selected share the same material ID, then 
// you should return -1.
int EditPolyObject::GetSelFaceUniqueMtlID(ModContext* mc) {
	int	i;
	int	mtlID;

	mtlID = GetSelFaceAnyMtlID(mc);

	if (mtlID == -1) return mtlID;
	for (i=0; i<mm.numf; i++) {
		if (mm.f[i].GetFlag (MN_SEL|MN_DEAD)) continue;
		if (mm.f[i].material != mtlID) continue;
		mtlID = -1;
		break;
	}

	return mtlID;
}

// Return the material ID of the selected face(s).
// If multiple faces are selected they should all have the same MtlID,
// otherwise you should return -1.
int EditPolyObject::GetSelFaceAnyMtlID(ModContext* mc) {
	int				mtlID = -1;
	BOOL			bGotFirst = FALSE;
	int				i;

	for (i=0; i<mm.numf; i++) {
		if (!mm.f[i].FlagMatch (MN_SEL|MN_DEAD, MN_SEL)) continue;
		if (bGotFirst) {
			if (mtlID != mm.f[i].material) {
				mtlID = -1;
				break;
			}
		} else {
			mtlID = mm.f[i].material;
			bGotFirst = TRUE;
		}
	}

	return mtlID;
}

// Return the highest MtlID used by the object.
int EditPolyObject::GetMaxMtlID(ModContext* mc) {
	MtlID mtlID = 0;
	for (int i=0; i<mm.numf; i++) mtlID = max(mtlID, mm.f[i].material);
	return mtlID;
}

#endif // _SUBMTLASSIGNMENT

BaseInterface *EditPolyObject::GetInterface (Interface_ID id) {
	if (id == EPOLY_INTERFACE) return (EPoly *)this;
	return FPMixinInterface::GetInterface(id);
}

void EditPolyObject::LocalDataChanged () {
	LocalDataChanged (PART_SELECT);
}

void EditPolyObject::SetSelLevel (DWORD lev) {
	DWORD sl;
	switch (lev) {
	case IMESHSEL_OBJECT:
		sl = EP_SL_OBJECT;
		break;
	case IMESHSEL_VERTEX:
		sl = EP_SL_VERTEX;
		break;
	case IMESHSEL_EDGE:
		if (meshSelLevel[selLevel] == MNM_SL_EDGE) sl = selLevel;
		else sl = EP_SL_EDGE;
		break;
	case IMESHSEL_FACE:
		if (meshSelLevel[selLevel] == MNM_SL_FACE) sl = selLevel;
		else sl = EP_SL_FACE;
		break;
	}
	if (ip) ip->SetSubObjectLevel (sl);
	else InvalidateTempData (PART_SUBSEL_TYPE);
}

DWORD EditPolyObject::GetSelLevel () {
	switch (selLevel) {
	case EP_SL_OBJECT:
		return IMESHSEL_OBJECT;
	case EP_SL_VERTEX:
		return IMESHSEL_VERTEX;
	case EP_SL_EDGE:
	case EP_SL_BORDER:
		return IMESHSEL_EDGE;
	case EP_SL_FACE:
	case EP_SL_ELEMENT:
		return IMESHSEL_FACE;
	}
	return IMESHSEL_OBJECT;
}

BitArray EditPolyObject::GetSel (int nsl) {
	BitArray ret;
	switch (nsl) {
	case NS_VERTEX:
		mm.getVertexSel (ret);
		break;
	case NS_EDGE:
		mm.getEdgeSel (ret);
		break;
	case NS_FACE:
		mm.getFaceSel (ret);
		break;
	}
	return ret;
}

void EditPolyObject::SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	if (ip) ip->ClearCurNamedSelSet();
	if (theHold.Holding()) theHold.Put (new ComponentFlagRestore (this, MNM_SL_VERTEX));
	mm.VertexSelect (set);
	if (set.GetSize () < mm.numv) {
		// Clear selection beyond set's boundaries:
		for (int i=set.GetSize(); i<mm.numv; i++) mm.v[i].ClearFlag (MN_SEL);
	}

	mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_SEL, MNM_SL_VERTEX, MN_HIDDEN, FALSE, FALSE);
	mm.getVertexSel (vsel);

	// Any change in selection should invalidate the preview.
	if (EpPreviewOn()) EpPreviewInvalidate ();
}

void EditPolyObject::SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	if (ip) ip->ClearCurNamedSelSet();
	if (theHold.Holding()) theHold.Put (new ComponentFlagRestore (this, MNM_SL_FACE));
	mm.FaceSelect (set);
	if (set.GetSize () < mm.numf) {
		// Clear selection beyond set's boundaries:
		for (int i=set.GetSize(); i<mm.numf; i++) mm.f[i].ClearFlag (MN_SEL);
	}

	mm.PropegateComponentFlags (MNM_SL_FACE, MN_SEL, MNM_SL_FACE, MN_HIDDEN, FALSE, FALSE);
	mm.getFaceSel (fsel);

	// Any change in selection should invalidate the preview.
	if (EpPreviewOn()) EpPreviewInvalidate ();
}

void EditPolyObject::SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t) {
	if (ip) ip->ClearCurNamedSelSet();
	if (theHold.Holding()) theHold.Put (new ComponentFlagRestore (this, MNM_SL_EDGE));
	mm.EdgeSelect (set);
	if (set.GetSize () < mm.nume) {
		// Clear selection beyond set's boundaries:
		for (int i=set.GetSize(); i<mm.nume; i++) mm.e[i].ClearFlag (MN_SEL);
	}

	// Clear selection on edges on hidden faces:
	mm.PropegateComponentFlags (MNM_SL_EDGE, MN_SEL, MNM_SL_FACE, MN_HIDDEN, true, false);
	mm.getEdgeSel (esel);

	// Any change in selection should invalidate the preview.
	if (EpPreviewOn()) EpPreviewInvalidate ();
}

void EditPolyObject::EpSetVertexFlags (BitArray &vset, DWORD flags, DWORD fmask, bool undoable) {
	if ((flags & MN_SEL) && ip) ip->ClearCurNamedSelSet();
	if (undoable && theHold.Holding()) theHold.Put (new ComponentFlagRestore (this, MNM_SL_VERTEX));

	fmask = fmask | flags;
	int i, max = (mm.numv > vset.GetSize()) ? vset.GetSize() : mm.numv;
	for (i=0; i<max; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (!vset[i]) continue;
		mm.v[i].ClearFlag (fmask);
		mm.v[i].SetFlag (flags);
	}
	if (flags & (MN_HIDDEN|MN_SEL)) {
		mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_SEL, MNM_SL_VERTEX, MN_HIDDEN, FALSE, FALSE);
		DWORD parts = PART_DISPLAY|PART_SELECT;
		if (flags & MN_HIDDEN) parts |= PART_GEOM;	// for bounding box change
		LocalDataChanged (parts);
	}
}

void EditPolyObject::EpSetEdgeFlags (BitArray &eset, DWORD flags, DWORD fmask, bool undoable) {
	if ((flags & MN_SEL) && ip) ip->ClearCurNamedSelSet();
	if (undoable && theHold.Holding()) theHold.Put (new ComponentFlagRestore (this, MNM_SL_EDGE));

	fmask = fmask | flags;
	int i, max = (mm.nume > eset.GetSize()) ? eset.GetSize() : mm.nume;
	for (i=0; i<max; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if (!eset[i]) continue;
		mm.e[i].ClearFlag (fmask);
		mm.e[i].SetFlag (flags);
	}
	if (flags & MN_SEL) LocalDataChanged (PART_SELECT|PART_DISPLAY);
}

void EditPolyObject::EpSetFaceFlags (BitArray &fset, DWORD flags, DWORD fmask, bool undoable) {
	if ((flags & MN_SEL) && ip) ip->ClearCurNamedSelSet();
	if (undoable && theHold.Holding()) theHold.Put (new ComponentFlagRestore (this, MNM_SL_FACE));

	fmask = fmask | flags;
	int i, max = (mm.numf > fset.GetSize()) ? fset.GetSize() : mm.numf;
	for (i=0; i<max; i++) {
		if (mm.f[i].GetFlag (MN_DEAD)) continue;
		if (!fset[i]) continue;
		mm.f[i].ClearFlag (fmask);
		mm.f[i].SetFlag (flags);
	}
	if (flags & (MN_HIDDEN|MN_SEL)) {
		mm.PropegateComponentFlags (MNM_SL_FACE, MN_SEL, MNM_SL_FACE, MN_HIDDEN, FALSE, FALSE);
		DWORD parts = PART_DISPLAY|PART_SELECT;
		if (flags & MN_HIDDEN) parts |= PART_GEOM;	// for bounding box change
		LocalDataChanged (parts);
	}
}

void EditPolyObject::SetSel (int nsl, BitArray & set, IMeshSelect *imod, TimeValue t) {
	switch (nsl) {
	case NS_VERTEX:
		SetVertSel (set, imod, t);
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Vertex"), mr_bitarray, &vsel);
		break;
	case NS_EDGE:
		SetEdgeSel (set, imod, t);
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Edge"), mr_bitarray, &esel);
		break;
	case NS_FACE:
		SetFaceSel (set, imod, t);
		macroRecorder->FunctionCall(_T("$.EditablePoly.SetSelection"), 2, 0,
			mr_name, _T("Face"), mr_bitarray, &fsel);
		break;
	}
}

// New variant of above for Maxscript.
BitArray *EditPolyObject::EpfnGetSelection (int msl) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	switch (msl) {
	case MNM_SL_VERTEX:
		return &(this->vsel);
	case MNM_SL_EDGE:
		return &(this->esel);
	case MNM_SL_FACE:
		return &(this->fsel);
	}
	return NULL;
}

void EditPolyObject::EpfnSetSelection (int msl, BitArray *newSel) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (msl == MNM_SL_OBJECT) return;

	switch (msl) {
	case MNM_SL_VERTEX:
		SetVertSel (*newSel, this, TimeValue(0));
		break;
	case MNM_SL_EDGE:
		SetEdgeSel (*newSel, this, TimeValue(0));
		break;
	case MNM_SL_FACE:
		SetFaceSel (*newSel, this, TimeValue(0));
		break;
	}
	LocalDataChanged (PART_SELECT);
	RefreshScreen ();
}

// DeltaConstrainer is used in conjunction with the ep_constrain_type option.
// The reason we make it a static class is to prevent excessive reallocation during
// drag operations.

// Note that it's only ok to have this static because the user can operate on only
// one EPoly at a time - if they could do two, like Edit Mesh, then there might be
// conflicts and we'd need to have one instance per EditPolyObject.
class DeltaConstrainer {
private:
	Tab<Point3> *mptConstrainedDelta;

public:
	DeltaConstrainer () : mptConstrainedDelta (NULL) {}
	~DeltaConstrainer () { Free(); }
	void Allocate ();
	void Free ();
	Tab<Point3> *ConstrainToEdges (MNMesh & mesh, Tab<Point3> & tDelta);
	Tab<Point3> *ConstrainToFaces (MNMesh & mesh, Tab<Point3> & tDelta);
};

static DeltaConstrainer theDeltaConstrainer;

void DeltaConstrainer::Allocate() {
	if (mptConstrainedDelta == NULL)
		mptConstrainedDelta = new Tab<Point3>;
}

void DeltaConstrainer::Free () {
	if (mptConstrainedDelta)
		delete mptConstrainedDelta;
	mptConstrainedDelta = NULL;
}

Tab<Point3> *DeltaConstrainer::ConstrainToEdges (MNMesh & mesh, Tab<Point3> & tDelta) {
	Allocate ();
	if (!mesh.GetFlag (MN_MESH_FILLED_IN)) return mptConstrainedDelta;
	if (!mesh.vedg) return mptConstrainedDelta;
	if (!tDelta.Count()) return mptConstrainedDelta;

	mptConstrainedDelta->SetCount (tDelta.Count());
	Point3 *pDelta = tDelta.Addr(0);
	Point3 *pConstr = mptConstrainedDelta->Addr(0);

	for (int i=0; i<tDelta.Count(); i++) {
		pConstr[i] = pDelta[i];	// Initialize.
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;	// Dead - who cares how we move.
		if (pDelta[i]==Point3(0,0,0)) continue;	// nothing to constrain

		// We want this vertex to move along any unflagged edge that lies in the
		// right general direction.
		int vct = mesh.vedg[i].Count();
		if (!vct) continue;	// no edges - move as if unconstrained.

		// Examine where we're being asked to go:
		float deltaLength = Length (pDelta[i]);
		if (deltaLength<.0001f) continue;	// Delta tiny - move as if unconstrained.
		Point3 deltaDir = pDelta[i]/deltaLength;

		int *vedg = mesh.vedg[i].Addr(0);
		float bestDotProd=0.0f;
		for (int j=0; j<vct; j++) {
			// Edge we may want to move along.
			Point3 edgeVector = mesh.P(mesh.e[vedg[j]].OtherVert(i)) - mesh.P(i);
			float edgeLength = Length (edgeVector);
			if (edgeLength<.0001f) continue;
			Point3 edgeDir = edgeVector/edgeLength;

			float dp;
			if ((dp=DotProd (deltaDir, edgeDir)) <= bestDotProd) continue;

			// Best match yet to desired direction:
			bestDotProd = dp;
			float desiredLength = deltaLength/dp;
			if (desiredLength>edgeLength) pConstr[i] = edgeVector;
			else pConstr[i] = edgeDir * desiredLength;
		}
		if (bestDotProd == 0.0f) {
			// Nowhere to go - don't move at all.
			pConstr[i] = Point3(0,0,0);
		}
	}
	
	return mptConstrainedDelta;
}

Tab<Point3> *DeltaConstrainer::ConstrainToFaces (MNMesh & mesh, Tab<Point3> & tDelta) {
	Allocate ();

	if (!mesh.GetFlag (MN_MESH_FILLED_IN)) return mptConstrainedDelta;
	if (!mesh.vfac) return mptConstrainedDelta;
	if (!tDelta.Count()) return mptConstrainedDelta;

	mptConstrainedDelta->SetCount (tDelta.Count());
	Point3 *pDelta = tDelta.Addr(0);
	Point3 *pConstr = mptConstrainedDelta->Addr(0);

	for (int i=0; i<tDelta.Count(); i++) {
		pConstr[i] = pDelta[i];	// Initialize.
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;	// Dead - who cares how we move.
		if (pDelta[i]==Point3(0,0,0)) continue;	// nothing to constrain

		// We want this vertex to move along the face (if any) that takes
		// it roughly the right direction.
		int vct = mesh.vfac[i].Count();
		if (!vct) continue;	// no faces - move as if unconstrained.

		// Examine where we're being asked to go:
		float deltaLength = Length (pDelta[i]);
		if (deltaLength<.0001f) continue;	// Delta tiny - move as if unconstrained.
		Point3 deltaDir = pDelta[i]/deltaLength;

		int *vfac = mesh.vfac[i].Addr(0);
		float bestLength=0.0f;
		for (int j=0; j<vct; j++) {
			// Find face plane:
			Point3 faceNormal = mesh.GetFaceNormal (vfac[j], true);

			// Project delta into this face's plane:
			Point3 faceVector = pDelta[i] - faceNormal * DotProd(faceNormal, pDelta[i]);

			// Find out if projection points into face, or away from it:
			MNFace & face = mesh.f[vfac[j]];
			int vertIndex = face.VertIndex (i);
			Point3 edgeIn = mesh.P(i) - mesh.P(face.vtx[(vertIndex+face.deg-1)%face.deg]);
			Point3 edgeOut = mesh.P(face.vtx[(vertIndex+1)%face.deg]) - mesh.P(i);

			// Face vector should be between these two edges' vectors:
			if (DotProd ((edgeIn^faceVector), faceNormal) <= 0) continue;
			if (DotProd ((edgeOut^faceVector), faceNormal) <= 0) continue;

			float faceLength;
			if ((faceLength = Length(faceVector)) < bestLength) continue;

			// Best match yet to desired direction:
			bestLength = faceLength;
			pConstr[i] = faceVector;
		}
		if (bestLength == 0.0f) {
			// Nowhere to go - don't move at all.
			pConstr[i] = Point3(0,0,0);
		}
	}
	
	return mptConstrainedDelta;
}

void EditPolyObject::ApplyMapDelta (int mapChannel, Tab<UVVert> & mapDelta, EPoly *pEPoly, TimeValue t) {
	MapVertRestore *mvr = NULL;
	if (theHold.Holding ()) mvr = new MapVertRestore (this, mapChannel);
	if (mm.M(mapChannel)->GetFlag (MN_DEAD)) return;
	UVVert *mv = mm.M(mapChannel)->v;
	for (int i=0; i<mm.M(mapChannel)->numv; i++) mv[i] += mapDelta[i];
	if (mvr) {
		if (!mvr->After()) {	// no changes occurred.
			delete mvr;
			return;
		}
		theHold.Put (mvr);
	}
	pEPoly->LocalDataChanged ((mapChannel<1) ? PART_VERTCOLOR : PART_TEXMAP);
}

Point3 *EditPolyObject::ConstrainDelta (MNMesh & mesh, TimeValue t, Tab<Point3> & delta) {
	if (!mSuspendConstraints) {
		int constrainType;
		pblock->GetValue (ep_constrain_type, t, constrainType, FOREVER);
		switch (constrainType) {
		case 1:
			return theDeltaConstrainer.ConstrainToEdges(mm, delta)->Addr(0);
			break;
		case 2:
			return theDeltaConstrainer.ConstrainToFaces(mm, delta)->Addr(0);
			break;
		}
	}
	return delta.Addr(0);
}

void EditPolyObject::ApplyDelta (Tab<Point3> & delta, EPoly *pEPoly, TimeValue t) {
	if (delta.Count() == 0) return;
	Point3 *pDelta = ConstrainDelta (mm, t, delta);

	if (AreWeKeying(t)) {
		BOOL addedCont = FALSE;
//		BOOL setkeymode = GetSetKeyMode(); SetSetKeyMode(FALSE);
		for (int i=0; i<mm.numv; i++) {
			if (mm.v[i].GetFlag (MN_DEAD)) continue;
			if (pDelta[i] == Point3(0,0,0)) continue;
			if (!PlugControl(t, i)) continue;
			addedCont = TRUE;
			// Set initial position:
			SuspendSetKeyMode();	// CAL-08/15/03: TODO: can we move suspend/resume outside the loop to improve performance?
				cont[i]->SetValue (TimeValue(0), &(mm.v[i].p));
			ResumeSetKeyMode();
		}
//		SetSetKeyMode(setkeymode);
		if (addedCont) NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	if (theHold.Holding ()) theHold.Put (new MeshVertRestore (this));
	for (int i=0; i<mm.numv; i++) {
		if (pDelta[i] == Point3(0,0,0)) continue;
		mm.v[i].p += pDelta[i];
		if (cont.Count() && cont[i]) cont[i]->SetValue (t, &(mm.v[i].p));
	}
	pEPoly->LocalDataChanged (PART_GEOM);
	pEPoly->RefreshScreen ();
}

MNTempData *EditPolyObject::TempData () {
	if (!tempData) tempData = new MNTempData(&(mm));
	return tempData;
}

void EditPolyObject::InvalidateTempData (PartID parts) {
	if (!tempMove) {
		if (tempData) tempData->Invalidate (parts);
		if (parts & (PART_TOPO|PART_GEOM|PART_SELECT|PART_SUBSEL_TYPE))
			InvalidateSoftSelectionCache ();
	}
	// we NEVER call InvalidateTopoCache, since that trashes the edge list.
	if (parts & PART_TOPO) mm.freeRVerts();
	if (parts & PART_GEOM) mm.InvalidateGeomCache ();
	if ((parts & PART_SELECT) && tempData) tempData->freeBevelInfo();


	// Any change in mesh should invalidate the preview.
	if (EpPreviewOn()) EpPreviewInvalidate ();
}

static int dragRestored;

void EditPolyObject::DragMoveInit () {
	if (tempMove) delete tempMove;
	tempMove = new TempMoveRestore (this);
	dragRestored = TRUE;
}

void EditPolyObject::DragMoveRestore () {
	if (!tempMove) return;
	if (dragRestored) return;
	tempMove->Restore (this);
	dragRestored = TRUE;
	LocalDataChanged (PART_GEOM|PART_TEXMAP|PART_VERTCOLOR);
}

void EditPolyObject::DragMove (Tab<Point3> & delta, EPoly *pEPoly, TimeValue t) {
	if (!tempMove) {
		ApplyDelta (delta, pEPoly, t);
		return;
	}

	if (delta.Count() == 0) return;
	Point3 *pDelta = ConstrainDelta (mm, t, delta);

	for (int i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (pDelta[i] == Point3(0,0,0)) continue;
		tempMove->active.Set (i);
		mm.v[i].p += pDelta[i];
	}
	if (theHold.Holding ()) theHold.Put (new CueDragRestore(this));
	dragRestored = FALSE;
	pEPoly->LocalDataChanged (PART_GEOM);
}

void EditPolyObject::DragMap (int mapChannel, Tab<UVVert> & mapDelta, EPoly *pEPoly, TimeValue t) {
	if (!tempMove) {
		ApplyMapDelta (mapChannel, mapDelta, pEPoly, t);
		return;
	}
	if (mm.M(mapChannel)->GetFlag (MN_DEAD)) return;
	UVVert *mv = mm.M(mapChannel)->v;
	int numMapVerts = mm.M(mapChannel)->numv;
	for (int i=0; i<numMapVerts; i++) mv[i] += mapDelta[i];
	pEPoly->LocalDataChanged ((mapChannel<1) ? PART_VERTCOLOR : PART_TEXMAP);
}

void EditPolyObject::DragMoveAccept (TimeValue t) {
	if (!tempMove) return;
	if (!tempMove->active.NumberSet()) {
		delete tempMove;
		tempMove = NULL;
		return;
	}

	// Check for polygons whose triangulation might now be invalid.
	// Also check to see if we need to add controllers.
	DWORD partsChanged = PART_GEOM|PART_TEXMAP|PART_VERTCOLOR;
	bool addedCont = false, needControllers = AreWeKeying(t) ? true : false;
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	for (int i=0; i<tempMove->active.GetSize(); i++) {
		if (!tempMove->active[i]) continue;
		if (needControllers && PlugControl (t,i)) {
			addedCont = true;
			if(Animating()) cont[i]->SetValue (0, tempMove->init[i]);
		}

		if (!mm.vfac) {
			DbgAssert (false);
			continue;
		}
		// For each face using this vertex:
		for (int j=0; j<mm.vfac[i].Count(); j++) {
			int fj = mm.vfac[i][j];
			// For each triangle using this vertex:
			Tab<int> tri;
			Point3 fnorm = mm.GetFaceNormal (fj, true);
			mm.f[fj].GetTriangles (tri);
			for (int k=0; k<tri.Count(); k+=3) {
				for (int kk=0; kk<3; kk++) {
					if (mm.f[fj].vtx[tri[k+kk]] == i) break;
				}
				if (kk==3) continue;

				// Check to see if triangle has flipped normal:
				Point3 A = mm.v[mm.f[fj].vtx[tri[k+1]]].p - mm.v[mm.f[fj].vtx[tri[k]]].p;
				Point3 B = mm.v[mm.f[fj].vtx[tri[k+2]]].p - mm.v[mm.f[fj].vtx[tri[k]]].p;
				if (DotProd (A^B, fnorm) < -.001f) break;
			}
			if (k<tri.Count()) {
				mm.RetriangulateFace (fj);
				partsChanged |= PART_TOPO;
			}
		}
	}
	if (tchange) {
		if (partsChanged & PART_TOPO) {
			tchange->After ();
			theHold.Put (tchange);
		} else {
			delete tchange;
		}
	}


	if (addedCont) NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);

	if (cont.Count()) {
		for (i=0; i<tempMove->active.GetSize(); i++) {
			if (!tempMove->active[i]) continue;
#ifndef WEBVERSION // fix for bug #415902 - not a problem in luna
			if (cont[i]) cont[i]->SetValue (t, &(mm.v[i].p));
#else // WEBVERSION
			if ( i < cont.Count() ) if (cont[i]) cont[i]->SetValue (t, &(mm.v[i].p));
#endif // WEBVERSION
		}
	}
	if (theHold.Holding()) {
		MeshVertRestore *mvr = new MeshVertRestore(this);
		memcpy (mvr->GetUndoPoints(), tempMove->init.Addr(0), mm.numv*sizeof(Point3));
		theHold.Put (mvr);
		for (int mapChannel=-NUM_HIDDENMAPS; mapChannel<mm.numm; mapChannel++) {
			if (mm.M(mapChannel)->GetFlag (MN_DEAD)) continue;
			int offsetMapChannel = NUM_HIDDENMAPS + mapChannel;
			MapVertRestore *mpvr = new MapVertRestore (this, mapChannel);
			memcpy (mpvr->GetUndoPoints(), tempMove->mapInit[offsetMapChannel].Addr(0),
				mpvr->NumUndoPoints()*sizeof(UVVert));
			if (!mpvr->After ()) delete mpvr;	// No changes in this channel
			else theHold.Put (mpvr);
		}
	}
	delete tempMove;
	tempMove = NULL;
	dragRestored = TRUE;
	LocalDataChanged (partsChanged);
}

void EditPolyObject::DragMoveClear () {
	if (tempMove) delete tempMove;
	tempMove=NULL;
}

// Class EPolyBackspaceUser: Used to process backspace key input.
void EPolyBackspaceUser::Notify() {
	if (!mpEditPoly) return;

	// If we're in create-face mode, send the backspace event there:
	if ((mpEditPoly->createFaceMode)  &&
		(mpEditPoly->ip->GetCommandMode () == mpEditPoly->createFaceMode)) {
		mpEditPoly->createFaceMode->Backspace ();
		return;
	}

	// Otherwise, use it to generate a "remove" event:
	if ((mpEditPoly->selLevel > EP_SL_OBJECT) && (mpEditPoly->selLevel < EP_SL_FACE)) {
		theHold.Begin ();
		mpEditPoly->EpActionButtonOp (epop_remove);
		theHold.Accept (GetString (IDS_REMOVE));
	}
}

// New Preview mode:
void EditPolyObject::EpPreviewClear () {
	mPreviewOn = false;
	mPreviewOperation = epop_null;
	mPreviewValid = false;
	mPreviewDrag = false;
	mPreviewMesh.Clear ();
	PreviewTempDataFree ();
}

void EditPolyObject::EpPreviewBegin (int previewOperation) {
	mPreviewOn = true;
	mPreviewOperation = previewOperation;
	NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
	RefreshScreen ();
}

void EditPolyObject::EpPreviewCancel () {
	if (!mPreviewOn) return;
	EpPreviewClear ();
	subdivValid.SetEmpty ();
	NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
	RefreshScreen ();
}

void EditPolyObject::EpPreviewAccept () {
	if (!mPreviewOn) return;
	mPreviewOn = false;	// necessary to avoid "flashing" in EpActionButtonOp's redraw.
	EpActionButtonOp (mPreviewOperation);
	EpPreviewClear ();
}

void EditPolyObject::EpPreviewInvalidate () {
	if (mPreviewDrag) {
		int fullyInteractive;
		pblock->GetValue (ep_interactive_full, TimeValue(0), fullyInteractive, FOREVER);
		if (!fullyInteractive) return;	// Can't invalidate while dragging.
	}
	if (mPreviewValid && mPreviewOn) NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
	mPreviewValid = false;
	PreviewTempDataFree ();
}

bool EditPolyObject::EpPreviewOn () {
	if (mPreviewSuspend) return false;
	// SCA 12/15/01:
	// Extrude, Chamfer, Bevel, and Inset all have interactive preview modes
	// using the Drag methods, not this Preview Mesh approach.  This is better for
	// them because the user can get an update faster by only dragging
	// geometry, instead of reconstructing the topological operation.
	// However, it means that we need to protect against a double-preview
	// that would occur if the dialog was up while a face was being actively
	// extruded, etc.  Thus:
	if (!mPreviewOn) return false;
	switch (mPreviewOperation) {
	case epop_extrude:
		if (inExtrude) return false;
		break;
	case epop_bevel:
		if (inBevel) return false;
		break;
	case epop_chamfer:
		if (inChamfer) return false;
		break;
	case epop_inset:
		if (inBevel) return false;
		break;
	// Note: epop_outline doesn't have a topo component, so it uses the normal preview.
	}
	return true;
}

MNTempData *EditPolyObject::PreviewTempData () {
	if (!mpPreviewTemp) mpPreviewTemp = new MNTempData;
	mpPreviewTemp->SetMesh (&mPreviewMesh);
	return mpPreviewTemp;
}

void EditPolyObject::PreviewTempDataFree () {
	if (mpPreviewTemp) {
		delete mpPreviewTemp;
		mpPreviewTemp = NULL;
	}
}

void EditPolyObject::UpdatePreviewMesh () {
	if (!mPreviewOn) return;
	if (mPreviewValid) return;
	mPreviewMesh = mm;
	PreviewTempDataFree ();
	ApplyMeshOp (mPreviewMesh, PreviewTempData(), mPreviewOperation);

	// Weld dialog has readouts of before, after vertex counts.
	// This seems like a logical place to update them.
	if (mPreviewOperation == epop_weld_sel) {
		HWND hWnd = GetDlgHandle (ep_settings);
		if (hWnd) {
			TSTR buf;
			HWND hBefore = GetDlgItem (hWnd, IDC_WELD_BEFORE);
			if (hBefore) {
				int numBefore = mm.numv;
				for (int i=0; i<mm.numv; i++) {
					if (mm.v[i].GetFlag (MN_DEAD)) numBefore--;
				}
				buf.printf ("%d", numBefore);
				LPCTSTR lbuf = static_cast<LPCTSTR>(buf);
				SendMessage (hBefore, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lbuf));
			}

			HWND hAfter = GetDlgItem (hWnd, IDC_WELD_AFTER);
			if (hAfter) {
				int numAfter = mPreviewMesh.numv;
				for (int i=0; i<mPreviewMesh.numv; i++) {
					if (mPreviewMesh.v[i].GetFlag (MN_DEAD)) numAfter--;
				}
				buf.printf ("%d", numAfter);
				LPCTSTR lbuf = static_cast<LPCTSTR>(buf);
				SendMessage (hAfter, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(lbuf));
			}
		}
	}

	subdivValid.SetEmpty ();
	mPreviewValid = true;
}

// For Maxscript access:
// Methods to get information about our mesh.
Point3 EditPolyObject::EpfnGetVertex (int vertIndex) {
	if ((vertIndex<0) || (vertIndex>=mm.numv)) return Point3(0,0,0);
	return mm.v[vertIndex].p;
}

int EditPolyObject::EpfnGetVertexFaceCount (int vertIndex) {
	if ((vertIndex<0) || (vertIndex>=mm.numv) || !mm.vfac) return -1;
	return mm.vfac[vertIndex].Count();
}

int EditPolyObject::EpfnGetVertexFace (int vertIndex, int whichFace) {
	if ((vertIndex<0) || (vertIndex>=mm.numv) || !mm.vfac) return -1;
	if ((whichFace<0) || (whichFace>=mm.vfac[vertIndex].Count())) return -1;
	return mm.vfac[vertIndex][whichFace];
}

int EditPolyObject::EpfnGetVertexEdgeCount (int vertIndex) {
	if ((vertIndex<0) || (vertIndex>=mm.numv) || !mm.vedg) return 0;
	return mm.vedg[vertIndex].Count();
}

int EditPolyObject::EpfnGetVertexEdge (int vertIndex, int whichEdge) {
	if ((vertIndex<0) || (vertIndex>=mm.numv) || !mm.vedg) return -1;
	if ((whichEdge<0) || (whichEdge>=mm.vedg[vertIndex].Count())) return -1;
	return mm.vedg[vertIndex][whichEdge];
}

int EditPolyObject::EpfnGetEdgeVertex (int edgeIndex, int end) {
	if ((edgeIndex<0) || (edgeIndex>=mm.nume)) return -1;
	if ((end<0) || (end>1)) return -1;
	return mm.e[edgeIndex][end];
}

int EditPolyObject::EpfnGetEdgeFace (int edgeIndex, int side) {
	if ((edgeIndex<0) || (edgeIndex>=mm.nume)) return -1;
	if ((side<0) || (side>1)) return -1;
	return side ? mm.e[edgeIndex].f2 : mm.e[edgeIndex].f1;
}

int EditPolyObject::EpfnGetFaceDegree (int faceIndex) {
	if ((faceIndex<0) || (faceIndex>=mm.numf)) return 0;
	return mm.f[faceIndex].deg;
}

int EditPolyObject::EpfnGetFaceVertex (int faceIndex, int corner) {
	if ((faceIndex<0) || (faceIndex>=mm.numf)) return -1;
	if ((corner<0) || (corner>=mm.f[faceIndex].deg)) return -1;
	return mm.f[faceIndex].vtx[corner];
}

int EditPolyObject::EpfnGetFaceEdge (int faceIndex, int side) {
	if ((faceIndex<0) || (faceIndex>=mm.numf)) return -1;
	if ((side<0) || (side>=mm.f[faceIndex].deg)) return -1;
	return mm.f[faceIndex].edg[side];
}

int EditPolyObject::EpfnGetFaceMaterial (int faceIndex) {
	if ((faceIndex<0) || (faceIndex>=mm.numf)) return 0;
	return mm.f[faceIndex].material;
}

DWORD EditPolyObject::EpfnGetFaceSmoothingGroup (int faceIndex) {
	if ((faceIndex<0) || (faceIndex>=mm.numf)) return 0;
	return mm.f[faceIndex].smGroup;
}

bool EditPolyObject::EpfnGetMapChannelActive (int mapChannel) {
	if ((mapChannel <= -NUM_HIDDENMAPS) || (mapChannel >= mm.numm)) return false;
	return mm.M(mapChannel)->GetFlag (MN_DEAD) ? false : true;
}

int EditPolyObject::EpfnGetNumMapVertices (int mapChannel) {
	if (!EpfnGetMapChannelActive(mapChannel)) return 0;
	return mm.M(mapChannel)->numv;
}

UVVert EditPolyObject::EpfnGetMapVertex (int mapChannel, int vertIndex) {
	if (!EpfnGetMapChannelActive(mapChannel)) return UVVert(0,0,0);
	if ((vertIndex < 0) || (vertIndex >= mm.M(mapChannel)->numv)) return UVVert(0,0,0);
	return mm.M(mapChannel)->v[vertIndex];
}

int EditPolyObject::EpfnGetMapFaceVertex (int mapChannel, int faceIndex, int corner) {
	if (!EpfnGetMapChannelActive(mapChannel)) return -1;
	if ((faceIndex<0) || (faceIndex>=mm.numf)) return -1;
	if ((corner<0) || (corner>=mm.f[faceIndex].deg)) return -1;
	return mm.M(mapChannel)->f[faceIndex].tv[corner];
}

BOOL EditPolyObject::PolygonCount(TimeValue t, int& numFaces, int& numVerts) {
	UpdateEverything (t);

	int useSubdiv=false;
	if (pblock) pblock->GetValue (ep_surf_subdivide, t, useSubdiv, FOREVER);

	MNMesh *pMeshToDisplay = NULL;
	if (EpPreviewOn()) pMeshToDisplay = &mPreviewMesh;
	if (useSubdiv) pMeshToDisplay = &subdivResult;

	if (!pMeshToDisplay) return PolyObject::PolygonCount (t, numFaces, numVerts);

	numFaces = 0;
	numVerts = 0;
	for (int i=0; i<pMeshToDisplay->numf; i++) if (!pMeshToDisplay->f[i].GetFlag (MN_DEAD)) numFaces++;
	for (i=0; i<pMeshToDisplay->numv; i++) if (!pMeshToDisplay->v[i].GetFlag (MN_DEAD)) numVerts++;
	return TRUE;
}
