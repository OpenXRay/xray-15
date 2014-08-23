/**********************************************************************
 *<
	FILE: PainterTextureSample.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "PainterTextureSample.h"




static PainterTextureSampleClassDesc PainterTextureSampleDesc;
ClassDesc2* GetPainterTextureSampleDesc() { return &PainterTextureSampleDesc; }





static ParamBlockDesc2 paintertexturesample_param_blk ( paintertexturesample_params, _T("params"),  0, &PainterTextureSampleDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_bitmap, _T("bitmap"),	TYPE_BITMAP, 0, IDS_PW_BITMAP,
		p_ui,			TYPE_BITMAPBUTTON, IDC_BITMAP,
		end,

	pb_color,	 _T("color"),	TYPE_RGBA,				0,	IDS_PW_COLOR,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_COLOR, 
		end,

	pb_coords,			_T("coords"),		TYPE_REFTARG,	P_OWNERS_REF,	IDS_COORDS,
		p_refno,		COORD_REF, 
		end,



	end
	);



ParamDlg* PainterTextureSample::uvGenDlg;

//--- PainterTextureSample -------------------------------------------------------
PainterTextureSample::PainterTextureSample()
{
	for (int i=0; i<NSUBTEX; i++) subtex[i] = NULL;
	//TODO: Add all the initializing stuff
	pblock = NULL;
	PainterTextureSampleDesc.MakeAutoParamBlocks(this);
	uvGen = NULL;
	iPaintButton = NULL;
	pPainter = NULL;
	texHandle = NULL;
	Reset();
	bm = NULL;
	width = 0;
}

PainterTextureSample::~PainterTextureSample()
{
	ReleaseICustButton(iPaintButton);
	iPaintButton = NULL;
	if (pPainter)
		{
		if (pPainter->InPaintMode() )
			pPainter->EndPaintSession();
		}
	DiscardTexHandle();
}

//From MtlBase
void PainterTextureSample::Reset() 
{
	if (uvGen) uvGen->Reset();
	else ReplaceReference( 0, GetNewDefaultUVGen());	
	//TODO: Reset texmap back to its default values

	ivalid.SetEmpty();

}

void PainterTextureSample::Update(TimeValue t, Interval& valid) 
{	
	//TODO: Add code to evaluate anything prior to rendering
}

Interval PainterTextureSample::Validity(TimeValue t)
{
	//TODO: Update ivalid here
	return ivalid;
}

ParamDlg* PainterTextureSample::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	IAutoMParamDlg* masterDlg = PainterTextureSampleDesc.CreateParamDlgs(hwMtlEdit, imp, this);
	uvGenDlg = uvGen->CreateParamDlg(hwMtlEdit, imp);
	masterDlg->AddDlg(uvGenDlg);

	paintertexturesample_param_blk.SetUserDlgProc(new PaintTextureTestDlgProc(this));

	//TODO: Set the user dialog proc of the param block, and do other initialization	


	return masterDlg;	
}

BOOL PainterTextureSample::SetDlgThing(ParamDlg* dlg)
{	
	if (dlg == uvGenDlg)
		uvGenDlg->SetThing(uvGen);
	else 
		return FALSE;
	return TRUE;
}

void PainterTextureSample::SetSubTexmap(int i, Texmap *m) 
{
	ReplaceReference(i+2,m);
	//TODO Store the 'i-th' sub-texmap managed by the texture
}

TSTR PainterTextureSample::GetSubTexmapSlotName(int i) 
{	
	//TODO: Return the slot name of the 'i-th' sub-texmap
	return TSTR(_T(""));
}


//From ReferenceMaker
RefTargetHandle PainterTextureSample::GetReference(int i) 
{
	//TODO: Return the references based on the index	
	switch (i) {
		case 0: return uvGen;
		case 1: return pblock;
		default: return subtex[i-2];
		}
}

void PainterTextureSample::SetReference(int i, RefTargetHandle rtarg) 
{
	//TODO: Store the reference handle passed into its 'i-th' reference
	switch(i) {
		case 0: uvGen = (UVGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
		default: subtex[i-2] = (Texmap *)rtarg; break;
	}
}

//From ReferenceTarget 
RefTargetHandle PainterTextureSample::Clone(RemapDir &remap) 
{
	PainterTextureSample *mnew = new PainterTextureSample();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	//TODO: Add other cloning stuff
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

	 
Animatable* PainterTextureSample::SubAnim(int i) 
{
	//TODO: Return 'i-th' sub-anim
	switch (i) {
		case 0: return uvGen;
		case 1: return pblock;
		default: return subtex[i-2];
		}
}

TSTR PainterTextureSample::SubAnimName(int i) 
{
	//TODO: Return the sub-anim names
	switch (i) {
		case 0: return GetString(IDS_COORDS);		
		case 1: return GetString(IDS_PARAMS);
		default: return GetSubTexmapTVName(i-1);
		}
}

RefResult PainterTextureSample::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) 
{
	//TODO: Handle the reference changed messages here	
	return(REF_SUCCEED);
}

IOResult PainterTextureSample::Save(ISave *isave) 
{
	//TODO: Add code to allow plugin to save its data
	return IO_OK;
}

IOResult PainterTextureSample::Load(ILoad *iload) 
{ 
	//TODO: Add code to allow plugin to load its data
	return IO_OK;
}

AColor PainterTextureSample::EvalColor(ShadeContext& sc)
{
	//TODO: Evaluate the color of texture map for the context.
	return AColor (0.0f,0.0f,0.0f,0.0f);
}

float PainterTextureSample::EvalMono(ShadeContext& sc)
{
	//TODO: Evaluate the map for a "mono" channel
	return Intens(EvalColor(sc));
}

Point3 PainterTextureSample::EvalNormalPerturb(ShadeContext& sc)
{
	//TODO: Return the perturbation to apply to a normal for bump mapping
	return Point3(0, 0, 0);
}

ULONG PainterTextureSample::LocalRequirements(int subMtlNum)
{
	//TODO: Specify various requirements for the material
	return uvGen->Requirements(subMtlNum); 
}

void PainterTextureSample::DiscardTexHandle() 
	{
	if (texHandle) 
		{
		texHandle->DeleteThis();
		texHandle = NULL;
		}
	}

void PainterTextureSample::ActivateTexDisplay(BOOL onoff)
{
	if (!onoff) 
		DiscardTexHandle();
}

BITMAPINFO* PainterTextureSample::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH) 
	{
//	Bitmap *bm;
	Interval v;
	Update(t,v);
	int size = thmaker.Size();
	bm = BuildBitmap(size);
	BITMAPINFO *bmi = thmaker.BitmapToDIB(bm,uvGen->SymFlags(),0,forceW,forceH);
//	bm->DeleteThis();
	valid.SetInfinite();

	return bmi;
	}


DWORD PainterTextureSample::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker)
	{
	if (texHandle) 
		{
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}

inline UWORD FlToWord(float r) {
	return (UWORD)(65535.0f*r);
	}


Bitmap *PainterTextureSample::BuildBitmap(int size) 
	{
	float u,v;
	BitmapInfo bi;
	bi.SetName(_T("checkerTemp"));
	bi.SetWidth(size);
	bi.SetHeight(size);
	bi.SetType(BMM_TRUE_32);
	if (bm == NULL)
		{
		bm = TheManager->Create(&bi);
		if (bm==NULL) return NULL;
		PixelBuf l64(size);
		float d = 1.0f/float(size);
		v = 1.0f - 0.5f*d;
		for (int y=0; y<size; y++) {
			BMM_Color_64 *p64=l64.Ptr();
			u = 0.0f;
			for (int x=0; x<size; x++, p64++) {
				AColor c(0.0f,0.0f,0.0f) ;
				p64->r = FlToWord(c.r); 
				p64->g = FlToWord(c.g); 
				p64->b = FlToWord(c.b);
				p64->a = 0xffff; 
				u += d;
				}
			bm->PutPixels(0,y, size, l64.Ptr()); 
			v -= d;
			}
		}
	width = size;
	return bm;
	}




void  PainterTextureSample::InitUI(HWND hWnd)
	{
	hwnd = hWnd;
	if (iPaintButton) ReleaseICustButton(iPaintButton);
	iPaintButton = NULL;

	iPaintButton = GetICustButton(GetDlgItem(hWnd,IDC_PAINT));
	iPaintButton->SetType(CBT_CHECK);
	iPaintButton->SetHighlightColor(GREEN_WASH);

	pPainter = NULL;

	ReferenceTarget *painterRef = (ReferenceTarget *) GetCOREInterface()->CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID);
	
//set it to the correct verion
	if (painterRef)
		{
		pPainter = (IPainterInterface_V5 *) painterRef->GetInterface(PAINTERINTERFACE_V5);
		}

	}
void  PainterTextureSample::DestroyUI(HWND hWnd)
	{
	if (iPaintButton) ReleaseICustButton(iPaintButton);
	iPaintButton = NULL;
	if (pPainter)
		{
		if (pPainter->InPaintMode() )
			pPainter->EndPaintSession();
		}
	}


void PainterTextureSample::PaintOptions()
{
	if (pPainter) //need to check this since some one could have taken the painterinterface plugin out
		{
		pPainter->BringUpOptions();
		}

}


//Some helper functions to handle painter UI
void PainterTextureSample::Paint()
{
	node = NULL;
//	pblock->GetValue(pb_node,0,node,FOREVER);

	int nodeCount =  GetCOREInterface()->GetSelNodeCount();
	if (nodeCount > 0)
		node = GetCOREInterface()->GetSelNode(0);

	if ((pPainter) && (node))//need to check this since some one could have taken the painterinterface plugin out
		{
		if (!pPainter->InPaintMode())  
			{
			pPainter->InitializeCallback(this); //initialize the callback

	//we dont use the point gather or normal data
			pPainter->SetEnablePointGather(TRUE);
			pPainter->SetBuildNormalData(FALSE);

//this sends all our dependant nodes to the painter
			Tab<INode*> nodes;
			nodes.Append(1,&node);
			pPainter->InitializeNodes(0, nodes);

		//since we are painting on the final mesh we dont need to do any special handling of nodes

			pPainter->StartPaintSession(); //start the paint session
			iPaintButton->SetCheck(TRUE);
			

//need to build mesh and uvw data 
			ObjectState sos;
			TimeValue t = GetCOREInterface()->GetTime();
			if (node)
				{
				sos = node->EvalWorldState(t);
		
				Mesh *msh = NULL;
				TriObject *collapsedtobj = NULL;

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

						int uvCount = msh->getNumMapVerts(1);
						int numFaces = msh->numFaces;
						uvwPoints.SetCount(uvCount);
						uvwFaces.SetCount(numFaces);
						memcpy(uvwPoints.Addr(0), msh->mapVerts(1),uvCount*sizeof(Point3));
						memcpy(uvwFaces.Addr(0), msh->mapFaces(1),numFaces*sizeof(TVFace));
						Matrix3 tm = node->GetObjectTM(t);
						BuildWorldSpaceData(msh,tm);
						int ct = worldSpaceList.Count();
						pPainter->LoadCustomPointGather(ct, worldSpaceList.Addr(0), node);
						pPainter->UpdateMeshes(TRUE);
						}
				

					if ((collapsedtobj) && (collapsedtobj != sos.obj)) collapsedtobj->DeleteThis();

					}
				}
			}
		else //we are currently in a paint mode so turn it off
			{
			pPainter->EndPaintSession(); //end the paint session
			iPaintButton->SetCheck(FALSE);
			}

		}


}

//This thing is ugly should really do scan conversion instead of barycentric
void  PainterTextureSample::BuildWorldSpaceData(Mesh *msh, Matrix3 tm)
{
//loop through each face
	Tab<Point3> tempWorldPointList;

	tempWorldPointList.SetCount(msh->numVerts);
	for (int i = 0; i < msh->numVerts; i++)
		{
		tempWorldPointList[i] = msh->verts[i] * tm;
		}

	worldSpaceList.ZeroCount();
	uvwList.ZeroCount();

	for (i = 0; i < msh->numFaces; i++)
		{

		TSTR name;
		name.printf("Processing Face %d/%d",i,msh->numFaces-1);
		SetWindowText(GetDlgItem(hwnd,IDC_TEXT),name);
		
//gets its world space position corners
		Point3 t[3];
		Point3 w[3];
		

		w[0] = tempWorldPointList[msh->faces[i].v[0]];
		w[1] = tempWorldPointList[msh->faces[i].v[1]];
		w[2] = tempWorldPointList[msh->faces[i].v[2]];


		t[0] = uvwPoints[uvwFaces[i].t[0]];
		t[1] = uvwPoints[uvwFaces[i].t[1]];
		t[2] = uvwPoints[uvwFaces[i].t[2]];
	
		t[0].z = 0.0f;
		t[1].z = 0.0f;
		t[2].z = 0.0f;

//scan it from bitmap space to world space
		int minX,maxX,minY,maxY;
		minX = floor(t[0].x*width);
		maxX = ceil(t[0].x*width);
		minY = floor(t[0].y*width);
		maxY = ceil(t[0].y*width);
		for (int j = 1; j < 3; j++)
			{
			int tempMinX, tempMaxX;
			int tempMinY, tempMaxY;
			tempMinX = floor(t[j].x*width);
			tempMaxX = ceil(t[j].x*width);
			tempMinY = floor(t[j].y*width);
			tempMaxY = ceil(t[j].y*width);

			if (tempMinX < minX) minX = tempMinX;
			if (tempMinY < minY) minY = tempMinY;

			if (tempMaxX > maxX) maxX = tempMaxX;
			if (tempMaxY > maxY) maxY = tempMaxY;

			}
//more ugliness below
		float baryMin = 0.0 - (20.0f/width);
		float baryMax = 1.0f+ (20.0f/width);
		minY--;
		minX--;
		maxY++;
		maxX++;

		for (j =minY; j <= maxY; j++)
			{
			for (int k =minX; k <= maxX; k++)
				{
				Point3 hitPoint;
				hitPoint.x = (float)k/(float)width;
				hitPoint.y = (float)j/(float)width;
				hitPoint.z = 0.0f;
				Point3 baryCoord = BaryCoords(t[0], t[1], t[2], hitPoint);
//check if inside a triangle

				if (baryCoord.x >= baryMin && baryCoord.x <= baryMax) 
					{
					if (baryCoord.y >= baryMin && baryCoord.y <= baryMax) 
						{
						if (baryCoord.z >= baryMin && baryCoord.z <= baryMax) 
							{
							//we are inside 
							Point3 wp = w[0] * baryCoord.x +
							 	 w[1] * baryCoord.y +
								 w[2] * baryCoord.z;


							worldSpaceList.Append(1,&wp,5000);
							uvwList.Append(1,&hitPoint,5000);
							}
						}
					}
				}
			}

		}
		TSTR name;
		name.printf("  ");
		SetWindowText(GetDlgItem(hwnd,IDC_TEXT),name);
	//FIX we can dump alot of this stuff for instance the worldSpace list
	//the uvwPoints and the UVW faces




}

void* PainterTextureSample::GetInterface(ULONG id)
{
	switch(id)
	{
		case PAINTERCANVASINTERFACE_V5 : return (IPainterCanvasInterface_V5 *) this;
			break;
		default: return Texmap::GetInterface(id);
			break;
	}
}

