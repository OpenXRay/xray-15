/**********************************************************************
 *<
	FILE: VERTCOL.CPP

	DESCRIPTION: Return interpolated vertex color

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "buildver.h"
#include "iparamm2.h"

#ifndef NO_MAPTYPE_VERTCOLOR // orb 01-03-2001 Removing map types

extern HINSTANCE hInstance;

static LRESULT CALLBACK CurveWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );



static Class_ID vcolClassID(VCOL_CLASS_ID,0);



class NameData
{
public:
	int channel,subid;
};


//--------------------------------------------------------------
// VCol: A Composite texture map
//--------------------------------------------------------------
class VCol: public Texmap { 

	Interval ivalid;
	BOOL useUVW;
	public:
		VCol();
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		ULONG Requirements(int subMtlNum) 
			{ 
			if (mapID == 0)
				return MTLREQ_UV2;
			else return MTLREQ_UV; 
			}
		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  
			if ((mapID >= 0) && (mapID < mapreq.GetSize()))
				mapreq.Set(mapID);
			}			
		void Update(TimeValue t, Interval& valid);
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }
		void NotifyChanged();

		// Evaluate the color of map for the context.
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		Class_ID ClassID() {	return vcolClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_VCOL); }  
		void DeleteThis() { delete this; }	

//pblock2
		int NumSubs() { return 1; }  

		// From ref
 		int NumRefs() { return 1; }

// JBW: direct ParamBlock access is added
		IParamBlock2 *pblock;   // ref #1
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg) {return FALSE;}

		RefTargetHandle GetReference(int i) 
		{
			return pblock ;
		}
		void SetReference(int i, RefTargetHandle rtarg) 
		{
			pblock = (IParamBlock2 *)rtarg; 
		}
		Animatable* SubAnim(int i) 
		{
			return pblock;
		}


		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		bool IsLocalOutputMeaningful( ShadeContext& sc );

		int mapID;
		int subID;
		HWND hWnd;
		Tab<NameData> channelData;
		void FilloutNames(HWND hWnd);
	};

class VColClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new VCol; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_VCOL_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return vcolClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_COLMOD;  }

// PW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("VertexColor"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};
	
class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	  int count;
	};
	
int MyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
//7-1-99
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
		Nodes.Append(1, (INode **)&rmaker);  
		count++;
		}


	return 0;
	}
		

static VColClassDesc vcolCD;

ClassDesc* GetVColDesc() { return &vcolCD;  }

// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { vertexcolor_params, };  // pblock ID
// checker_params param IDs
enum 
{ 
	vertexcolor_map,vertexcolor_subid,
	
	  // access for UVW mapping

};

class VColPBAccessor : public PBAccessor
{ 
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		VCol* p = (VCol*)owner;
		switch (id)
		{
			case vertexcolor_map:
			case vertexcolor_subid:
			{
				if (p->hWnd)
					p->FilloutNames(p->hWnd);
				break;

			}

		}
	}
};

static VColPBAccessor vcol_accessor;

static ParamBlockDesc2 vertexcolor_param_blk ( vertexcolor_params, _T("parameters"),  0, &vcolCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_VCOL1, IDS_DS_VCOLPARAMS, 0, 0, NULL, 
	// params
	vertexcolor_map,	_T("map"),   TYPE_INT,			0 ,	IDS_MAP,
		p_default,		0,
		p_range,		0, 99,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT,  IDC_MAP_EDIT, IDC_MAP_SPIN, 0.01f,
		p_accessor,		&vcol_accessor,		
		end,
	vertexcolor_subid, 			_T("subid"), 		TYPE_INT, 	0, 	IDS_SUBID, 
		p_default, 		0, 
		p_ui, TYPE_INTLISTBOX, IDC_SUBIDCOMBO, 4, IDS_ALL, IDS_RED,IDS_GREEN,IDS_BLUE,
		p_accessor,		&vcol_accessor,		
		end,
	
	end
);

//dialog stuff to get the Set Ref button
class VColDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		VCol *tex;		
		VColDlgProc(VCol *m) {tex = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void SetThing(ReferenceTarget *m) {
			tex = (VCol*)m;
//			ReloadDialog();
			}

	};



BOOL VColDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: 
			tex->hWnd = hWnd;
			tex->FilloutNames(hWnd);
			break;			
		case WM_DESTROY:
			tex->hWnd = NULL;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_UPDATE:				
					{
					tex->FilloutNames(hWnd);
					break;
					}
				case IDC_NAMECOMBO:
					{
					if (HIWORD(wParam)==CBN_SELCHANGE)
						{
						int fsel;
						fsel = SendMessage(
							GetDlgItem(hWnd,IDC_NAMECOMBO),
							CB_GETCURSEL,0,0);	
						if ((fsel >=1)  && (fsel < tex->channelData.Count()))
							{
							int mapID, subID;
							HWND h = tex->hWnd;
							tex->hWnd = NULL;
							mapID = tex->channelData[fsel].channel;
							subID = tex->channelData[fsel].subid+1;
							tex->pblock->SetValue(vertexcolor_map,0,mapID);
							tex->pblock->SetValue(vertexcolor_subid,0,subID);
							tex->hWnd = h;
							}
						}					
					break;
					}
				}
			break;
		}
	return FALSE;
	}


//-----------------------------------------------------------------------------
//  VCol
//-----------------------------------------------------------------------------

#define VCOL_VERSION 1


void VCol::Reset() {
	useUVW = FALSE;
	ivalid.SetEmpty();
	}

void VCol::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

VCol::VCol() {
	mapID = 0;
	subID= 0;
	hWnd = NULL;
	vcolCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Reset();
	}

bool VCol::IsLocalOutputMeaningful( ShadeContext& sc ) 
{ 
    // A Node instance is needed to evaluate the vert colors
    // at a certain point on a face of the object.
    // One can get to the mesh from the tri obj from the node OR
    // from ShadeContext::RenderGlobalContext.GetRenderInstance
    // [dl | 13feb2002] Changed this 'if' condition from an OR to and AND.
    // This ensures that the function will return true if it can get the tri
    // object either from the node or the RenderGlobalContext.
    if ( sc.NodeID() < 0 &&
            sc.globContext == NULL )
        return false;
    
    return true; 
}

AColor VCol::EvalColor(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
//	Point3 p = sc.UVW(1-useUVW);
	Point3 p = sc.UVW(mapID);

	if (subID == 0)
	{
		p.y = p.x;
		p.z = p.x;
	}
	else if (subID == 1)
	{
		p.x = p.y;
		p.z = p.y;
	}
	else if (subID == 2)
	{
		p.y = p.z;
		p.x = p.z;

	}

	return AColor(p.x,p.y,p.z,1.0f);
	}

float VCol::EvalMono(ShadeContext& sc) {
	return Intens(EvalColor(sc));
	}

Point3 VCol::EvalNormalPerturb(ShadeContext& sc) {
	return Point3(0,0,0);
	}

RefTargetHandle VCol::Clone(RemapDir &remap) {
	VCol *mnew = new VCol();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff

	mnew->ReplaceReference(0,remap.CloneRef(pblock));

	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* VCol::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	// create the rollout dialogs
	IAutoMParamDlg* masterDlg = vcolCD.CreateParamDlgs(hwMtlEdit, imp, this);
//attach a dlg proc to handle the swap button 
	vertexcolor_param_blk.SetUserDlgProc(new VColDlgProc(this));

	return masterDlg;
	}


void VCol::Update(TimeValue t, Interval& valid) {		
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		}


	pblock->GetValue( vertexcolor_map, t, mapID, ivalid );
	pblock->GetValue( vertexcolor_subid, t, subID, ivalid );	
	subID--;

	valid &= ivalid;
	}


RefResult VCol::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
//pblock2
		case REFMSG_CHANGE:
			{
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
//				if (hTarget != uvGen  && hTarget != pblock ) 
					vertexcolor_param_blk.InvalidateUI(changing_param);
				}
			break;
			}
/*
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
//			if (paramDlg&&!paramDlg->isActive) 
			if (paramDlg) 
					paramDlg->Invalidate();
			break;
*/
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
//			gpn->name= TSTR(GetString(name_id[gpn->index]));
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define USE_UVW_CHUNK 0x5000

