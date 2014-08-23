/**********************************************************************
 *<
	FILE: ViewportLoader.cpp

	DESCRIPTION:	Viewport Manager for loading up Effects

	CREATED BY:		Neil Hazzard

	HISTORY:		Created:  02/15/02
					

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#include "d3dx9.h"
#include "ViewportManager.h"
#include "ViewportLoader.h"


class ViewportLoaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new ViewportLoader;}
	const TCHAR *	ClassName() { return GetString(IDS_VPMCLASSNAME); }
	SClass_ID		SuperClassID() {return CUST_ATTRIB_CLASS_ID;}
	Class_ID 		ClassID() {return VIEWPORTLOADER_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	const TCHAR*	InternalName() { return _T("ViewportManager"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static ViewportLoaderClassDesc theViewportLoaderClassDesc;
ClassDesc2* GetViewportLoaderDesc(){ return &theViewportLoaderClassDesc;}


class EffectPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		ViewportLoader* m = (ViewportLoader*)owner;
		IParamMap2 * map = m->pblock->GetMap();
		ICustAttribContainer * cc = NULL;
		//Lets find our Parent - which should be a custom Attribute Container
		RefList &list = m->GetRefList();
		RefListItem *ptr = list.first;
		while (ptr) 
		{
			if (ptr->maker) 
			{
				if (ptr->maker->ClassID()==Class_ID(0x5ddb3626, 0x23b708db) ) 
				{
					cc = (ICustAttribContainer*)ptr->maker;
				}
			}
			ptr = ptr->next;
		}

		bool hasParent = true;
		MtlBase * mb = NULL;
		if(cc)
			mb = (MtlBase *) cc->GetOwner();
		else
			hasParent = false;

		switch (id)
		{
			case pb_effect: {
				if(v.i == m->effectNum)
					break;

				ClassDesc* pCD = m->GetEffectCD((v.i)-1);
				BOOL state;
				m->pblock->GetValue(pb_enabled,t,state,FOREVER);

				if(pCD==NULL)
				{
					if(map)
						map->Enable(pb_enabled,FALSE);
					if(hasParent)
						mb->ClearMtlFlag(MTL_HW_MAT_ENABLED);
				}
				else
				{
					if(map)
						map->Enable(pb_enabled,TRUE);
					if(state && hasParent)
						mb->SetMtlFlag(MTL_HW_MAT_ENABLED);
				}
				m->effectNum = v.i;
				m->LoadEffect(pCD); 
				GetCOREInterface()->RedrawViews(true);
				break;
			}
			case pb_enabled:
				{
					if((m->effect == NULL && v.i == 1)||!hasParent)
					{
						v.i = 0;
					}
//					IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
//					MtlBase *mtl = mtlEdit->GetCurMtl();
					MtlBase *mtl = mb;
					// The following forces the GFX to copy our Viewportdata and not from the standard material data
					if(hasParent)
					{
						if(v.i)
							mtl->SetMtlFlag(MTL_HW_MAT_ENABLED);
						else
							mtl->ClearMtlFlag(MTL_HW_MAT_ENABLED);
						
						GetCOREInterface()->RedrawViews(true);
					}
				}
				break;
		}
	}

};

static EffectPBAccessor effectPBAccessor;


BOOL EffectsDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			//disable the interface for Non DirectX cases
/*			if(!CheckForDX())
			{
				HWND dlg;
				dlg = GetDlgItem(hWnd,IDC_EFFECTLIST);
				EnableWindow(dlg,false);
				dlg = GetDlgItem(hWnd,IDC_ENABLED);
				EnableWindow(dlg,false);

			}
*/
			HWND hwndeffect = GetDlgItem(hWnd, IDC_EFFECTLIST);
			SendMessage(hwndeffect, CB_RESETCONTENT, 0L, 0L);
			for (int i = 0; i < vl->NumShaders(); i++) {
				ClassDesc* pClassD = vl->GetEffectCD(i);
				int n = SendMessage(hwndeffect, CB_ADDSTRING, 0L, (LPARAM)(pClassD->ClassName()) );
			}
			SendMessage(hwndeffect, CB_INSERTSTRING, 0L, (LPARAM)GetString(IDS_STR_NONE));
			SendMessage(hwndeffect, CB_SETCURSEL, vl->effectNum,0L);
			if(vl->effectNum == 0)
				map->Enable(pb_enabled,FALSE);
			else
				map->Enable(pb_enabled,TRUE);
			return TRUE;
		}
	}
	return FALSE;
}




