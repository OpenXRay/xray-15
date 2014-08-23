#include "VertexPaint.h"

static	PaintCommandMode thePaintCommandMode;
static	HCURSOR hPaintCursor = NULL;	// Paint cursor
static	HCURSOR hDropperCursor = NULL;	// Paint cursor
static	HCURSOR hNoPaintCursor = NULL;	// NoPaint cursor

TriObject *GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);

void NukePaintCommandMode()
{
	CommandMode* cmdMode = GetCOREInterface()->GetCommandMode();
	if (!cmdMode || cmdMode->ID() == thePaintCommandMode.ID()) {
		// Our command mode is at the top of the stack.
		// Set selection mode and remove our mode
		// We need to make sure some kind of command mode is set, so
		// we set selection mode to be the default.
		GetCOREInterface()->SetStdCommandMode(CID_OBJSELECT);
		GetCOREInterface()->DeleteMode(&thePaintCommandMode);
	}
	else {
		// Our command mode is not at the top of the stack,
		// so we can safely remove our mode.
		GetCOREInterface()->DeleteMode(&thePaintCommandMode);
	}
}

BOOL VertexPaint::ActivatePaint(BOOL bOnOff, BOOL bPick)
	{
	
	if (bOnOff) {
		
		thePaintCommandMode.mouseProc.SetPickMode(bPick);
		
		if (!hPaintCursor) {
			hPaintCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_PAINTCURSOR));
		}
		if (!hNoPaintCursor) {
			hNoPaintCursor = LoadCursor(NULL,IDC_CROSS);
		}
		if (!hDropperCursor) {
			hDropperCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_DROPPER_CURSOR));
		}

		thePaintCommandMode.SetInterface(GetCOREInterface());
		thePaintCommandMode.SetModifier(this);
		ip->SetCommandMode(&thePaintCommandMode);
		}
	else {
		NukePaintCommandMode();
		}

	return TRUE;
	}

ModContext* VertexPaint::ModContextFromNode(INode* node)
	{
	Object* obj = node->GetObjectRef();

	if (!obj)	return FALSE;

    while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
		IDerivedObject* dobj = (IDerivedObject*)obj;
		int m;
		int numMods = dobj->NumModifiers();

		for (m=0; m<numMods; m++) {
			ModContext* mc = dobj->GetModContext(m);
			for (int i=0; i<modContexts.Count(); i++) {
				if (mc == modContexts[i]) {
					return mc;
					}
				}
			}

		obj = dobj->GetObjRef();
		}

	return NULL;
	}