IOResult VCol::Save(ISave *isave) { 
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();
	if (useUVW) {
		isave->BeginChunk(USE_UVW_CHUNK);
		isave->EndChunk();
		}
	return IO_OK;
	}	
	  

IOResult VCol::Load(ILoad *iload) { 
//	ULONG nb;
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
//			case USE_UVW_CHUNK:
//				useUVW = TRUE;
//				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

void VCol::FilloutNames(HWND hWnd)
{
	//get the node
	MyEnumProc dep;              
	EnumDependents(&dep);

	channelData.ZeroCount();

	SendMessage(GetDlgItem(hWnd,IDC_NAMECOMBO),	CB_RESETCONTENT ,0,0);


	int sel = 0;

	int mapID;
	int subID;
	TimeValue t = GetCOREInterface()->GetTime();
		
	pblock->GetValue( vertexcolor_map, t, mapID, ivalid );
	pblock->GetValue( vertexcolor_subid, t, subID, ivalid );	
	subID--;
	

	if (dep.Nodes.Count() > 0)
		{
		INode *node = dep.Nodes[0];

	//loop through our channels
		TSTR blank(" ");
		SendMessage(GetDlgItem(hWnd,IDC_NAMECOMBO),	CB_ADDSTRING,0,(LPARAM)(TCHAR*)blank);		
		NameData data;
		data.channel = -1;
		data.subid = -1;						
		channelData.Append(1,&data,10);
		
		int ct = 0;

	//add them to our channel list and the UI
		for (int i = 0; i < 99; i++)
			{
			for (int j = -1; j < 3; j++)
				{
				TSTR key, name;
				
				if (j == -1)
					key.printf("MapChannel:%d",i);
				else key.printf("MapChannel:%d(%d)",i,j);
				if (node->UserPropExists(key))
					{
					node->GetUserPropString(key,name);

					if (name.Length() > 0)
						{
						if (j == -1)
							name.printf("%d:All : %s",i,name);
						else if (j == 0)
							name.printf("%d:Red : %s",i,name);						
						else if (j == 1)
							name.printf("%d:Green : %s",i,name);						
						else if (j == 2)
							name.printf("%d:Blue : %s",i,name);						
						NameData data;
						data.channel = i;
						data.subid = j;
						SendMessage(GetDlgItem(hWnd,IDC_NAMECOMBO),	CB_ADDSTRING,0,(LPARAM)(TCHAR*)name);						
						channelData.Append(1,&data,10);
						ct++;
						if ((mapID == i) && (subID == j))
							sel = ct;
						
						}

					}
				}
			}
		}
	SendMessage(GetDlgItem(hWnd,IDC_NAMECOMBO), CB_SETCURSEL  ,sel,0);	

}

#endif // NO_MAPTYPE_VERTCOLOR