static ParamBlockDesc2 param_blk ( viewport_manager_params, _T("DirectX Manager"),  0, &theViewportLoaderClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_CUSTATTRIB, IDS_VIEWPORT_MANAGER, 0, 0, NULL, 
	// params

	pb_enabled, 	_T("enabled"),		TYPE_BOOL, 	0, 	IDS_ENABLED, 
		p_default, 		FALSE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_ENABLED,
		p_accessor,		&effectPBAccessor,
		end,

	pb_effect,		_T("effect"),	TYPE_INT,		0, 	IDS_EFFECT, 	
		p_default, 		0, 
		p_ui, 			TYPE_INTLISTBOX, IDC_EFFECTLIST, 0,
		p_accessor,		&effectPBAccessor,
		end,

	end
);


static FPInterfaceDesc viewport_manager_interface(
    VIEWPORT_SHADER_MANAGER_INTERFACE, _T("viewportmanager"), 0, &theViewportLoaderClassDesc, 0,

		IViewportShaderManager::get_num_effects,		_T("getNumViewportEffects"),		0, TYPE_INT, 0, 0,
		IViewportShaderManager::get_active_effect,		_T("getActiveViewportEffect"),		0, TYPE_REFTARG, 0, 0,
		IViewportShaderManager::set_effect,				_T("setViewportEffect"),			0, TYPE_REFTARG,0,1,
			_T("effectindex"),	0,	TYPE_INT,
		IViewportShaderManager::get_effect_name,		_T("getViewportEffectName"),		0,TYPE_STRING,0,1,
			_T("effectindex"), 0, TYPE_INT,
		IViewportShaderManager::activate_effect,		_T("activateEffect"),				0,0,0,2,
			_T("material"), 0, TYPE_MTL,
			_T("state"),0,TYPE_BOOL,

			
	end
);


//  Get Descriptor method for Mixin Interface
//  *****************************************
FPInterfaceDesc* IViewportShaderManager::GetDesc()
{
     return &viewport_manager_interface;
}


ViewportLoader::ViewportLoader()
{
	theViewportLoaderClassDesc.MakeAutoParamBlocks(this); 
	effectIndex = NOT_FOUND;
	LoadEffectsList();
	masterDlg = NULL;
	clientDlg = NULL;
	effect = NULL;
	effectNum = 0;
	mEdit =  NULL;
	mparam = NULL;
	oldEffect = NULL;
	undo = false;
}

ViewportLoader::~ViewportLoader()
{
}

ReferenceTarget *ViewportLoader::Clone(RemapDir &remap)
{
	ViewportLoader *pnew = new ViewportLoader;
	pnew->ReplaceReference(0,pblock->Clone(remap));
	if(effect)
		pnew->ReplaceReference(1,effect->Clone(remap));
	pnew->effectNum = effectNum;
	BaseClone(this, pnew, remap);
	return pnew;
}


ParamDlg *ViewportLoader::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
{

	mEdit = hwMtlEdit;
	mparam = imp;;
	masterDlg =  theViewportLoaderClassDesc.CreateParamDlgs(hwMtlEdit, imp, this);
	param_blk.SetUserDlgProc(new EffectsDlgProc(this));


	if(effect){
		IDXDataBridge * vp = (IDXDataBridge*)effect->GetInterface(VIEWPORT_SHADER_CLIENT_INTERFACE);
		if(vp)
		{
			clientDlg = vp->CreateEffectDlg(hwMtlEdit,imp);
			masterDlg->AddDlg(clientDlg);
		}
	}

//	int n;
//	pblock->GetValue(pb_effect,0, n, FOREVER);
//	if( n != effectNum )
//		pblock->SetValue(pb_effect,0,effectNum);

	return masterDlg;
}

void ViewportLoader::SetReference(int i, RefTargetHandle rtarg) 
{
	switch(i)
	{
		case PBLOCK_REF: 
			pblock = (IParamBlock2 *)rtarg;
			break;
		case 1:
			effect = (ReferenceTarget *)rtarg;
			break;
	}
}

RefTargetHandle ViewportLoader::GetReference(int i)
{
	switch(i)
	{
		case PBLOCK_REF: 
			return pblock;
		case 1:
			return effect;

		default: return NULL;
	}
}	

RefResult ViewportLoader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,PartID &partID, RefMessage message)
{
	return REF_SUCCEED;
}


int ViewportLoader::NumSubs()  
{
	return 1;
}

Animatable* ViewportLoader::SubAnim(int i)
{
	if(effect)
	{
		IParamBlock2 * pblock2 =  effect->GetParamBlock(0);
		return pblock2; 
	}

	return NULL;
}