void PaintMouseProc::DoPainting(HWND hWnd, IPoint2 m)
	{
	INode*		node = GetCOREInterface()->PickNode(hWnd,m,NULL);
	TimeValue	t = GetCOREInterface()->GetTime();
	Ray			ray;

	if (pModifier->IsValidNode(node)) {
		SetCursor(hPaintCursor);
		ModContext* mc = pModifier->ModContextFromNode(node);

		// If we got an instanced modifier but the second node is not selected
		// it mght be valid, but it will return NULL for the ModContext
		if(!mc || !mc->localData) return;

		VertexPaintData* d = (VertexPaintData*)mc->localData;
		ViewExp*	pView = GetCOREInterface()->GetViewport(hWnd);
		Mesh*		mesh = 	d->GetMesh();
		MNMesh*		pMNMesh = 	d->GetMNMesh();

		if (mesh) {
			pView->MapScreenToWorldRay((float)m.x, (float)m.y, ray);
			Matrix3 obtm  = node->GetObjTMAfterWSM(t);
			Matrix3 iobtm = Inverse(obtm);
			ray.p   = iobtm * ray.p;
			ray.dir = VectorTransform(iobtm, ray.dir);	

			float at;
			Point3 norm;
			DWORD fi;
			Point3 bary;
			float opacity;

			if (mesh->IntersectRay(ray, at, norm, fi, bary)) {
				Face* f = &mesh->faces[fi];
				for (int i=0; i<3; i++) {
					opacity = bary[i]*pModifier->fTint;
					if(opacity > 1) opacity = 1;
					if(opacity < 0) opacity = 0;
					if(mesh->selLevel != MESH_FACE || mesh->FaceSel()[fi]) {
						for (int j=0; j<2; j++) {
							if (!pModifier->affectChannel[j]) continue;
							d->SetColor (f->v[i],  opacity, fi, i, pModifier->GetActiveColor(), j);
						}
						if (pModifier->affectChannel[2])
							d->SetAlpha (f->v[i],  opacity, fi, i, pModifier->alpha);
					}
				}
				pModifier->NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(t);
			}

			GetCOREInterface()->ReleaseViewport(pView);
			}

		else if (pMNMesh){
			pView->MapScreenToWorldRay((float)m.x, (float)m.y, ray);
			Matrix3 obtm  = node->GetObjTMAfterWSM(t);
			Matrix3 iobtm = Inverse(obtm);
			ray.p   = iobtm * ray.p;
			ray.dir = VectorTransform(iobtm, ray.dir);	

			float at;
			Point3 norm;
			int fi;
			Tab<float> bary;
			float opacity;

			if (pMNMesh->IntersectRay(ray, at, norm, fi, bary)) {
				MNFace* pFace = pMNMesh->F(fi);
				for (int i=0; i<pFace->deg; i++) {
					opacity = bary[i]*pModifier->fTint;
					if(opacity > 1) opacity = 1;
					if(opacity < 0) opacity = 0;
					if(pMNMesh->selLevel != MNM_SL_FACE || pFace->GetFlag(MN_SEL)) {
						for (int j=0; j<2; j++) {
							if (!pModifier->affectChannel[j]) continue;
							d->SetColor(pFace->vtx[i],  opacity ,fi,i, pModifier->GetActiveColor(), j);
						}
						if (pModifier->affectChannel[2])
							d->SetAlpha (pFace->vtx[i], opacity, fi, i, pModifier->alpha);
					}
				}
				pModifier->NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(t);
			}
			GetCOREInterface()->ReleaseViewport(pView);
		}

		else {
			SetCursor(hNoPaintCursor);
		}
	}
	return;
}

