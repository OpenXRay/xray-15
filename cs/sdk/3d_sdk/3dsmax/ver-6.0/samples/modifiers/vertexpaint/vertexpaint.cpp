/**********************************************************************
 *<
	FILE: VertexPaint.cpp

	DESCRIPTION:	Modifier implementation	

	CREATED BY: Christer Janson, Nikolai Sander

	HISTORY: 

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "VertexPaint.h"
#include "meshdelta.h"

// flags:
#define VP_DISP_END_RESULT 0x01

static WNDPROC colorSwatchOriginalWndProc;

static	HIMAGELIST hButtonImages = NULL;

static void LoadImages() {
	if (hButtonImages) return;
	HBITMAP hBitmap, hMask;
	hButtonImages = ImageList_Create(15, 14, ILC_MASK, 2, 0);	// 17 is kluge to center square. -SA
	hBitmap     = LoadBitmap (hInstance,MAKEINTRESOURCE(IDB_BUTTONS));
	hMask       = LoadBitmap (hInstance,MAKEINTRESOURCE(IDB_BUTTON_MASK));
	ImageList_Add(hButtonImages, hBitmap, hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
}

ClassDesc* GetVertexPaintDesc();


class VertexPaintClassDesc:public ClassDesc {
	public:
	// MAB - 5/23/03 - Original Vertex Paint is retired and removed from the UI;
	// Class name changed to OldVertexPaint to avoid maxscript name conflict with the newer modifier
	int 			IsPublic()					{return 0;}

	void *			Create(BOOL loading = FALSE){return new VertexPaint();}
	const TCHAR *	ClassName()					{return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID()				{return OSM_CLASS_ID;}
	Class_ID		ClassID()					{return VERTEXPAINT_CLASS_ID;}
	const TCHAR* 	Category()					{return GetString(IDS_CATEGORY);}
	void			ResetClassParams(BOOL fileReset) {}
	};

static VertexPaintClassDesc VertexPaintDesc;
ClassDesc* GetVertexPaintDesc() {return &VertexPaintDesc;}

static INT_PTR CALLBACK VertexPaintDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	int numPoints;
	ISpinnerControl *spin;
	VertexPaint *mod = (VertexPaint*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!mod && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {
	case WM_INITDIALOG:
		LoadImages();
		mod = (VertexPaint*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
		mod->hParams = hWnd;
		mod->iPaintButton = GetICustButton(GetDlgItem(hWnd, IDC_PAINT));
		mod->iPaintButton->SetType(CBT_CHECK);
		mod->iPaintButton->SetHighlightColor(GREEN_WASH);
		mod->iPaintButton->SetCheck(mod->ip->GetCommandMode()->ID() == CID_PAINT && 
			!((PaintMouseProc *)mod->ip->GetCommandMode()->MouseProc(&numPoints))->GetPickMode());
		mod->iPaintButton->SetImage(hButtonImages,0,0,0,0,15,14);
		mod->iPaintButton->SetTooltip (TRUE, GetString (IDS_PAINT));

		mod->iPickButton = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
		mod->iPickButton->SetType(CBT_CHECK);
		mod->iPickButton->SetHighlightColor(GREEN_WASH);
		mod->iPickButton->SetCheck(mod->ip->GetCommandMode()->ID() == CID_PAINT && 
			((PaintMouseProc *)mod->ip->GetCommandMode()->MouseProc(&numPoints))->GetPickMode());
		mod->iPickButton->SetImage(hButtonImages,1,1,1,1,15,14);
		mod->iPickButton->SetTooltip (TRUE, GetString (IDS_PICK));

		mod->iColor = GetIColorSwatch(GetDlgItem(hWnd, IDC_COLOR));
		mod->iColor->SetColor(mod->lastColor);

		CheckDlgButton (hWnd, IDC_AFFECT_COLOR, mod->affectChannel[0]);
		CheckDlgButton (hWnd, IDC_AFFECT_ILLUM, mod->affectChannel[1]);
		CheckDlgButton (hWnd, IDC_AFFECT_ALPHA, mod->affectChannel[2]);
		SetupFloatSpinner(hWnd, IDC_ALPHA_SPIN, IDC_ALPHA,
			0.0f, 100.0f, mod->alpha*100.0f, 0.1f);
		break;

	case WM_POSTINIT:
		mod->InitPalettes();
		break;

	case CC_COLOR_CHANGE:
		if (LOWORD(wParam) == IDC_COLOR) {
			IColorSwatch* iCol = (IColorSwatch*)lParam;
			mod->lastColor = iCol->GetColor();
			}
		break;

	case WM_DESTROY:
		mod->SavePalettes();
		mod->iPaintButton = NULL;
		mod->iPickButton = NULL;
		mod->iColor = NULL;
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl *) lParam;
		switch (LOWORD(wParam)) {
		case IDC_TINT_SPIN:
			mod->fTint = spin->GetFVal()/100.f;
			break;
		case IDC_ALPHA_SPIN:
			mod->alpha = spin->GetFVal()/100.f;
			break;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_PAINT:
			mod->ActivatePaint(mod->iPaintButton->IsChecked());
			break;
		case IDC_PICK:
			mod->ActivatePaint(mod->iPickButton->IsChecked(),TRUE);
			break;
		case IDC_VC_ON:
			mod->TurnVCOn(FALSE);
			break;
		case IDC_SHADED:
			mod->TurnVCOn(TRUE);
			break;
		case IDC_AFFECT_COLOR:
			mod->affectChannel[0] = IsDlgButtonChecked(hWnd, IDC_AFFECT_COLOR) ? true : false;
			break;
		case IDC_AFFECT_ILLUM:
			mod->affectChannel[1] = IsDlgButtonChecked(hWnd, IDC_AFFECT_ILLUM) ? true : false;
			break;
		case IDC_AFFECT_ALPHA:
			mod->affectChannel[2] = IsDlgButtonChecked(hWnd, IDC_AFFECT_ALPHA) ? true : false;
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
	}

// Subclass procedure 
LRESULT APIENTRY colorSwatchSubclassWndProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{
	switch (uMsg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK: {
			HWND hPanel = GetParent(hwnd);
			LONG_PTR mod = GetWindowLongPtr(hPanel,GWLP_USERDATA);
			if (mod) {
				((VertexPaint*)mod)->PaletteButton(hwnd);
				}
			}
			break;
		case WM_DESTROY:
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) colorSwatchOriginalWndProc); 
			// Fallthrough...
		default:
			return CallWindowProc(colorSwatchOriginalWndProc, hwnd, uMsg, wParam, lParam); 
			break;
		}
	return 0;
	}
 

IObjParam *VertexPaint::ip			= NULL;
HWND VertexPaint::hParams			= NULL;		
VertexPaint* VertexPaint::editMod	= NULL;
ICustButton* VertexPaint::iPaintButton	= NULL;
ICustButton* VertexPaint::iPickButton	= NULL;
IColorSwatch* VertexPaint::iColor	= NULL;
COLORREF VertexPaint::lastColor		= RGB(255,255,255);
COLORREF VertexPaint::palColors[]	= {
	RGB(255,  0,  0),	RGB(  0,255,  0),	RGB(  0,  0,255),	RGB(255,255,255),
	RGB(255,255,  0),	RGB(  0,255,255),	RGB(255,  0,255),	RGB(170,170,170),
	RGB(128,  0,  0),	RGB(  0,128,  0),	RGB(  0,  0,128),	RGB( 85, 85, 85),
	RGB(128,128,  0),	RGB(  0,128,128),	RGB(128,  0,128),	RGB(  0,  0,  0)
	};


//--- VertexPaint -------------------------------------------------------
VertexPaint::VertexPaint() : iTint(NULL), fTint(1.0f)
	{
	flags = 0x0;
	for (int i=0; i<3; i++) affectChannel[i] = (i==0) ? true : false;
	alpha = 1.0f;
	}

VertexPaint::~VertexPaint()
	{
	}

Interval VertexPaint::LocalValidity(TimeValue t)
	{
	return FOREVER;
	}

BOOL VertexPaint::DependOnTopology(ModContext &mc)
	{
	return TRUE;
	}

RefTargetHandle VertexPaint::Clone(RemapDir& remap)
	{
	VertexPaint* newmod = new VertexPaint();	
	BaseClone(this, newmod, remap);
	for (int i=0; i<3; i++) newmod->affectChannel[i] = affectChannel[i];
	newmod->alpha = alpha;
	return(newmod);
	}

void VertexPaint::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
	{
	if (!mc->localData) return;
	((VertexPaintData*)mc->localData)->FreeCache();

	}

void VertexPaint::ModifyTriObject(TriObject* tobj, TimeValue t, ModContext &mc) {
	tobj->ReadyChannelsForMod(GEOM_CHANNEL|TOPO_CHANNEL|VERTCOLOR_CHANNEL|TEXMAP_CHANNEL);
	Mesh* mesh = &tobj->GetMesh();
	if (!mesh) return;
	if (mesh->selLevel == MESH_EDGE) return;
	UVVert *mv;
	TVFace *mf;

	VertexPaintData *d  = (VertexPaintData*)mc.localData;
	if (!d) mc.localData = d = new VertexPaintData(tobj->GetMesh());
	if (!d->GetMesh()) d->SetCache(*mesh);

	MeshDelta md(*mesh);
	for (int chan = 0; chan < 3; chan++) {
		if (!d->ChannelActive(chan)) continue;

		// If the incoming Mesh had no vertex colors, this will add a default map to start with.
		// The default map has the same topology as the Mesh (so one color per vertex),
		// with all colors set to white.
		if (!mesh->mapSupport(-chan)) {
			md.AddMap (-chan);
			mv = NULL;
			mf = NULL;
		} else {
			mv = mesh->mapVerts(-chan);
			mf = mesh->mapFaces(-chan);
		}

		// If we're not in face SO mode, it's easier !
		if(mesh->selLevel == MESH_OBJECT || mesh->selLevel == MESH_VERTEX) {
			// We used two routines -- VCreate to add new map vertices, and FRemap to make the
			// existing map faces use the new verts.  frFlags tell FRemap which vertices on a face
			// should be "remapped", and the ww array contains the new locations.			
			VertColor nvc(0,0,0);
			int j;
			for (int v=0; v < d->GetNumColors(chan); v++) {
				ColorData cd;
				AlphaData ad;
				if (chan<2) {
					cd = d->GetColorData(v,chan);
					if (cd.color == 0xffffffff) continue;
				} else {
					ad = d->GetAlphaData (v);
					if (ad.alpha == 1) continue;
				}

				if(mesh->selLevel == MESH_VERTEX && !mesh->vertSel[v]) continue;

				// Blend it together with the VertexColor of the incoming mesh
				if (chan<2) {
					nvc = cd.bary * Color(cd.color);
					if (!mv || !mf || (cd.fi >= mesh->numFaces)) nvc += (1.0f-cd.bary)*UVVert(1,1,1);
					else nvc += (1.0f-cd.bary)*mv[mf[cd.fi].t[cd.vi]];
				} else {
					nvc.x = ad.bary * ad.alpha;
					if (!mv || !mf || (ad.fi >= mesh->numFaces)) nvc.x += 1.0f-ad.bary;
					else nvc.x += (1.0f-ad.bary)*mv[mf[ad.fi].t[ad.vi]].x;
					nvc.y = nvc.z = nvc.x;
				}

				DWORD ww[3], frFlags;
				md.Map(-chan).VCreate (&nvc);

				// increase the number of vcol's and set the vcfaces as well
				for (int i=0; i<d->GetNVert(v).faces.Count(); i++) {
					j = d->GetNVert(v).whichVertex[i];
					frFlags = (1<<j);
					ww[j] = md.Map(-chan).outVNum()-1;
					md.Map(-chan).FRemap(d->GetNVert(v).faces[i], frFlags, ww);
				}
			}
		} else {	// mesh.selLevel == MESH_FACE.
			// We used two routines -- VCreate to add new map vertices, and FRemap to make the
			// existing map faces use the new verts.  frFlags tell FRemap which vertices on a face
			// should be "remapped", and the ww array contains the new locations.			
			int j;
			ColorData cd;
			AlphaData ad;
			for (int v=0; v < d->GetNumColors(chan); v++) {
				VertColor nvc(0,0,0);
				if (chan<2) {
					cd = d->GetColorData (v,chan);
					if (cd.color == 0xffffffff) continue;
					if (!mesh->FaceSel()[cd.fi]) continue;
				} else {
					ad = d->GetAlphaData (v);
					if (ad.alpha == 1.0f) continue;
					if (!mesh->FaceSel()[ad.fi]) continue;
				}

				// Blend it together with the VertexColor of the incoming mesh
				if (chan<2) {
					nvc = cd.bary * Color(cd.color);
					if (mv && mf) nvc += (1.0f - cd.bary) * mv[mf[cd.fi].t[cd.vi]];
					else nvc += (1.0f - cd.bary) * UVVert(1,1,1);
				} else {
					nvc.x = ad.bary * ad.alpha;
					if (mv && mf) nvc.x += (1.0f - ad.bary) * mv[mf[ad.fi].t[ad.vi]].x;
					else nvc.x += 1.0f - ad.bary;
					nvc.y = nvc.z = nvc.x;
				}

				DWORD ww[3], frFlags;
				DWORD ivc;
				BOOL firstround = TRUE;
				BOOL fsel = FALSE;
				
				Tab<DWORD> selFaceTab;
				Tab<DWORD> unselFaceTab;
				Tab<DWORD> vcSelTab;
				Tab<DWORD> vcSelOnlyTab;
				
				BOOL NoFaceSelected = TRUE;
				
				// Check, if any of the faces is selected, if not continue
				for(int fc = 0; fc < d->GetNVert(v).faces.Count() && NoFaceSelected; fc++) {
					if(mesh->FaceSel()[d->GetNVert(v).faces[fc]])
						NoFaceSelected = FALSE;
				}
				if (NoFaceSelected) continue;

				fsel = TRUE;
				DWORD ivtx;
				BOOL SelFaceColsAllSame = TRUE;

				if (mf && mv) {
					// For all selected faces, that reference this vertex
					for(fc = 0; fc < d->GetNVert(v).faces.Count() && SelFaceColsAllSame ; fc++) {
						if(!mesh->FaceSel()[d->GetNVert(v).faces[fc]]) continue;
						int face = d->GetNVert(v).faces[fc];

						// get the vertex color of this face
						ivtx = mf ? mf[face].t[d->GetNVert(v).whichVertex[fc]] : 0;

						if(firstround) {
							// Just store the vertex color
							ivc = ivtx;
							firstround = FALSE;
						} else {
							// Are the vertex colors the same ?
							if(ivc != ivtx) {
								// No they are not.
								SelFaceColsAllSame = FALSE;
							}
						}
						vcSelTab.Append(1,&ivtx);
					}// End For all selected faces, that reference this vertex
				}

				if(SelFaceColsAllSame) {
					BOOL CreateNewColor = FALSE;

					// Check, if the ivc is referenced by any face, that is not selected
					// and shares the vertex, or does not share the vertex at all

					for(int vcfc = 0; vcfc < d->GetNVCVert(ivc,chan).faces.Count() && CreateNewColor == FALSE; vcfc++) {
						// Is one of faces that reference the physical vertex equal to the faces, 
						// that reference the vc vertex ?
						for(fc = 0; fc < d->GetNVert(v).faces.Count() && CreateNewColor == FALSE ; fc++) {
							if(d->GetNVert(v).faces[fc] == d->GetNVCVert(ivc,chan).faces[vcfc]) {
								if(!mesh->FaceSel()[d->GetNVCVert(ivc,chan).faces[vcfc]])
									CreateNewColor = TRUE;
							} else CreateNewColor = TRUE;
						}
					}
					if (!mv || !mf) CreateNewColor = TRUE;

					if(CreateNewColor)
					{
						// Create a new Color
						md.Map(-chan).VCreate (&nvc);

						// Assign the color of the selected faces to the new color
						for(fc = 0;  fc < d->GetNVert(v).faces.Count() ; fc++)
						{
							if(!mesh->FaceSel()[d->GetNVert(v).faces[fc]])
								continue;

							j = d->GetNVert(v).whichVertex[fc];
							frFlags = (1<<j);
							ww[j] = md.Map(-chan).outVNum()-1;
							md.Map(-chan).FRemap(d->GetNVert(v).faces[fc], frFlags, ww);
						}

					} else {	// OneUnselFaceHasSameColor
						// Set the vcolor to the new value
						md.Map(-chan).Set (ivtx, nvc);
					}
				}
				else // (SelFaceColsAllSame) Not all selected faces have the same color
				{
					// Collect all vcolors, that are only referenced by selected faces
					BOOL VColorIsReferencedByUnselectedFace = FALSE;
					
					// For all VColors, that are on selected faces
					for(int v2 = 0 ; v2 < vcSelTab.Count() ; v2++)
					{
						DWORD ivcv = vcSelTab[v2];
						// Check these vertex colors, if they are on any face, that 
						// is not selected
						for(int vfc = 0 ; vfc < d->GetNVCVert(ivcv, chan).faces.Count() && !VColorIsReferencedByUnselectedFace ; vfc++)
						{
							if(mesh->FaceSel()[d->GetNVCVert(ivcv, chan).faces[vfc]])
								VColorIsReferencedByUnselectedFace = TRUE;
						}
						if(!VColorIsReferencedByUnselectedFace)
							vcSelOnlyTab.Append(1,&ivcv);
					}

					if(vcSelOnlyTab.Count() > 0) {
						// Set the vcolor to the new value
						md.Map(-chan).Set (ivtx, nvc);

						for(fc = 0;  fc < d->GetNVert(v).faces.Count() ; fc++) {
							if(!mesh->FaceSel()[d->GetNVert(v).faces[fc]])
							continue;
							
							j = d->GetNVert(v).whichVertex[fc];
							frFlags = (1<<j);

							ww[j] = ivtx;
							md.Map(-chan).FRemap(d->GetNVert(v).faces[fc], frFlags, ww);
						}
					}
					else
					{
						// Create a new Color
						md.Map(-chan).VCreate (&nvc);
											
						// Assign the color of the selected faces to the new color
						for(fc = 0;  fc < d->GetNVert(v).faces.Count() ; fc++)
						{
							if(!mesh->FaceSel()[d->GetNVert(v).faces[fc]])
								continue;

							j = d->GetNVert(v).whichVertex[fc];
							frFlags = (1<<j);
							ww[j] = md.Map(-chan).outVNum()-1;

							md.Map(-chan).FRemap(d->GetNVert(v).faces[fc], frFlags, ww);
						}
					}
				}
			}
		}// end if(mesh->selLevel == MESH_FACE)
	}

	md.Apply(*mesh);
	// sca 5/1/01 - this line isn't needed!  It's handled by things like PartsChanged.
	//NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	tobj->UpdateValidity(VERT_COLOR_CHAN_NUM, Interval(t,t));
}

void VertexPaint::ModifyPolyObject(PolyObject* pPolyObj, TimeValue t, ModContext &mc)
{
	pPolyObj->ReadyChannelsForMod(GEOM_CHANNEL|TOPO_CHANNEL|VERTCOLOR_CHANNEL|TEXMAP_CHANNEL);
	MNMesh* mesh = &pPolyObj->GetMesh();
	if (!mesh) return;
	if (mesh->selLevel == MNM_SL_EDGE) return;
	if (mesh->MNum() < 1) mesh->SetMapNum (1);

	VertexPaintData *d  = (VertexPaintData*)mc.localData;
	if (!d) mc.localData = d = new VertexPaintData(pPolyObj->GetMesh());
	if (!d->GetMesh()) d->SetCache(*mesh);
	
	for (int chan=0; chan<3; chan++) {
		if (!d->ChannelActive(chan)) continue;
		// get the vertex color map
		MNMap* pVCMap = mesh->M(-chan);

		// initialize to an all white map if necessary
		bool initializedMap = false;
		if (pVCMap->GetFlag(MN_DEAD)) {
			mesh->InitMap(-chan);
			initializedMap = true;
		}

		ColorData cd;
		AlphaData ad;

		// If we're not in face SO mode, it's easier !
		if(mesh->selLevel == MNM_SL_OBJECT || mesh->selLevel == MNM_SL_VERTEX) {
			// set the color for each vertex according to the local mod data
			VertColor nvc(0,0,0);
			int j;
			for (int v=0; v < d->GetNumColors(chan); v++) {
				if (chan<2) {
					cd = d->GetColorData(v,chan);
					if (cd.color == 0xffffffff) continue;
				} else {
					ad = d->GetAlphaData (v);
					if (ad.alpha == 1) continue;
				}

				if(mesh->selLevel == MNM_SL_VERTEX && !mesh->V(v)->GetFlag(MN_SEL)) continue;

				// Blend it together with the VertexColor of the incoming mesh
				if (chan<2) {
					nvc = cd.bary * Color(cd.color);
					if (initializedMap || (cd.fi >= pVCMap->numf) || (cd.vi >= pVCMap->f[cd.fi].deg))
						nvc += (1.0f - cd.bary) * UVVert(1,1,1);
					else nvc += (1.0f-cd.bary)*(pVCMap->V(pVCMap->F(cd.fi)->tv[cd.vi]) );
				} else {
					nvc.x = ad.bary * ad.alpha;
					if (initializedMap || (ad.fi >= pVCMap->numf) || (ad.vi >= pVCMap->f[ad.fi].deg))
						nvc.x += 1.0f - ad.bary;
					else nvc.x += (1.0f-ad.bary)*(pVCMap->V(pVCMap->F(ad.fi)->tv[ad.vi])).x;
					nvc.y = nvc.z = nvc.x;
				}

				int newVCVert = pVCMap->NewVert(nvc);
				// increase the number of vcol's and set the vcfaces as well	
				for(int i = 0 ; i < d->GetNVert(v).faces.Count() ; i++) {
					j = d->GetNVert(v).whichVertex[i];
					MNMapFace* pVCFace = pVCMap->F(d->GetNVert(v).faces[i]);
					pVCFace->tv[j] = newVCVert;
				}
			} 
			pVCMap->CollapseDeadVerts (mesh->f);
		}

		else if(mesh->selLevel == MNM_SL_FACE) {
			int j;	
			for (int v=0; v < d->GetNumColors(-chan); v++) {
				VertColor nvc(0,0,0);
				if (chan<2) {
					cd = d->GetColorData(v, chan);
					if(  cd.color == 0xffffffff ) continue;
					if(!mesh->F(cd.fi)->GetFlag(MN_SEL)) continue;
				} else {
					ad = d->GetAlphaData (v);
					if (ad.alpha == 1) continue;
					if(!mesh->F(ad.fi)->GetFlag(MN_SEL)) continue;
				}

				// Blend it together with the VertexColor of the incoming mesh
				if (chan<2) {
					nvc = cd.bary * Color(cd.color);
					if (initializedMap) nvc += (1.0f - cd.bary) * UVVert(1,1,1);
					else nvc += (1.0f-cd.bary)*(pVCMap->V(pVCMap->F(cd.fi)->tv[cd.vi]) );
				} else {
					nvc.x = ad.bary * ad.alpha;
					if (initializedMap) nvc.x += 1.0f - ad.bary;
					else nvc.x += (1.0f-ad.bary)*(pVCMap->V(pVCMap->F(ad.fi)->tv[ad.vi])).x;
					nvc.y = nvc.z = nvc.x;
				}

				DWORD ivc;
				BOOL firstround = TRUE;
				BOOL fsel = FALSE;
				Tab<DWORD> selFaceTab;
				Tab<DWORD> unselFaceTab;
				Tab<int> vcSelTab;
				Tab<DWORD> vcSelOnlyTab;
				
				BOOL NoFaceSelected = TRUE;
				
				// Check, if any of the faces is selected, if not continue
				for(int fc = 0; fc < d->GetNVert(v).faces.Count() && NoFaceSelected; fc++) {		
					if(mesh->F(d->GetNVert(v).faces[fc])->GetFlag(MN_SEL)) 
						NoFaceSelected = FALSE;
				}
				if(NoFaceSelected) continue;		
				
				fsel = TRUE;
				int ivtx;
				BOOL SelFaceColsAllSame = TRUE;

				if (!initializedMap) {
					// For all selected faces, that reference this vertex
					for(fc = 0; fc < d->GetNVert(v).faces.Count() && SelFaceColsAllSame ; fc++)	{
						if(!mesh->F(d->GetNVert(v).faces[fc])->GetFlag(MN_SEL)) continue;
						int face = d->GetNVert(v).faces[fc];
						// get the vertex color of this face
						ivtx = pVCMap->F(face)->tv[d->GetNVert(v).whichVertex[fc]];
						if(firstround) {
							// Just store the vertex color
							ivc = ivtx;
							firstround = FALSE;
						}
						else {
							// Are the vertex colors the same ?
							if(ivc != ivtx)	{
								// No they are not.
								SelFaceColsAllSame = FALSE;
							}
						}
						vcSelTab.Append(1,&ivtx);
					}// End For all selected faces, that reference this vertex
				}

				if(SelFaceColsAllSame) {
					BOOL CreateNewColor = FALSE;
					// Check, if the ivc is referenced by any face, that is not selected
					// and shares the vertex, or does not share the vertex at all
					for(int vcfc = 0; vcfc < d->GetNVCVert(ivc, chan).faces.Count() && CreateNewColor == FALSE; vcfc++) {
						// Is one of faces that reference the physical vertex equal to the faces, 
						// that reference the vc vertex ?
						for(fc = 0; fc < d->GetNVert(v).faces.Count() && CreateNewColor == FALSE ; fc++) {
							if(d->GetNVert(v).faces[fc] == d->GetNVCVert(ivc, chan).faces[vcfc]) {
								if(!mesh->F(d->GetNVCVert(ivc,chan).faces[vcfc])->GetFlag(MN_SEL))
									CreateNewColor = TRUE;
							} else CreateNewColor = TRUE;
						}
					}

					if (initializedMap) CreateNewColor = TRUE;

					if(CreateNewColor) {
						// Create a new Color
						int newVCVert = pVCMap->NewVert(nvc);
						// Assign the color of the selected faces to the new color
						for(fc = 0;  fc < d->GetNVert(v).faces.Count() ; fc++) {
							if(!mesh->F(d->GetNVert(v).faces[fc])->GetFlag(MN_SEL))	continue;
							j = d->GetNVert(v).whichVertex[fc];
							MNMapFace* pVCFace = pVCMap->F(d->GetNVert(v).faces[fc]);
							pVCFace->tv[j] = newVCVert;
						}
					}
					else { // OneUnselFaceHasSameColor
						// Set the vcolor to the new value
						pVCMap->v[ivtx] = nvc;
					}
				}
				else { // (SelFaceColsAllSame) Not all selected faces have the same color
					// Collect all vcolors, that are only referenced by selected faces
					BOOL VColorIsReferencedByUnselectedFace = FALSE;
					// For all VColors, that are on selected faces
					for(int v2 = 0 ; v2 < vcSelTab.Count() ; v2++) {
						DWORD ivcv = vcSelTab[v2];
						// Check these vertex colors, if they are on any face, that 
						// is not selected
						for(int vfc = 0 ; vfc < d->GetNVCVert(ivcv, chan).faces.Count() && !VColorIsReferencedByUnselectedFace ; vfc++) {							
							if(mesh->F(d->GetNVCVert(ivcv, chan).faces[vfc])->GetFlag(MN_SEL))
								VColorIsReferencedByUnselectedFace = TRUE;		
						}
						if(!VColorIsReferencedByUnselectedFace)
							vcSelOnlyTab.Append(1,&ivcv);
					}
					if(vcSelOnlyTab.Count() > 0) {
						// Set the vcolor to the new value
						pVCMap->v[ivtx] = nvc;
						
						for(fc = 0;  fc < d->GetNVert(v).faces.Count() ; fc++) {
							if(!mesh->F(d->GetNVert(v).faces[fc])->GetFlag(MN_SEL))	continue;
							j = d->GetNVert(v).whichVertex[fc];
							MNMapFace* pVCFace = pVCMap->F(d->GetNVert(v).faces[fc]);
							pVCFace->tv[j] = ivtx;
						}
					}
					else {
						// Create a new Color
						int newVCVert = pVCMap->NewVert(nvc);
						// Assign the color of the selected faces to the new color
						for(fc = 0;  fc < d->GetNVert(v).faces.Count() ; fc++)
						{
							if(!mesh->F(d->GetNVert(v).faces[fc])->GetFlag(MN_SEL))	continue;
							j = d->GetNVert(v).whichVertex[fc];
							MNMapFace* pVCFace = pVCMap->F(d->GetNVert(v).faces[fc]);
							pVCFace->tv[j] = newVCVert;
						}
					}
				}
			}	
			pVCMap->CollapseDeadVerts (mesh->f);
		}// end if(mesh->selLevel == MESH_FACE)

	}

	// sca 5/1/01 - this line isn't needed!  It's handled by things like PartsChanged.
	//NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	pPolyObj->UpdateValidity(VERT_COLOR_CHAN_NUM, Interval(t,t));
}

void VertexPaint::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	
	
	// handle TriObjects
	if (os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *tobj = (TriObject*)os->obj;
		ModifyTriObject(tobj, t, mc);
	}
	// handle PolyObjects
	else if (os->obj->IsSubClassOf(polyObjectClassID)) {
		PolyObject *pPolyObj = (PolyObject*)os->obj;
		ModifyPolyObject(pPolyObj, t, mc);
	}
	// Convert to a tri mesh if possible
	else if(os->obj->CanConvertToType(triObjectClassID)) {
		TriObject  *triOb = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
		// Now stuff this into the pipeline!
		os->obj = triOb;

		ModifyTriObject(triOb, t, mc);
	}
}		

static bool oldShowEnd;

void VertexPaint::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	
	this->ip = ip;
	editMod = this;
	if (!hParams) {
		hParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_PANEL),
				VertexPaintDlgProc, 
				GetString(IDS_PARAMS), 
				(LPARAM)this);

		// Subclass the palette controls
		hPaletteWnd[ 0] = GetDlgItem(hParams, IDC_PALETTE_1);
		hPaletteWnd[ 1] = GetDlgItem(hParams, IDC_PALETTE_2);
		hPaletteWnd[ 2] = GetDlgItem(hParams, IDC_PALETTE_3);
		hPaletteWnd[ 3] = GetDlgItem(hParams, IDC_PALETTE_4);
		hPaletteWnd[ 4] = GetDlgItem(hParams, IDC_PALETTE_5);
		hPaletteWnd[ 5] = GetDlgItem(hParams, IDC_PALETTE_6);
		hPaletteWnd[ 6] = GetDlgItem(hParams, IDC_PALETTE_7);
		hPaletteWnd[ 7] = GetDlgItem(hParams, IDC_PALETTE_8);
		hPaletteWnd[ 8] = GetDlgItem(hParams, IDC_PALETTE_9);
		hPaletteWnd[ 9] = GetDlgItem(hParams, IDC_PALETTE_10);
		hPaletteWnd[10] = GetDlgItem(hParams, IDC_PALETTE_11);
		hPaletteWnd[11] = GetDlgItem(hParams, IDC_PALETTE_12);
		hPaletteWnd[12] = GetDlgItem(hParams, IDC_PALETTE_13);
		hPaletteWnd[13] = GetDlgItem(hParams, IDC_PALETTE_14);
		hPaletteWnd[14] = GetDlgItem(hParams, IDC_PALETTE_15);
		hPaletteWnd[15] = GetDlgItem(hParams, IDC_PALETTE_16);

		for (int i=0; i<NUMPALETTES; i++) {
			colorSwatchOriginalWndProc = (WNDPROC) SetWindowLongPtr(hPaletteWnd[i], GWLP_WNDPROC, (LONG_PTR) colorSwatchSubclassWndProc); 
			}

		SendMessage(hParams, WM_POSTINIT, 0, 0);
		}
	else {
		SetWindowLongPtr(hParams,GWLP_USERDATA,(LONG_PTR)this);
		}
	iTint = SetupIntSpinner (hParams, IDC_TINT_SPIN, IDC_TINT, 0, 100, (int) (fTint*100.0f));

	// Set show end result.
	oldShowEnd = ip->GetShowEndResult() ? TRUE : FALSE;
	ip->SetShowEndResult (GetFlag (VP_DISP_END_RESULT));

	// Force an eval to update caches.
	NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	}

void VertexPaint::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
	{
	ActivatePaint(FALSE);
	
	ReleaseISpinner (iTint);
	
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list,nodes);
	for (int i=0; i<list.Count(); i++) {
		VertexPaintData *vd = (VertexPaintData*)list[i]->localData;
		if (vd) vd->FreeCache();
	}
	nodes.DisposeTemporary();

	// Reset show end result
	SetFlag (VP_DISP_END_RESULT, ip->GetShowEndResult() ? TRUE : FALSE);
	ip->SetShowEndResult(oldShowEnd);


	ip->DeleteRollupPage(hParams);
	hParams = NULL;
	editMod = NULL;
	iTint = NULL;
	this->ip = NULL;
	}


//From ReferenceMaker 
RefResult VertexPaint::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
	{
	return REF_SUCCEED;
	}

int VertexPaint::NumRefs() 
	{
	return 0;
	}

RefTargetHandle VertexPaint::GetReference(int i) 
	{
	return NULL;
	}

void VertexPaint::SetReference(int i, RefTargetHandle rtarg)
	{
	}

int VertexPaint::NumSubs() 
	{ 
	return 0;
	}  

Animatable* VertexPaint::SubAnim(int i) 
	{ 
	return NULL; 
	}

TSTR VertexPaint::SubAnimName(int i) 
	{ 
	return _T("");
	}


#define VERSION_CHUNKID			0x100
#define AFFECT_COLOR 0x0130
#define AFFECT_ILLUM 0x0134
#define AFFECT_ALPHA 0x0138
#define ALPHA_VALUE 0x013c

static int currentVersion = 1;

IOResult VertexPaint::Load(ILoad *iload)
	{
	IOResult res;
	ULONG nb;
	int version = 1;
	Modifier::Load(iload);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case VERSION_CHUNKID:
			iload->Read (&version, sizeof(version), &nb);
			break;
		case AFFECT_COLOR:
			iload->Read (&(affectChannel[0]), sizeof(bool), &nb);
			break;
		case AFFECT_ILLUM:
			iload->Read (&(affectChannel[1]), sizeof(bool), &nb);
			break;
		case AFFECT_ALPHA:
			iload->Read (&(affectChannel[2]), sizeof(bool), &nb);
			break;
		case ALPHA_VALUE:
			iload->Read (&alpha, sizeof(float), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}

	return IO_OK;
	}

IOResult VertexPaint::Save(ISave *isave)
	{
	IOResult res;
	ULONG nb;

	Modifier::Save(isave);

	isave->BeginChunk(VERSION_CHUNKID);
	res = isave->Write (&currentVersion, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk (AFFECT_COLOR);
	res = isave->Write (&(affectChannel[0]), sizeof(bool), &nb);
	isave->EndChunk ();

	isave->BeginChunk (AFFECT_ILLUM);
	res = isave->Write (&(affectChannel[1]), sizeof(bool), &nb);
	isave->EndChunk ();

	isave->BeginChunk (AFFECT_ALPHA);
	res = isave->Write (&(affectChannel[2]), sizeof(bool), &nb);
	isave->EndChunk ();

	isave->BeginChunk (ALPHA_VALUE);
	res = isave->Write (&alpha, sizeof(float), &nb);
	isave->EndChunk ();

	return IO_OK;
	}

#define COLORLIST_CHUNKID	0x120
#define ILLUMLIST_CHUNKID	0x124
#define ALPHALIST_CHUNKID	0x128

IOResult VertexPaint::SaveLocalData(ISave *isave, LocalModData *ld) {
	VertexPaintData*	d = (VertexPaintData*)ld;
	IOResult	res;
	ULONG		nb;
	int			numColors;
	int			maxNumColors;
	AlphaData   alph;
	ColorData	col;
	int i, j;
	
	isave->BeginChunk(VERSION_CHUNKID);
	res = isave->Write (&currentVersion, sizeof(int), &nb);
	isave->EndChunk();

	for (j=0; j<2; j++) {
		if (!d->ChannelActive(j)) continue;
		if (j==0) isave->BeginChunk (COLORLIST_CHUNKID);
		else isave->BeginChunk (ILLUMLIST_CHUNKID);
		numColors = d->GetNumColors(j);
		res = isave->Write(&numColors, sizeof(int), &nb);
		maxNumColors = d->GetMaxNumColors(j);
		res = isave->Write(&maxNumColors, sizeof(int), &nb);
		for (i=0; i<maxNumColors; i++) {
			col = d->GetColorData(i, j);
			isave->Write(&col.color,sizeof(col.color),&nb);
			isave->Write(&col.bary,sizeof(col.bary),&nb);
			isave->Write(&col.fi,sizeof(col.fi),&nb);
			isave->Write(&col.vi,sizeof(col.vi),&nb);
		}
		isave->EndChunk();
	}

	if (d->ChannelActive(2)) {
		isave->BeginChunk (ALPHALIST_CHUNKID);
		numColors = d->GetNumColors (2);
		res = isave->Write (&numColors, sizeof(int), &nb);
		maxNumColors = d->GetMaxNumColors (j);
		res = isave->Write (&maxNumColors, sizeof(int), &nb);
		for (i=0; i<maxNumColors; i++) {
			alph = d->GetAlphaData (i);
			isave->Write (&alph.alpha, sizeof(alph.alpha), &nb);
			isave->Write (&alph.bary, sizeof(alph.bary), &nb);
			isave->Write (&alph.fi, sizeof(alph.fi), &nb);
			isave->Write (&alph.vi, sizeof(alph.vi), &nb);
		}
		isave->EndChunk();
	}
	return IO_OK;
}

IOResult VertexPaint::LoadLocalData(ILoad *iload, LocalModData **pld) {
	VertexPaintData *d = new VertexPaintData;
	IOResult	res;	
	ULONG		nb;
	int			version = 1;
	int			numColors;
	int			maxNumColors;
	ColorData	col;
	AlphaData   alph;
	int i;

	*pld = d;

	while (IO_OK==(res=iload->OpenChunk())) {
		int channel = 0;
		switch(iload->CurChunkID())  {
		case VERSION_CHUNKID:
			iload->Read (&version, sizeof(version), &nb);
			break;
		case ILLUMLIST_CHUNKID:
			channel = 1;
		case COLORLIST_CHUNKID:
			d->SetChannelActive(channel);
			iload->Read(&numColors,sizeof(int), &nb);
			iload->Read(&maxNumColors,sizeof(int), &nb);
			d->AllocColorData(maxNumColors, channel);
			for (i=0; i<maxNumColors; i++) {
				iload->Read(&col.color,sizeof(col.color), &nb);
				iload->Read(&col.bary,sizeof(col.bary), &nb);
				iload->Read(&col.fi,sizeof(col.fi), &nb);
				iload->Read(&col.vi,sizeof(col.vi), &nb);
				d->SetColor(i, col.bary, col.fi,col.vi,col.color, channel);
			}
			d->AllocColorData(numColors, channel);
			break;
		case ALPHALIST_CHUNKID:
			d->SetChannelActive(2);
			iload->Read(&numColors,sizeof(int), &nb);
			iload->Read(&maxNumColors,sizeof(int), &nb);
			d->AllocColorData(maxNumColors, 2);
			for (i=0; i<maxNumColors; i++) {
				iload->Read(&alph.alpha,sizeof(alph.alpha), &nb);
				iload->Read(&alph.bary,sizeof(alph.bary), &nb);
				iload->Read(&alph.fi,sizeof(alph.fi), &nb);
				iload->Read(&alph.vi,sizeof(alph.vi), &nb);
				d->SetAlpha (i, alph.bary, alph.fi, alph.vi, alph.alpha);
			}
			d->AllocColorData(numColors, 2);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

void VertexPaint::PaletteButton(HWND hWnd) {
	IColorSwatch* iPal = GetIColorSwatch(hWnd);
	if (iPal && iColor) {
		iColor->SetColor(iPal->GetColor(), TRUE);
	}
}

void VertexPaint::InitPalettes()
	{
	IColorSwatch* c;
	for (int i=0; i<NUMPALETTES; i++) {
		c = GetIColorSwatch(hPaletteWnd[i]);
		c->SetColor(palColors[i]);
		ReleaseIColorSwatch(c);
		}
	}

void VertexPaint::SavePalettes()
	{
	IColorSwatch* c;
	for (int i=0; i<NUMPALETTES; i++) {
		c = GetIColorSwatch(hPaletteWnd[i]);
		palColors[i] = c->GetColor();
		ReleaseIColorSwatch(c);
		}
	}

void VertexPaint::TurnVCOn(BOOL shaded)
{
	ModContextList list;
	INodeTab NodeTab;
	
	// Only the selected nodes will be affected
	ip->GetModContexts(list,NodeTab);

	for( int i = 0 ; i < NodeTab.Count() ; i++)
	{
		if(shaded)
			NodeTab[i]->SetShadeCVerts(!NodeTab[i]->GetShadeCVerts());
		else
			NodeTab[i]->SetCVertMode(!NodeTab[i]->GetCVertMode());	
		
	}
	NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

void VertexPaint::SetActiveAlpha (float alphaValue) {
	alpha = alphaValue;
	ISpinnerControl *spin = GetISpinner (GetDlgItem (hParams, IDC_ALPHA_SPIN));
	if (spin) {
		spin->SetValue (alphaValue*100.0f, FALSE);
		ReleaseISpinner (spin);
	}
}

VertexPaintData::VertexPaintData(Mesh& m) : mesh(NULL), mpMNMesh(NULL), colordata(NULL),
nverts(NULL), numnverts(0), illumdata(NULL), alphadata(NULL)
{
	for (int i=0; i<3; i++) {
		numColors[i] = 0;
		numnvcverts[i] = 0;
		maxNumColors[i] = 0;
		nvcverts[i] = NULL;
		channelActive[i] = false;
	}
	SetCache(m);
}

VertexPaintData::VertexPaintData(MNMesh& m) : mesh(NULL), mpMNMesh(NULL), colordata(NULL),
nverts(NULL), numnverts(0), illumdata(NULL), alphadata(NULL)
{
	for (int i=0; i<3; i++) {
		numColors[i] = 0;
		numnvcverts[i] = 0;
		maxNumColors[i] = 0;
		nvcverts[i] = NULL;
		channelActive[i] = false;
	}
	SetCache(m);
}

VertexPaintData::VertexPaintData() : mesh(NULL), mpMNMesh(NULL), colordata(NULL),
nverts(NULL), numnverts(0), illumdata(NULL), alphadata(NULL)
{
	for (int i=0; i<3; i++) {
		numColors[i] = 0;
		numnvcverts[i] = 0;
		maxNumColors[i] = 0;
		nvcverts[i] = NULL;
		channelActive[i] = false;
	}
}

VertexPaintData::~VertexPaintData() {
	FreeCache();

	if (colordata) delete [] colordata;
	if(nverts) delete [] nverts;
	for (int i=0; i<3; i++) {
		if(nvcverts[i]) delete [] nvcverts[i];
		nvcverts[i] = NULL;
		maxNumColors[i] = 0;
		numColors[i] = 0;
		numnvcverts[i] = 0;
	}

	nverts = NULL;
	colordata = NULL;
	numnverts = 0;
}

void VertexPaintData::SetCache(Mesh& m) {
	FreeCache();
	mesh = new Mesh(m);
	SynchVerts(m);
	for (int i=0; i<3; i++) {
		AllocColorData(mesh->getNumVerts(), i);
	}
}

void VertexPaintData::SetCache(MNMesh& m) {
	FreeCache();
	mpMNMesh = new MNMesh(m);
	SynchVerts(m);
	for (int i=0; i<3; i++) {
		AllocColorData(mpMNMesh->VNum(), i);
	}
}

void VertexPaintData::FreeCache() {
	if (mesh) delete mesh;
	if (mpMNMesh) delete mpMNMesh;
	if(nverts) delete [] nverts;
	mesh = NULL;
	mpMNMesh = NULL;
	nverts = NULL;
	numnverts = 0;
	for (int i=0; i<3; i++) {
		if (nvcverts[i]) delete [] nvcverts[i];
		nvcverts[i] = NULL;
		numnvcverts[i] = 0;
	}
}

Mesh* VertexPaintData::GetMesh() {
	return mesh;
}

MNMesh* VertexPaintData::GetMNMesh() {
	return mpMNMesh;
}

NVert&  VertexPaintData::GetNVert(int i)
{
	static NVert nv;
	
	if (numnverts > i)
		return nverts[i];
	else
		return nv;
}

NVert& VertexPaintData::GetNVCVert(int i, int chan) {
	static NVert nv;
	
	if (numnvcverts[chan] > i) return nvcverts[chan][i];
	else return nv;
}

COLORREF& VertexPaintData::GetColor(int i, int chan) {
	static COLORREF c = RGB(1,1,1);
	if (maxNumColors[chan] > i) return chan ? illumdata[i].color : colordata[i].color;
	else return c;
}

ColorData& VertexPaintData::GetColorData(int i, int chan) {
	static ColorData c;
	if (maxNumColors[chan] > i) return chan ? illumdata[i] : colordata[i];
	else return c;
}

float VertexPaintData::GetAlpha (int i) {
	static float a(1.0f);
	if (maxNumColors[2] > i) return alphadata[i].alpha;
	else return a;
}

AlphaData& VertexPaintData::GetAlphaData (int i) {
	static AlphaData a;
	if (maxNumColors[2] > i) return alphadata[i];
	else return a;
}

void VertexPaintData::SetColor(int i, float bary, DWORD fi, int vi, COLORREF c, int chan)
{
	if (((chan==0) && !colordata) || ((chan==1) && !illumdata) || maxNumColors[chan] <= i) return;

	ColorData & cd = GetColorData (i, chan);
	if(cd.color != 0xffffffff) {
		// This color was set before !
		float oldbary = cd.bary;
		float zeta = (1.0f-bary);
		float weight;
		if(bary+(zeta*oldbary) > 0) weight = 1/(bary+(zeta*oldbary));
		else weight = 0;
		cd.color = (weight*zeta*oldbary*Color(cd.color) + weight*bary*Color(c)).toRGB();
		cd.bary = zeta*oldbary+bary;
	} else {
		cd.color = c;
		cd.bary = bary;
	}
	cd.fi = fi;
	cd.vi = vi;
	channelActive[chan] = true;
}

void VertexPaintData::SetAlpha (int i, float bary, DWORD fi, int vi, float a)
{
	if (!alphadata || maxNumColors[2] <= i) return;

	AlphaData & ad = GetAlphaData (i);
	if(ad.alpha != 1.0f) {
		// This color was set before !
		float oldbary = ad.bary;
		float zeta = (1.0f-bary);
		float weight;
		if(bary+(zeta*oldbary) > 0) weight = 1/(bary+(zeta*oldbary));
		else weight = 0;
		ad.alpha = weight*zeta*oldbary*ad.alpha + weight*bary*a;
		ad.bary = zeta*oldbary+bary;
	} else {
		ad.alpha = a;
		ad.bary = bary;
	}
	ad.fi = fi;
	ad.vi = vi;
	channelActive[2] = true;
}

int VertexPaintData::GetNumColors(int chan) {
	return numColors[chan];
}

int VertexPaintData::GetMaxNumColors(int chan) {
	return maxNumColors[chan];
}

void VertexPaintData::AllocColorData(int numcols, int chan) {
	ColorData* newColorData;
	AlphaData *newAlphaData;

	// Colors already exist.
	if (numColors[chan] == numcols) return;

	if (numColors[chan] == 0) {
		// Allocate a complete new set of colors
		numColors[chan] = numcols;
		maxNumColors[chan] = numcols;
		switch (chan) {
		case 0:
			colordata = new ColorData[numcols];
			break;
		case 1:
			illumdata = new ColorData[numcols];
			break;
		case 2:
			alphadata = new AlphaData[numcols];
			break;
		}
		return;
	}

	// If the new number of colors is bigger than what we have in the colordata array
	if (numcols > maxNumColors[chan]) {
		// Allocate a new color list and fill in as many as
		// we have from the previous set
		if (chan<2) newColorData = new ColorData[numcols];
		else newAlphaData = new AlphaData[numcols];

		for (int i=0; i<maxNumColors[chan]; i++) {
			switch (chan) {
			case 0:
				newColorData[i] = colordata[i];
				break;
			case 1:
				newColorData[i] = illumdata[i];
				break;
			case 2:
				newAlphaData[i] = alphadata[i];
				break;
			}
			DWORD & fi = (chan<2) ? newColorData[i].fi : newAlphaData[i].fi;
			int & vi = (chan<2) ? newColorData[i].vi : newAlphaData[i].vi;

			if (mesh || mpMNMesh) {
				int numOfFaces = mesh? mesh->getNumFaces(): mpMNMesh->FNum();
				if (fi >= (DWORD) numOfFaces) {
					BOOL found = FALSE;
					for(int j = 0 ; j < nverts[i].faces.Count() && !found; j++) {
						if(nverts[i].faces[j] < numOfFaces) {
							fi = nverts[i].faces[j];
							vi = nverts[i].whichVertex[j];
						}
					}
					if (!found) vi = 0;
				}
			}
		}
		switch (chan) {
		case 0:
			delete [] colordata;
			colordata = newColorData;
			break;
		case 1:
			delete [] illumdata;
			illumdata = newColorData;
			break;
		case 2:
			delete [] alphadata;
			alphadata = newAlphaData;
			break;
		}
		maxNumColors[chan] = numColors[chan] = numcols;

	} else {
		// If the new number of colors is smaller than what we have in the colordata array
		if (mesh || mpMNMesh) {
			int numOfFaces = mesh? mesh->getNumFaces(): mpMNMesh->FNum();

			for (int i=0; i<numcols; i++) {
				DWORD & fi = (chan<2) ? (chan ? illumdata[i].fi : colordata[i].fi) : alphadata[i].fi;
				int & vi = (chan<2) ? (chan ? illumdata[i].vi : colordata[i].vi) : alphadata[i].vi;

				if(fi >= (DWORD)numOfFaces) {
					BOOL found = FALSE;
					for(int j = 0 ; j < nverts[i].faces.Count() && !found; j++) {
						if(nverts[i].faces[j] < numOfFaces) {
							fi = nverts[i].faces[j];
							vi = nverts[i].whichVertex[j];
						}
					}
					if (!found) vi = 0;
				}
			}
		}
		numColors[chan] = numcols;
	}
}

LocalModData* VertexPaintData::Clone()
	{
	VertexPaintData* d = new VertexPaintData();
	int i, j;

	if (colordata) {
		d->colordata = new ColorData[maxNumColors[0]];
		d->numColors[0] = numColors[0];
		d->maxNumColors[0] = maxNumColors[0];
		for (i=0; i<maxNumColors[0]; i++) d->colordata[i] = colordata[i];
	}
	if (illumdata) {
		d->illumdata = new ColorData[maxNumColors[1]];
		d->numColors[1] = numColors[1];
		d->maxNumColors[1] = maxNumColors[1];
		for (i=0; i<maxNumColors[1]; i++) d->illumdata[i] = illumdata[i];
	}
	if (alphadata) {
		d->alphadata = new AlphaData[maxNumColors[2]];
		d->numColors[2] = numColors[2];
		d->maxNumColors[2] = maxNumColors[2];
		for (i=0; i<maxNumColors[2]; i++) d->alphadata[i] = alphadata[i];
	}
	if(nverts) {
		d->nverts = new NVert[numnverts];
		for (i=0 ; i < numnverts ; i++ ) d->nverts[i] = nverts[i];
		d->numnverts = numnverts;
	}

	for (j=0; j<3; j++) {
		if(nvcverts[j]) {
			d->nvcverts[j] = new NVert[numnvcverts[j]];
			for(i=0 ; i<numnvcverts[j]; i++ ) d->nvcverts[j][i] = nvcverts[j][i];
			d->numnvcverts[j] = numnvcverts[j];
		}
		d->channelActive[j] = channelActive[j];
	}

	return d;
}

void VertexPaintData::SynchVerts(Mesh &m) {
	int i, j, chan;

	if (nverts) delete [] nverts;
	numnverts = m.getNumVerts();
	nverts = new NVert[numnverts];

	for (chan=0; chan<3; chan++) {
		if (nvcverts[chan]) delete [] nvcverts[chan];
		numnvcverts[chan] = m.getNumMapVerts (-chan);
		nvcverts[chan] = new NVert[numnvcverts[chan]];
	}

	for (i = 0 ; i < mesh->getNumFaces() ; i++) {	
		// for each vertex of each face
		for (j = 0 ; j < 3 ; j++) {		
			int iCur = nverts[mesh->faces[i].v[j]].faces.Count();

			// Tell the vertex, which to which face it belongs and which 
			// of the three face v-indices corresponds to the vertex
			nverts[mesh->faces[i].v[j]].faces.SetCount(iCur+1);
			nverts[mesh->faces[i].v[j]].whichVertex.SetCount(iCur+1);
			nverts[mesh->faces[i].v[j]].faces[iCur] = i;
			nverts[mesh->faces[i].v[j]].whichVertex[iCur] = j;

			// Do the same for vertex color maps:
			for (chan=0; chan<3; chan++) {
				TVFace *mf = mesh->mapFaces(-chan);
				if (!mf) continue;

				NVert & nvcv = nvcverts[chan][mf[i].t[j]];
				iCur = nvcv.faces.Count();
				nvcv.faces.SetCount(iCur+1);
				nvcv.whichVertex.SetCount(iCur+1);
				nvcv.faces[iCur] = i;
				nvcv.whichVertex[iCur] = j;
			}
		}
	}
}

void VertexPaintData::SynchVerts(MNMesh &m) {
	int i, j, chan;
	if(nverts) delete [] nverts;
	numnverts = m.VNum();
	nverts = new NVert[numnverts];

	for (chan=0; chan<3; chan++) {
		if (nvcverts[chan]) delete [] nvcverts[chan];
		numnvcverts[chan] = m.M(-chan)->VNum();
		nvcverts[chan] = new NVert[numnvcverts[chan]];
	}

	for (i=0; i < mpMNMesh->FNum() ; i++){	
		// for each vertex of each face
		for (j=0; j < mpMNMesh->F(i)->deg ; j++){		
			int iCur = nverts[mpMNMesh->F(i)->vtx[j]].faces.Count();
			// Tell the vertex, which to which face it belongs and which 
			// of the face v-indices corresponds to the vertex
			nverts[mpMNMesh->F(i)->vtx[j]].faces.SetCount(iCur+1);
			nverts[mpMNMesh->F(i)->vtx[j]].whichVertex.SetCount(iCur+1);
			nverts[mpMNMesh->F(i)->vtx[j]].faces[iCur] = i;
			nverts[mpMNMesh->F(i)->vtx[j]].whichVertex[iCur] = j;

			// Do the same for the vertex color maps:
			for (chan=0; chan<3; chan++) {
				MNMap* pVCMesh = mpMNMesh->M(-chan);
				if (!pVCMesh || pVCMesh->GetFlag(MN_DEAD)) continue;

				NVert & nvcv = nvcverts[chan][pVCMesh->F(i)->tv[j]];
				iCur = nvcv.faces.Count();
				nvcv.faces.SetCount(iCur+1);
				nvcv.whichVertex.SetCount(iCur+1);
				nvcv.faces[iCur] = i;
				nvcv.whichVertex[iCur] = j;
			}
		}
	}
}

//***************************************************************************
//**
//** NVert
//**
//***************************************************************************

NVert::NVert() {
	faces.SetCount(0);
	whichVertex.SetCount(0);
}

NVert& NVert::operator= (NVert &nvert) {
	faces = nvert.faces;
	whichVertex = nvert.whichVertex;
	return *this;
}

//***************************************************************************
//**
//** ColorData, AlphaData
//**
//***************************************************************************

ColorData::ColorData(DWORD col) : color(col), bary(0.0f), fi(0), vi(0)
{
}

ColorData::ColorData() : color(0xffffffff), bary(0.0f), fi(0), vi(0)
{
}

AlphaData::AlphaData (float alph) : alpha(alph),bary(0.0f),fi(0),vi(0)
{
}

AlphaData::AlphaData () : alpha(1.0f),bary(0.0f),fi(0),vi(0)
{
}

//***************************************************************************
//**
//** VertexPaintColorRestore : public RestoreObj
//**
//***************************************************************************

VertexPaintColorRestore::VertexPaintColorRestore(VertexPaintData *pLocalData, VertexPaint *pVPaint, int chan) 
: pMod(pVPaint), pPaintData(pLocalData), redoColordata(NULL), channel(chan)
{
	colordata = new ColorData[pPaintData->maxNumColors[channel]];
	ColorData *pLocalColData = channel ? pPaintData->illumdata : pPaintData->colordata;
	for (int i = 0; i < pPaintData->maxNumColors[channel]; i++) colordata[i] = pLocalColData[i];
	numcolors = pPaintData->numColors[channel];
	maxnumcolors = pPaintData->maxNumColors[channel];
}

VertexPaintColorRestore::~VertexPaintColorRestore() {
	if(colordata) delete [] colordata;
	if(redoColordata) delete [] redoColordata;
}

void VertexPaintColorRestore::Restore(int isUndo) {
	if(!isUndo) return;
	assert(pPaintData->colordata);

	ColorData **pLocalColData = channel ? &(pPaintData->illumdata) : &(pPaintData->colordata);
	redoColordata = *pLocalColData;
	redonumcolors = pPaintData->numColors[channel];
	redomaxnumcolors = pPaintData->maxNumColors[channel];

	*pLocalColData = colordata;
	pPaintData->numColors[channel] = numcolors;
	pPaintData->maxNumColors[channel] = maxnumcolors;

	colordata = NULL;

	pMod->NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void VertexPaintColorRestore::Redo()
{
	assert(pPaintData->colordata);
	
	ColorData **pLocalColData = channel ? &(pPaintData->illumdata) : &(pPaintData->colordata);

	colordata = *pLocalColData;
	numcolors = pPaintData->numColors[channel];
	maxnumcolors = pPaintData->maxNumColors[channel];
	
	*pLocalColData = redoColordata;
	pPaintData->numColors[channel] = redonumcolors;
	pPaintData->maxNumColors[channel] = redomaxnumcolors;

	redoColordata = NULL;

	pMod->NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

int  VertexPaintColorRestore::Size()
{
	int iSize = 0;
	if(colordata) iSize += sizeof(ColorData) * maxnumcolors;
	if(redoColordata) iSize += sizeof(ColorData) * redonumcolors;
	return iSize;
}

//***************************************************************************
//**
//** VertexPaintAlphaRestore : public RestoreObj
//**
//***************************************************************************

VertexPaintAlphaRestore::VertexPaintAlphaRestore(VertexPaintData *pLocalData, VertexPaint *pVPaint) 
: pMod(pVPaint), pPaintData(pLocalData), redoAlphadata(NULL)
{
	alphadata = new AlphaData[pPaintData->maxNumColors[2]];
	for (int i = 0; i < pPaintData->maxNumColors[2]; i++) alphadata[i] = pPaintData->alphadata[i];
	numcolors = pPaintData->numColors[2];
	maxnumcolors = pPaintData->maxNumColors[2];
}

VertexPaintAlphaRestore::~VertexPaintAlphaRestore() {
	if(alphadata) delete [] alphadata;
	if(redoAlphadata) delete [] redoAlphadata;
}

void VertexPaintAlphaRestore::Restore(int isUndo) {
	if(!isUndo) return;
	DbgAssert(pPaintData->alphadata);

	redoAlphadata = pPaintData->alphadata;
	redonumcolors = pPaintData->numColors[2];
	redomaxnumcolors = pPaintData->maxNumColors[2];

	pPaintData->alphadata = alphadata;
	pPaintData->numColors[2] = numcolors;
	pPaintData->maxNumColors[2] = maxnumcolors;

	alphadata = NULL;

	pMod->NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void VertexPaintAlphaRestore::Redo()
{
	assert(pPaintData->colordata);
	
	alphadata = pPaintData->alphadata;
	numcolors = pPaintData->numColors[2];
	maxnumcolors = pPaintData->maxNumColors[2];

	pPaintData->alphadata = redoAlphadata;
	pPaintData->numColors[2] = redonumcolors;
	pPaintData->maxNumColors[2] = redomaxnumcolors;

	redoAlphadata = NULL;

	pMod->NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

int  VertexPaintAlphaRestore::Size()
{
	int iSize = 0;
	if(alphadata) iSize += sizeof(AlphaData) * maxnumcolors;
	if(redoAlphadata) iSize += sizeof(AlphaData) * redonumcolors;
	return iSize;
}