TSTR ViewportLoader::SubAnimName(int i)
{
	// we use the data from the Actual shader
	if(effect)
	{
		IDXDataBridge * vp = (IDXDataBridge*)effect->GetInterface(VIEWPORT_SHADER_CLIENT_INTERFACE);
		TCHAR * name = vp->GetName();
		return TSTR(name);

	}
	else
		return TSTR(_T(""));
}

SvGraphNodeReference ViewportLoader::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags) 
{ 
	// Only continue traversal if an effect is present and active
	if( effect && GetDXShaderManager()->IsVisible())
		return SvStdTraverseAnimGraph(gom, owner, id, flags); 
	else
		return SvGraphNodeReference();
}

#define MANAGER_ACTIVENUM_CHUNK	0x1000
#define MANAGER_ENABLED_CHUNK	0x1001

IOResult ViewportLoader::Save(ISave *isave)
{
	ULONG nb;
	isave->BeginChunk(MANAGER_ACTIVENUM_CHUNK);
	isave->Write(&effectNum, sizeof(effectNum), &nb);			
	isave->EndChunk();
	return IO_OK;
}


// this is used in the case that the file is being opened on a system with more effects
// it will try to find the effect from the new list, otherwise it will force a "None" and no UI
class PatchEffect : public PostLoadCallback 
{
public:
	ViewportLoader*	v;
	PatchEffect(ViewportLoader* pv){ v = pv;}
	void proc(ILoad *iload)
	{
		v->effectNum = v->FindEffectIndex(v->effect);
		v->pblock->SetValue(pb_effect,0,v->effectNum);
		delete this;

	}
};


IOResult ViewportLoader::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;
	int id;
	PatchEffect* pe = new PatchEffect(this);
	iload->RegisterPostLoadCallback(pe);
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {

			case MANAGER_ACTIVENUM_CHUNK:
				res = iload->Read(&effectNum, sizeof(effectNum), &nb);
				break;

		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}




// compare function for sorting Shader Tab<>
static int classDescListCompare(const void *elem1, const void *elem2) 
{
	ClassDesc* s1 = *(ClassDesc**)elem1;
	ClassDesc* s2 = *(ClassDesc**)elem2;
	TSTR sn1 = s1->ClassName();  // need to snap name string, since both use GetString()
	TSTR sn2 = s2->ClassName();
	return _tcscmp(sn1.data(), sn2.data());
}



ClassDesc * ViewportLoader::FindandLoadDeferedEffect(ClassDesc * defered)
{
	SubClassList* scList = GetCOREInterface()->GetDllDir().ClassDir().GetClassList(REF_TARGET_CLASS_ID);
	for (long i = 0, j = 0; i < scList->Count(ACC_ALL); ++i) {
		if ( (*scList)[ i ].IsPublic() && ((*scList)[ i ].ClassID() == defered->ClassID()) ) {

			ClassDesc* pClassD = (*scList)[ i ].FullCD();
			LoadEffectsList();
			return pClassD;
		}
	}
	return NULL;

}

#define D3D9_GRAPHICS_WINDOW_INTERFACE_ID Interface_ID(0x56424386, 0x2151b83)

void ViewportLoader::LoadEffectsList()
{
	// loads static shader list with name-sorted Shader ClassDesc*'s
	bool bdx9 = false;

	ViewExp *vpt;
	vpt = GetCOREInterface()->GetActiveViewport();	
	GraphicsWindow *gw = vpt->getGW();

	if(gw->GetInterface(D3D9_GRAPHICS_WINDOW_INTERFACE_ID))
	{
		bdx9 = true;
	}

	effectsList.ZeroCount();
	SubClassList* scList = GetCOREInterface()->GetDllDir().ClassDir().GetClassList(REF_TARGET_CLASS_ID);
	theHold.Suspend(); // LAM - 3/24/03 - defect 446356 - doing a DeleteThis on created effects, need to make sure hold is off
	for (long i = 0, j = 0; i < scList->Count(ACC_ALL); ++i) {
		if ( (*scList)[ i ].IsPublic() ) {
			ClassDesc* pClassD = (*scList)[ i ].FullCD();
			const TCHAR *cat = pClassD->Category();
			TCHAR *defcat = GetString(IDS_DX_VIEWPORT_EFFECT);
			if ((cat) && (_tcscmp(cat,defcat) == 0)) 
			{
			
				ReferenceTarget * effect = (ReferenceTarget *)pClassD->Create(TRUE);
				if(effect)
				{

					IDX9DataBridge * vp = (IDX9DataBridge*)effect->GetInterface(VIEWPORT_SHADER9_CLIENT_INTERFACE);
					if( vp)
					{
						if(bdx9)
						{
							
							if(vp->GetDXVersion() >=9.0f || vp->GetDXVersion() == 1.0f)
							{
								effectsList.Append(1, &pClassD);
							}
						}
						else
						{
							if(vp->GetDXVersion() < 9.0f)
							{
								effectsList.Append(1, &pClassD);
							}

						}
					}
					else
					{
						IDXDataBridge * vp = (IDXDataBridge*)effect->GetInterface(VIEWPORT_SHADER_CLIENT_INTERFACE);
						if(vp && !bdx9)
						{
							effectsList.Append(1, &pClassD);
						}
					}

					effect->DeleteAllRefsFromMe();
					effect->DeleteThis();
				}
			}

		}
	}
	theHold.Resume();
	effectsList.Sort(&classDescListCompare);
}