void PaintMouseProc::DoPickColor(HWND hWnd, IPoint2 m)
	{
	INode*		node = GetCOREInterface()->PickNode(hWnd,m,NULL);
	TimeValue	t = GetCOREInterface()->GetTime();
	Ray			ray;

	if(!node) 
	{
		SetCursor(hNoPaintCursor);
		return;
	}

	ObjectState os = node->EvalWorldState(t);
	
	if(os.obj->ClassID() == Class_ID(EDITTRIOBJ_CLASS_ID,0))
	{
		SetCursor(hDropperCursor);
		TriObject *pTri = (TriObject *) os.obj;

		ViewExp*	pView = GetCOREInterface()->GetViewport(hWnd);
		Mesh*		mesh = &pTri->mesh;

		if (mesh) {
			pView->MapScreenToWorldRay((float)m.x, (float)m.y, ray);
			Matrix3 obtm  = node->GetObjTMAfterWSM(t);
			Matrix3 iobtm = Inverse(obtm);
			ray.p   = iobtm * ray.p;
			ray.dir = VectorTransform(iobtm, ray.dir);	

			float at;
			Point3 norm;
			DWORD fi;
			Point3 bary;
			float opacity = 0;

			int mapChan = 0;
			if (!pModifier->affectChannel[0] && pModifier->affectChannel[1]) mapChan = MAP_SHADING;

			if (mesh->IntersectRay(ray, at, norm, fi, bary)) 
			{
				UVVert *pMapVerts = mesh->mapVerts(mapChan);
				if (pMapVerts) {
					TVFace* tvf = &mesh->mapFaces(mapChan)[fi];
					Color PickCol(0,0,0);
					for (int i=0; i<3; i++) {
						PickCol += (bary[i])*pMapVerts[tvf->t[i]];
					}
					pModifier->iColor->SetColor(PickCol.toRGB());
				} else {
					pModifier->iColor->SetColor (0xffffffff);
				}
				if (pModifier->affectChannel[2]) {
					pMapVerts = mesh->mapVerts (MAP_ALPHA);
					if (pMapVerts) {
						TVFace* tvf = &mesh->mapFaces(MAP_ALPHA)[fi];
						Color PickCol(0,0,0);
						for (int i=0; i<3; i++) {
							PickCol += (bary[i])*pMapVerts[tvf->t[i]];
						}
						pModifier->SetActiveAlpha (PickCol.r);
					} else {
						pModifier->SetActiveAlpha (1.0f);
					}
				}
			}

			GetCOREInterface()->ReleaseViewport(pView);
			}
		}

	else if (os.obj->IsSubClassOf(polyObjectClassID)) {
		SetCursor(hDropperCursor);
		PolyObject *pPolyObj = (PolyObject *) os.obj;

		ViewExp*	pView = GetCOREInterface()->GetViewport(hWnd);
		MNMesh*		pMNMesh = &pPolyObj->GetMesh();

		if (pMNMesh) {
			pView->MapScreenToWorldRay((float)m.x, (float)m.y, ray);
			Matrix3 obtm  = node->GetObjTMAfterWSM(t);
			Matrix3 iobtm = Inverse(obtm);
			ray.p   = iobtm * ray.p;
			ray.dir = VectorTransform(iobtm, ray.dir);	
			float at;
			Point3 norm;
			int fi;
			Tab<float> bary;
			float opacity = 0;
			int mapChan = 0;
			if (!pModifier->affectChannel[0] && pModifier->affectChannel[1]) mapChan = MAP_SHADING;
			if (pMNMesh->IntersectRay(ray, at, norm, fi, bary)) {
				MNMap* pVCMap = pMNMesh->M(mapChan);
				if (!pVCMap->GetFlag(MN_DEAD)) {
					MNMapFace* pVCFace = pVCMap->F(fi);
					Color PickCol(0,0,0);
					for (int i=0; i<pVCFace->deg; i++) {
						PickCol += (bary[i])*pVCMap->V(pVCFace->tv[i]);
					}		
					pModifier->iColor->SetColor(PickCol.toRGB());
				} else {
					Color PickCol(1,1,1);
					pModifier->iColor->SetColor (PickCol.toRGB());
				}
				if (pModifier->affectChannel[2]) {
					pVCMap = pMNMesh->M(MAP_ALPHA);
					if (!pVCMap->GetFlag (MN_DEAD)) {
						MNMapFace *pVCFace = pVCMap->F(fi);
						Color PickCol(0,0,0);
						for (int i=0; i<pVCFace->deg; i++) {
							PickCol += (bary[i])*pVCMap->V(pVCFace->tv[i]);
						}
						pModifier->SetActiveAlpha (PickCol.r);
					} else {
						pModifier->SetActiveAlpha (1.0f);
					}
				}
			}
			GetCOREInterface()->ReleaseViewport(pView);
		}
	}

	else {
			pModifier->iColor->SetColor(node->GetWireColor());
			}
		
	return;
	}

int PaintMouseProc::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	int			nRetval = FALSE;

	BOOL		bButtonDown = flags & MOUSE_LBUTTON;

	switch (msg) {
		case MOUSE_POINT:
			
			if( flags & MOUSE_CTRL || bPickMode )
				DoPickColor(hWnd, m);
			else
			{
				if(bButtonDown)
					MaybeStartHold();
				
				DoPainting(hWnd, m);
				
				if(!bButtonDown)
					MaybeEndHold();
			}

			break;
		case MOUSE_MOVE:
			if(flags & MOUSE_CTRL || bPickMode)
				DoPickColor(hWnd, m);
			else
				DoPainting(hWnd, m);
			break;
		case MOUSE_ABORT:
			if(wasHolding)
				theHold.Cancel();
			
			GetCOREInterface()->SetStdCommandMode(CID_OBJSELECT);
			break;
		case MOUSE_PROPCLICK:
			GetCOREInterface()->SetStdCommandMode(CID_OBJSELECT);
			break;
		case MOUSE_FREEMOVE:
			{				
				INode* node = GetCOREInterface()->PickNode(hWnd,m,NULL);

				if(flags & MOUSE_CTRL || bPickMode)
				{
					if(node)
						SetCursor(hDropperCursor);
					else
						SetCursor(hNoPaintCursor);
				}
				else
				{
					if (pModifier->IsValidNode(node)) {
						SetCursor(hPaintCursor);
					}
					else {
						SetCursor(hNoPaintCursor);
					}
				}
			}
			break;
	}
		
	return TRUE;
}

