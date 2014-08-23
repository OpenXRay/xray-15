/**********************************************************************
 *<
	FILE: ViewportManagerControl.cpp

	DESCRIPTION:	Add the  ViewportManager to the Material Editor

	CREATED BY:		Neil Hazzard

	HISTORY:		02/15/02

	TODO:			Provide a class structure for handing the CA in material
					there is alot of code copying going on at the moment

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#include "d3dx9.h"
#include "ViewportManager.h"
#include "ViewportLoader.h"
#include "Notify.h"

#define VIEWPORTMANAGERCONTROL_CLASS_ID	Class_ID(0x286308e0, 0x5b309c41)


class ViewportManagerControl : public GUP, public ReferenceMaker {
	public:

		bool reEntry;
		static HWND hParams;
		BOOL ShowManager;
		MtlBase * mb;

		// GUP Methods
		DWORD	Start			( );
		void	Stop			( );
		DWORD	Control			( DWORD parameter );
		
		// Loading/Saving
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);


		//Constructor/Destructor

		ViewportManagerControl();
		~ViewportManagerControl();	
		
		BOOL IsManagerLoaded();
		void RemoveManager();
		void LoadManager();
		BOOL IsValidMaterial();

		void SetReference(int i, ReferenceTarget * targ);
		ReferenceTarget * GetReference(int i);
		int NumRefs(){return 1;}
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,  PartID &partID, RefMessage message);
	
		
};


ViewportManagerControl theViewportManagerControl;
class ViewportManagerControlClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &theViewportManagerControl; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return VIEWPORTMANAGERCONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("ViewportManagerControl"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static ViewportManagerControlClassDesc ViewportManagerControlDesc;
ClassDesc2* GetViewportManagerControlDesc() { return &ViewportManagerControlDesc; }



ViewportManagerControl::ViewportManagerControl()
{
	ShowManager = false;
	mb = NULL;
	reEntry = false;

}

ViewportManagerControl::~ViewportManagerControl()
{
	DeleteAllRefsFromMe();

}
static void UpdateINIFile(void *param, NotifyInfo *info) 
{
	TCHAR filename[MAX_PATH];
   	TCHAR Buf[256];
	IDXShaderManagerInterface * sm = GetDXShaderManager();
	itoa(sm->IsVisible(),Buf,10);
	
	_tcscpy(filename,GetCOREInterface()->GetDir(APP_PLUGCFG_DIR));
	_tcscat(filename,_T("\\DXManager.ini")); 
	if(CheckForDX())
		WritePrivateProfileString(_T("DXSettings"),_T("ManagerEnabled"),Buf,filename);
	else
		WritePrivateProfileString(_T("OGLSettings"),_T("ManagerEnabled"),Buf,filename);

}

// Activate and Stay Resident
DWORD ViewportManagerControl::Start( ) {
	
	// TODO: Return if you want remain loaded or not
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	MtlBase *mtl = mtlEdit->GetCurMtl();

	int defaultVis = 0;

    DependentIterator di(mtl); 
    RefMakerHandle rm; 
    while (NULL!=(rm=di.Next())) { 
		if ((rm->SuperClassID() == REF_MAKER_CLASS_ID) &&  (rm->ClassID() == Class_ID(MEDIT_CLASS_ID,0))) {
			ReplaceReference(0,(ReferenceTarget*)rm,TRUE);
		}
    }

	//Setup as ::Stop is called too late to use GetCOREInterface()
	RegisterNotification(UpdateINIFile,this,NOTIFY_SYSTEM_SHUTDOWN);

	TCHAR filename[MAX_PATH];
	IDXShaderManagerInterface * sm = GetDXShaderManager();
	_tcscpy(filename,GetCOREInterface()->GetDir(APP_PLUGCFG_DIR));
	_tcscat(filename,"\\DXManager.ini");

	// Always on for DX
	// NH 04|16|03 - Relaxed this, from a request from PFB and the vis users
	if(CheckForDX())
	{
		ShowManager = GetPrivateProfileInt(_T("DXSettings"),_T("ManagerEnabled"),1,filename);
		sm->SetVisible(ShowManager);
	}
		
	else
	{

		ShowManager = GetPrivateProfileInt(_T("OGLSettings"),_T("ManagerEnabled"),0,filename);
		sm->SetVisible(ShowManager);
	}
	return GUPRESULT_KEEP;
}

void ViewportManagerControl::Stop( ) {

	UnRegisterNotification(UpdateINIFile,this,NOTIFY_SYSTEM_SHUTDOWN);
	DeleteAllRefsFromMe();
}

DWORD ViewportManagerControl::Control( DWORD parameter ) {
	return 0;
}

#define MANAGER_REMOVEALL_CHUNK	0x1000

IOResult ViewportManagerControl::Save(ISave *isave)
{
	return IO_OK;
}

IOResult ViewportManagerControl::Load(ILoad *iload)
{
	return IO_OK;
}

void ViewportManagerControl::SetReference(int i, ReferenceTarget * targ)
{
	mb = (MtlBase*)targ;
}

ReferenceTarget * ViewportManagerControl::GetReference(int i )
{
	return mb;
}


RefResult ViewportManagerControl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,PartID &partID, RefMessage message)
{
	switch (message)
	{
		case REFMSG_TARGET_DELETED:
			break;

		case REFMSG_TARGET_SELECTIONCHANGE:
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:

			if(!reEntry)
			{
				if(IsValidMaterial())
				{
					if(!IsManagerLoaded())
						reEntry = true;
						LoadManager();
				}
				else{
					RemoveManager();
					reEntry = false;
				}
			}
			if(reEntry)
				return REF_STOP;
								
			break;

	}
	
    return REF_SUCCEED;
}

BOOL ViewportManagerControl::IsManagerLoaded()
{
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);

	MtlBase *mtl = mtlEdit->GetCurMtl();
	if(!mtl)
		return true;	//this just happens with early ref messeages
	
	ICustAttribContainer* cc = mtl->GetCustAttribContainer();
	if(!cc)
	{
		return false;
	}

	for(int i=0; i<cc->GetNumCustAttribs();i++)
	{
		CustAttrib * ca = cc->GetCustAttrib(i);
		if(ca->GetInterface(VIEWPORT_SHADER_MANAGER_INTERFACE))
			return true;
	}
	return false;
}

// This needs to be reconsidered - at we should not really delete the CA - we just want to hide it
void ViewportManagerControl::RemoveManager()
{
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);

	MtlBase *mtl = mtlEdit->GetCurMtl();
	if(!mtl)
		return;
	
	ICustAttribContainer* cc = mtl->GetCustAttribContainer();
	if(!cc)
	{
		mtl->AllocCustAttribContainer();
		cc = mtl->GetCustAttribContainer();
	}

	for(int i=0; i<cc->GetNumCustAttribs();i++)
	{
		CustAttrib * ca = cc->GetCustAttrib(i);
		if(ca->GetInterface(VIEWPORT_SHADER_MANAGER_INTERFACE))
			cc->RemoveCustAttrib(i);
	}
	
}

void ViewportManagerControl::LoadManager()
{
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	MtlBase *mtl = mtlEdit->GetCurMtl();
	if(!mtl)
		return;		//only in very BAD cases or early ref messages
	if(IsManagerLoaded())
		return;

	ICustAttribContainer* cc = mtl->GetCustAttribContainer();
	if(!cc)
	{
		mtl->AllocCustAttribContainer();
		cc = mtl->GetCustAttribContainer();
	}
	if(cc->GetNumCustAttribs()>0)
		cc->InsertCustAttrib(0,(CustAttrib *)CreateInstance(CUST_ATTRIB_CLASS_ID,VIEWPORTLOADER_CLASS_ID));
	else
		cc->AppendCustAttrib((CustAttrib *)CreateInstance(CUST_ATTRIB_CLASS_ID,VIEWPORTLOADER_CLASS_ID));

	reEntry  = false;

}

static Class_ID multiClassID(MULTI_CLASS_ID,0);
static Class_ID bakeShellClassID(BAKE_SHELL_CLASS_ID,0);
#define DXMATERIAL_DYNAMIC_UI Class_ID(0xef12512, 0x11351ed1)

bool IsDynamicDxMaterial(MtlBase * newMtl)
{

	DllDir * lookup = GetCOREInterface()->GetDllDirectory();
	ClassDirectory & dirLookup = lookup->ClassDir();

	ClassDesc * cd = dirLookup.FindClass(MATERIAL_CLASS_ID,newMtl->ClassID());
	if(cd->SubClassID() == DXMATERIAL_DYNAMIC_UI)
		return true;
	else
		return false;


}


BOOL ViewportManagerControl::IsValidMaterial()
{
	// We need to limit where the effect can be applied.  It can go on any top level material, but we restrict it
	// to only Multi Materials as a sublevel.  So a StdMtl2 on a blend is not allowed, but is OK for Multi.
	// If the material is only referenced by the editor then its a top level material, 
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	MtlBase *mtl = mtlEdit->GetCurMtl();
	if(!mtl)
		return false;		//only in very BAD cases or early ref messages

	if(mtl->SuperClassID()== TEXMAP_CLASS_ID)
		return false;
	// as the multi material is special we do not allow it at the parent, but rather at the child level
	// also added Shell Material
	if(mtl->ClassID()==multiClassID ||mtl->ClassID()==bakeShellClassID )
		return false;
	// we don't let it on the Viewport Shader Material.
	if(IsDynamicDxMaterial(mtl) )
		return false;



	RefList &list = mtl->GetRefList();
	RefListItem *ptr = list.first;
	BOOL multiparent = false;
	int mtlcount = 0;


	while (ptr) 
	{
		if (ptr->maker) 
		{
			if (ptr->maker->SuperClassID()==MATERIAL_CLASS_ID ) 
			{
				if(ptr->maker->ClassID()==multiClassID)
					multiparent = true;
				if(ptr->maker->SuperClassID()==TEXMAP_CLASS_ID)
					multiparent = true;
				// added as per Claude's request
				if(ptr->maker->ClassID()==bakeShellClassID)
					multiparent = true;

				mtlcount++;
			}
		}
		ptr = ptr->next;
	}
	
	if(mtlcount==0 || (mtlcount>0 && multiparent))
		return true;
	else 
		return false;

}



class DXShaderManager :public IDXShaderManagerInterface
{
	public:
		BOOL vis;
		enum {
			getShaderManager,isVisible,setVisible,addShaderManager
		};
		
		CustAttrib* FindViewportShaderManager (MtlBase* mtl);
		CustAttrib*	AddViewportShaderManager(MtlBase * mtl);
		void	SetVisible(BOOL show=TRUE);
		BOOL	IsVisible();
	
		
		DECLARE_DESCRIPTOR(DXShaderManager);

		BEGIN_FUNCTION_MAP
			FN_1(getShaderManager, TYPE_REFTARG, FindViewportShaderManager, TYPE_MTL);
			FN_0(isVisible,TYPE_BOOL, IsVisible);
			VFN_1(setVisible, SetVisible,  TYPE_BOOL);
			FN_1(addShaderManager,TYPE_REFTARG,AddViewportShaderManager,TYPE_MTL);

		END_FUNCTION_MAP

};

static DXShaderManager iShaderManagerInterface (IDX_SHADER_MANAGER, _T("dxshadermanager"), 0,
		NULL, FP_CORE + FP_STATIC_METHODS,
	//methods
	DXShaderManager::getShaderManager, _T("getViewportManager"), 0, TYPE_REFTARG, 0, 1,
		_T("material"), 0, TYPE_MTL,
	
	DXShaderManager::isVisible,_T("IsVisible"),0,TYPE_BOOL,0,0,

	DXShaderManager::setVisible, _T("SetVisible"),0,0,0,1,
		_T("show"), 0, TYPE_BOOL,	
	
	DXShaderManager::addShaderManager, _T("addViewportManager"), 0, TYPE_REFTARG, 0, 1,
	_T("material"), 0, TYPE_MTL,

	end
	); 


CustAttrib * DXShaderManager::FindViewportShaderManager (MtlBase* mtl)
{	
	
	if(!mtl)
		return NULL;

	ICustAttribContainer* cc = mtl->GetCustAttribContainer();
	if(!cc)
	{
		return NULL;
	}

	for(int i=0; i<cc->GetNumCustAttribs();i++)
	{
		CustAttrib * ca = cc->GetCustAttrib(i);
		if(ca->GetInterface( VIEWPORT_SHADER_MANAGER_INTERFACE))
			return ca;
	}
	return NULL;
}

CustAttrib * DXShaderManager::AddViewportShaderManager (MtlBase* mtl)
{	
	if(!mtl)
		return NULL;

	CustAttrib * manager = (CustAttrib *)CreateInstance(CUST_ATTRIB_CLASS_ID,VIEWPORTLOADER_CLASS_ID);

	if(!manager)
		return NULL;

	ICustAttribContainer* cc = mtl->GetCustAttribContainer();
	if(!cc)
	{
		mtl->AllocCustAttribContainer();
		cc = mtl->GetCustAttribContainer();
	}
	if(cc->GetNumCustAttribs()>0)
		cc->InsertCustAttrib(0,manager);
	else
		cc->AppendCustAttrib(manager);

	return manager;
}


void DXShaderManager::SetVisible(BOOL show)
{
	IMtlEditInterface *mtlEdit = (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE);
	vis = show;
//  NH 04|16|03 - Relaxed this, from a request from PFB and the vis users
//	if(CheckForDX())
//		vis = true;

	//force an update
	int slot = mtlEdit->GetActiveMtlSlot();
	mtlEdit->SetActiveMtlSlot(slot);
	

}
BOOL DXShaderManager::IsVisible()
{
	return vis;
}