int ViewportLoader::NumShaders()
{
	if (effectsList.Count() == 0)
		LoadEffectsList();
	return effectsList.Count();
}

ClassDesc* ViewportLoader::GetEffectCD(int i)
{
	if (effectsList.Count() == 0)
		LoadEffectsList();
	return (i >= 0 && i < effectsList.Count()) ? effectsList[i] : NULL;
}

int ViewportLoader::FindEffectIndex(ReferenceTarget * e)
{
	if(e==NULL)
		return 0;
	for(int i=0;i<effectsList.Count();i++)
	{
		if(e->ClassID()==effectsList[i]->ClassID())
			return i+1;		//take into account "None" at 0
	}
	return 0;	//none found put up the "None"
}


void ViewportLoader::LoadEffect(ClassDesc * pd)
{

	ReferenceTarget * newEffect;
	ReferenceTarget * oldEffect;

	if (theHold.Holding())
	{
		oldEffect = effect;

//		theHold.Suspend(); 


		if(pd == NULL){
			newEffect = NULL;
		}
		else{
			newEffect = (ReferenceTarget *)pd->Create(FALSE);
			if(!newEffect)
			{
				// maybe defered
				ClassDesc * def = FindandLoadDeferedEffect(pd);
				if(def)
				{
					newEffect = (ReferenceTarget *)def->Create(FALSE);
				}

			}

		}

//		theHold.Resume(); 

		if (theHold.Holding())
			theHold.Put(new AddEffectRestore(this,oldEffect,newEffect));

		SwapEffect(newEffect);
	}
}

void ViewportLoader::SwapEffect(ReferenceTarget *e)
{
	ReplaceReference(1,e);

//	theHold.Suspend(); 

	if(clientDlg)
		clientDlg->DeleteThis();

	if(masterDlg)
	{
		masterDlg->DeleteDlg(clientDlg);
		masterDlg->ReloadDialog();
		clientDlg = NULL;
	}


	if(mEdit && mparam && effect)
	{
		IDXDataBridge * vp = (IDXDataBridge*)effect->GetInterface(VIEWPORT_SHADER_CLIENT_INTERFACE);
		clientDlg = vp->CreateEffectDlg(mEdit,mparam);
		masterDlg->AddDlg(clientDlg);
		
	}
//	theHold.Resume(); 
	
	NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	undo = false;

}

/////////////////////////// IViewportShaderManager Function Publishing Methods/////////////////////////////////////

int ViewportLoader::GetNumEffects()
{
	return NumShaders();
}

//We only return the effect if we are active..
ReferenceTarget* ViewportLoader::GetActiveEffect()
{
	return effect;	//return our active effect.
}


TCHAR * ViewportLoader::GetEffectName(int i)
{

	ClassDesc* pClassD = GetEffectCD(i-1);
	return (TCHAR*)pClassD->ClassName();
}

ReferenceTarget * ViewportLoader::SetViewportEffect(int i)
{
	theHold.Begin();
	pblock->SetValue(pb_effect,0,i);	//"None" is at position 0;
	theHold.Accept(GetString(IDS_STR_UNDO));

	if(effect)
		return effect;
	else
		return NULL;
}

void ViewportLoader::ActivateEffect(MtlBase * mtl, BOOL state)
{

	ICustAttribContainer* cc = mtl->GetCustAttribContainer();
	if(!cc)
		return;
	MtlBase * mb = (MtlBase *) cc->GetOwner();
	if(!mb)
		return;

	if(state && effect)
		mb->SetMtlFlag(MTL_HW_MAT_ENABLED);
	else
		mb->ClearMtlFlag(MTL_HW_MAT_ENABLED);
	
	GetCOREInterface()->RedrawViews(true);

}


BaseInterface* ViewportLoader::GetInterface(Interface_ID id) 
{ 
	if (id == VIEWPORT_SHADER_MANAGER_INTERFACE) 
		return (ViewportLoader*)this; 
	else 
		return FPMixinInterface::GetInterface(id);
}