void PaintMouseProc::MaybeStartHold()
{
	if(!theHold.Holding()) {
		theHold.Begin();
		wasHolding = TRUE;
	}
	else wasHolding = FALSE;

	ModContextList	modContexts;
	INodeTab		nodeTab;
	GetCOREInterface()->GetModContexts(modContexts, nodeTab);

	for (int i=0; i<modContexts.Count(); i++) {
		ModContext *mc = modContexts[i];
		if(mc && mc->localData) {
			for (int j=0; j<3; j++) {
				if (!pModifier->affectChannel[j]) continue;
				if (j<2) theHold.Put(new VertexPaintColorRestore((VertexPaintData*)mc->localData,pModifier, j));
				else theHold.Put (new VertexPaintAlphaRestore ((VertexPaintData*)mc->localData, pModifier));
			}
		}
	}
}

void PaintMouseProc::MaybeEndHold() {
	if(wasHolding) theHold.Accept(GetString(IDS_RESTORE));
}

//=====================================

PaintCommandMode::PaintCommandMode()
{
	iInterface	= NULL;
}

int PaintCommandMode::Class()
{
	return PICK_COMMAND;
}

int PaintCommandMode::ID()
{
	return CID_PAINT;
}

MouseCallBack* PaintCommandMode::MouseProc(int *numPoints)
{
	*numPoints = 2; return &mouseProc;
}

ChangeForegroundCallback* PaintCommandMode::ChangeFGProc()
{
	return CHANGE_FG_SELECTED;
}

BOOL PaintCommandMode::ChangeFG( CommandMode *oldMode )
{
	return FALSE;
}

void PaintCommandMode::EnterMode() 
{
	iInterface->PushPrompt(GetString(IDS_PAINTPROMPT));
	pModifier->EnterMode();

}

void PaintCommandMode::ExitMode() 
{
	iInterface->PopPrompt();
	pModifier->ExitMode();

}

void VertexPaint::EnterMode()
	{
		int numPoints;
		iPaintButton->SetCheck(ip->GetCommandMode()->ID() == CID_PAINT && 
				!((PaintMouseProc *)ip->GetCommandMode()->MouseProc(&numPoints))->GetPickMode());
		iPickButton->SetCheck(ip->GetCommandMode()->ID() == CID_PAINT && 
				((PaintMouseProc *)ip->GetCommandMode()->MouseProc(&numPoints))->GetPickMode());
		ip->GetModContexts(modContexts, nodeTab);
	}

void VertexPaint::ExitMode()
	{
		iPaintButton->SetCheck(FALSE);
		iPickButton->SetCheck(FALSE);

		nodeTab.DisposeTemporary();
		modContexts.ZeroCount();
		modContexts.Shrink();
	}

BOOL VertexPaint::IsValidNode(INode* node)
	{
	if (!node) return FALSE;

	for (int i=0; i<nodeTab.Count(); i++) {
		if (nodeTab[i] == node) {
			return TRUE;
			}
		}
	return FALSE;
	}

COLORREF VertexPaint::GetActiveColor()
	{
	return iColor->GetColor();
	}

// ------------- Convert object to TriObject -----------------------

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject *GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(0, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object

		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
		}
	else {
		return NULL;
		}
	}
