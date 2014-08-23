#include "MonoflectDialog.h"

/*===========================================================================*\
||	Class Descriptor                                                         ||
\*===========================================================================*/

class MonoFlectorClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic()					{ return FALSE; }
	void *			Create(BOOL loading)		{ return new MonoFlector; }
	const TCHAR *	ClassName()					{ return GetString(IDS_AP_MONONAME); }
	SClass_ID		SuperClassID()				{ return WSM_OBJECT_CLASS_ID; }
	Class_ID 		ClassID()					{ return MONOFLECTOR_CLASSID; }
	const TCHAR* 	Category()					{return GetString(SPACEWARPS_FOR_PARTICLES);}

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("MonoFlector"); }
	HINSTANCE		HInstance()					{ return hInstance; }
};

HWND       MonoFlector::hParams    = NULL;

static int typedlgname[] = 
{
	IDS_PLANEPARAMS,
	IDS_SPHEREPARAMS,
	IDS_MESHPARAMS
};

static int compdlgname[] = 
{
	IDS_SIMPPARAMS,
	IDS_COMPLEXPARAMS,
	IDS_DYNAMPARAMS
};

static MonoFlectorClassDesc MonoFlectorCD;
dlglist::dlglist(int newcnt,int *list)
{ 
	cnt=newcnt;
	namelst=list;
}

static dlglist nodlgname=dlglist(0,NULL);
ClassDesc* GetMonoFlectorDesc() 
{ 
	return &MonoFlectorCD; 
}

/*===========================================================================*\
 |	Paramblock2 Descriptor
\*===========================================================================*/
#define numstypedlgs 2

#define IMAGE_W 20 
#define IMAGE_H 18 

class BasicMonoFlectorDlgProc : public ParamMap2UserDlgProc 
{
	public:
		BasicFlectorObj *sso;
		BasicMonoFlectorDlgProc() {}
		BasicMonoFlectorDlgProc(BasicFlectorObj *sso_in) {sso = sso_in;}
		void DeleteThis() { }
		void SetParamBlock(IParamBlock2 *pb) 
		{
			sso = (BasicFlectorObj*)pb->GetOwner();
		}
};

class MonoFlectorTypeObjDlgProc : public BasicMonoFlectorDlgProc 
{	public:
		MonoFlector *mf;
		MonoFlectorTypeObjDlgProc(MonoFlector *imf,BasicFlectorObj *sso_in) {mf=imf;sso = sso_in;}
		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void SetParamBlock(IParamBlock2 *pb) 
		{ mf=(MonoFlector*)pb->GetOwner();
		  sso = mf->bfobj;
		}
};

BOOL MonoFlectorTypeObjDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	switch (msg) 
	{
		case WM_INITDIALOG:
		{ 	ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,IDC_EP_PICKBUTTON));
			iBut->SetType(CBT_CHECK);
			iBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(iBut);
			mf->hParams=hWnd;
			mf->ShowName(mf->pblock2->GetINode(PB_MESHNODE));
		}
			break;
		case WM_DESTROY:
			ICustButton *iBut;
			iBut=GetICustButton(GetDlgItem(hWnd,IDC_EP_PICKBUTTON));
			if (iBut) iBut->SetCheck(FALSE);
			ReleaseICustButton(iBut);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
			  case IDC_EP_PICKBUTTON:
			  { sso->IntoPickMode();
				break;
			  }
			}
			break;
	}
	return TRUE;
}

static ParamBlockDesc2 MonoFlectorTypes 
(	pbType_subani,
	_T("MonoFlector Type parameters"),  
	0, 
	&MonoFlectorCD, 
	P_AUTO_CONSTRUCT+P_AUTO_UI+P_MULTIMAP, 
	pbType_subani, 

	// params
	3,	
		flectplanepbd, IDD_MF_0101_FLECTTYPEPLANE, typedlgname[flectplanepbd], 0, 0, NULL,
		flectspherepbd, IDD_MF_0102_FLECTTYPESPHERE, typedlgname[flectspherepbd], 0, 0, NULL,
		flectmeshpbd, IDD_MF_0103_FLECTTYPEMESH, typedlgname[flectmeshpbd], 0, 0, NULL,

	PB_WIDTH, 	_T("IconSize"), 	TYPE_FLOAT, 	P_ANIMATABLE+P_RESET_DEFAULT,	IDS_ICONSIZE,
		p_default, 0.0f,
		p_range, 0.0f, 65535.0f,
		p_ui, flectplanepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_ICONSIZE0, IDC_EP_ICONSIZE0SPIN, 0.1f,
		p_uix, flectspherepbd,
		p_uix, flectmeshpbd,
		end,

	PB_LENGTH,  _T("Length"), TYPE_FLOAT, P_ANIMATABLE+P_RESET_DEFAULT, IDS_ICONHEIGHT,
		p_default, 0.0f,
		p_range, 0.0f, 65535.0f,
		p_ui, flectplanepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_ICONSIZE1, IDC_EP_ICONSIZE1SPIN, 0.1f,
		end,

	PB_QUALITY,  _T("Collision Quality"), TYPE_INT, P_RESET_DEFAULT, IDS_EP_QUALITY,
		p_default, 20,
		p_range, 0, 100,
		p_ui, flectplanepbd, TYPE_SPINNER, EDITTYPE_INT, IDC_EP_QUALITY, IDC_EP_QUALITYSPIN, SPIN_AUTOSCALE,
		end,

	PB_MESHNODE,  _T("Flector Custom Node"), TYPE_INODE,0,IDS_NODETXT,
//		p_ui, flectmeshpbd, TYPE_PICKNODEBUTTON, IDC_EP_PICKBUTTON,
		end,

	PB_HIDEICON, _T("Hide Icon"), TYPE_BOOL, P_RESET_DEFAULT, IDS_HIDEICON,
		p_default, 		FALSE, 
		p_ui, flectmeshpbd, TYPE_SINGLECHEKBOX, IDC_EP_HIDEICON, 
		end,

	end
);

static ParamBlockDesc2 MonoFlectorComplex ( pbComplex_subani, _T("Particle Type parameters"),  0, &MonoFlectorCD, P_AUTO_CONSTRUCT+P_AUTO_UI+P_MULTIMAP, pbComplex_subani, 
	// params
	3,	flectsimplepbd, IDD_MF_0201_FLECTCPLXSIMPLE, compdlgname[flectsimplepbd], 0, 0, NULL,
		flectcomplexpbd, IDD_MF_0202_FLECTCPLXCPLX, compdlgname[flectcomplexpbd], 0, 0, NULL,
		flectdynamicpbd, IDD_MF_0202_FLECTCPLXCPLX, compdlgname[flectdynamicpbd], 0, 0, NULL,

	PB_REFLECTS, _T("Reflects"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_REFLECTS,
		p_default, 1.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectsimplepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_AFFECTS, IDC_EP_AFFECTSSPIN, 0.1f,
		p_uix, flectcomplexpbd,
		p_uix, flectdynamicpbd,
		end,

	PB_BOUNCE, _T("Bounce"),  TYPE_FLOAT, P_ANIMATABLE+P_RESET_DEFAULT, IDS_BOUNCE,
		p_default, 1.0f,
		p_range, 0.0f, 65535.0f,
		p_ui, flectsimplepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_BOUNCE, IDC_EP_BOUNCESPIN, 0.1f,
		p_uix, flectcomplexpbd,
		p_uix, flectdynamicpbd,
		end,

	PB_BVAR, _T("BounceVariation"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_BVAR,
		p_default,0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectsimplepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_BOUNCEVAR, IDC_EP_BOUNCEVARSPIN, 0.1f,
		p_uix, flectcomplexpbd,
		p_uix, flectdynamicpbd,
		end,

	PB_CHAOS, _T("Chaos"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_CHAOS,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectsimplepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_CHAOS, IDC_EP_CHAOSSPIN, 0.1f,
		p_uix, flectcomplexpbd,
		p_uix, flectdynamicpbd,
		end,

	PB_FRICTION, _T("Friction"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_FRICTION,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectsimplepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_FRICTION, IDC_EP_FRICTIONSPIN, 0.1f,
		p_uix, flectcomplexpbd,
		p_uix, flectdynamicpbd,
		end,

	PB_INHERVEL, _T("Inherit Velocity"), TYPE_FLOAT, P_ANIMATABLE+P_RESET_DEFAULT, IDS_INHERVEL,
		p_default, 1.0f,
		p_range, 0.0f, 10000.0f,
		p_ui, flectsimplepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_INHERIT, IDC_EP_INHERITSPIN, 0.1f,
		p_uix, flectcomplexpbd,
		p_uix, flectdynamicpbd,
		end,

	PB_TIMEON,  _T("TimeOn"), TYPE_FLOAT, P_ANIMATABLE+P_RESET_DEFAULT, IDS_TIMEON,
		p_default, 0.0f,
		p_range, 0.0f, 65535.0f,
		p_ui, flectsimplepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_TIMEON, IDC_EP_TIMEONSPIN, 0.1f,
		p_uix, flectcomplexpbd, 
		p_uix, flectdynamicpbd,
		end,

	PB_TIMEOFF, _T("TimeOff"), TYPE_FLOAT, P_ANIMATABLE+P_RESET_DEFAULT, IDS_TIMEOFF,
		p_default, 100.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, flectsimplepbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_TIMEOFF, IDC_EP_TIMEOFFSPIN, 0.1f,
		p_uix, flectcomplexpbd, 
		p_uix, flectdynamicpbd,
		end,

	PB_REFRACTS, _T("Refracts"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_REFRACTS,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_REFRACTS, IDC_EP_REFRACTSSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,

	PB_PASSVEL,  _T("PassVelocity"), TYPE_FLOAT, P_ANIMATABLE+P_RESET_DEFAULT, IDS_PASSVELOCITY,
		p_default, 0.0f,
		p_range, 0.0f, 65535.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_DECEL, IDC_EP_DECELSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,

	PB_PASSVELVAR,  _T("PassVelocityVar"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_PASSVELOCITYVAR,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_DECELVAR, IDC_EP_DECELVARSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,

	PB_DISTORTION,  _T("Distortion"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_DISTORTION,
		p_default, 0.0f,
		p_range, -100.0f, 100.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_REFRACTION, IDC_EP_REFRACTIONSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,

	PB_DISTORTIONVAR,  _T("DistortionVar"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_DISTORTIONVAR,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_REFRACTVAR, IDC_EP_REFRACTVARSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,

	PB_DIFFUSION,  _T("Diffusion"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_DIFFUSION,
		p_default, 0.0f,
		p_range, -100.0f, 100.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_DIFFUSION, IDC_EP_DIFFUSIONSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,

	PB_DIFFUSIONVAR,  _T("DiffusionVar"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_DIFFUSIONVAR,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_DIFFUSIONVAR, IDC_EP_DIFFUSIONVARSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,

	PB_COLAFFECTS,  _T("CollisionAffects"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_COLAFFECTS,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_SPAWNSONLY, IDC_EP_SPAWNSONLYSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,

	PB_COLPASSVEL,  _T("Diffusion"), TYPE_FLOAT, P_ANIMATABLE+P_RESET_DEFAULT, IDS_COLPASSVEL,
		p_default, 0.0f,
		p_range, 0.0f, 65535.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_SPAWNONLYDECEL, IDC_EP_SPAWNONLYDECELSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,

	PB_COLPASSVELVAR,  _T("DiffusionVar"), TYPE_PCNT_FRAC, P_ANIMATABLE+P_RESET_DEFAULT, IDS_COLPASSVELVAR,
		p_default, 0.0f,
		p_range, 0.0f, 100.0f,
		p_ui, flectcomplexpbd, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EP_SPAWNSONLYDECELVAR, IDC_EP_SPAWNSONLYDECELVARSPIN, 0.1f,
		p_uix, flectdynamicpbd,
		end,
	end
);

/*===========================================================================*\
 |	Constructor
 |  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

MonoFlector::MonoFlector()
{
//	for (int i=0;i<numpblocks;i++) theParam[i]=NULL;
	MonoFlectorCD.MakeAutoParamBlocks(this);
	assert(pblock2);
	assert(pbComplex);
	bfobj=NULL;
	macroRec->Disable();   // 10/00
	pblock2->SetValue(PB_MESHNODE,0,(INode*)NULL);
	macroRec->Enable();  
}

MonoFlector::MonoFlector(BasicFlectorObj *bobj)
{//	for (int i=0;i<numpblocks;i++) theParam[i]=NULL;
	MonoFlectorCD.MakeAutoParamBlocks(this);
	assert(pblock2);assert(pbComplex);bfobj=bobj;
	macroRec->Disable();   // 10/00
	pblock2->SetValue(PB_MESHNODE,0,(INode*)NULL);
	macroRec->Enable();  
}

void MonoFlector::InvalidateUI()
{
	pmap[pbType_subani]->Invalidate();
	pmap[pbComplex_subani]->Invalidate();
}

/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/
void MonoFlector::ShowName(INode *oldn)
{ TSTR name=(oldn ? TSTR(oldn->GetName()) : TSTR(GetString(IDS_EP_NONE)));
  SetWindowText(GetDlgItem(hParams, IDC_EP_PICKNAME), name);
}

Animatable* MonoFlector::SubAnim(int i) 	
{
	switch (i) 
	{
		case pbType_subani:		return pblock2;
		case pbComplex_subani:	return pbComplex;
		default:					return NULL;
	}
}

TSTR MonoFlector::SubAnimName(int i) 
{
	switch (i) 
	{
		case pbType_subani:		return GetString(IDS_DLG_FTYPE);
		case pbComplex_subani:	return GetString(IDS_DLG_FCOMPLEX);
		default:					return _T("");
	}
}

RefTargetHandle MonoFlector::GetReference(int i)
{
	switch (i) 
	{
		case pbType_subani:		return pblock2;
		case pbComplex_subani:	return pbComplex;
		default:					return NULL;
	}
}
void MonoFlector::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) 
	{
		case pbType_subani:		pblock2 = (IParamBlock2*)rtarg;		break;
		case pbComplex_subani:	pbComplex = (IParamBlock2*)rtarg;		break;
	}
}

IParamBlock2* MonoFlector::GetParamBlock(int i)
{
	switch(i) 
	{
		case pbType_subani:		return pblock2;
		case pbComplex_subani:	return pbComplex;
		default:					return NULL;
	}
}
IParamBlock2* MonoFlector::GetParamBlockByID(BlockID id)
{
	if(pblock2->ID() == id)
		return pblock2;
	else if(pbComplex->ID() == id)
		return pbComplex;
	else
		return NULL;
}
RefTargetHandle MonoFlector::Clone(RemapDir& remap) 
{
	MonoFlector* newob = new MonoFlector();	
	if (pblock2) newob->ReplaceReference(pbType_subani,pblock2->Clone(remap));
	if (pbComplex) newob->ReplaceReference(pbComplex_subani,pbComplex->Clone(remap));
	BaseClone(this, newob, remap);
	return newob;
}

RefResult MonoFlector::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	IParamMap2* pm2 = 0;
	switch (message) 
	{
		case REFMSG_CHANGE:
			if(hTarget == pblock2)
			{	if((pm2 = pblock2->GetMap()) != 0) 
					pm2->Invalidate(pblock2->LastNotifyParamID());
			}
			else if(hTarget == pbComplex)
			{	if((pm2 = pbComplex->GetMap()) != 0)	
					pm2->Invalidate(pbComplex->LastNotifyParamID());
			}
				break;
		case REFMSG_TARGET_DELETED:	
			{  INode *inode=pblock2->GetINode(PB_MESHNODE);
			   if (hTarget==inode) 
			   {  pblock2->SetValue(PB_MESHNODE,0,(INode*)NULL);
			   }
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			{ INode *inode=pblock2->GetINode(PB_MESHNODE);
			  if (hTarget==inode) 
			  {  ShowName(inode);
				}
			  break;
			}
	}
	return REF_SUCCEED;
}

dlglist MonoFlector::GetNameList(int which)
{ 
	switch(which)
	{
		case pbType_subani:		return dlglist(3,typedlgname);
		case pbComplex_subani:	return dlglist(3,compdlgname);
	}
	return nodlgname;
}

/*===========================================================================*\
 |	Support the Parammap UI
\*===========================================================================*/
IParamMap2 *GetNewPmap(MonoFlector *stype, Interface *ip,int num,HWND hOldRollup=NULL)
{	IParamMap2 *pmap=NULL;
	switch(num)
	{
		case flectplanepbd:
			 pmap = CreateCPParamMap2(flectplanepbd,stype->pblock2,ip,hInstance,
					MAKEINTRESOURCE(IDD_MF_0101_FLECTTYPEPLANE),GetString(typedlgname[flectplanepbd]),0,NULL,hOldRollup);
			 break;
		case flectspherepbd: 
			 pmap = CreateCPParamMap2(flectspherepbd, stype->pblock2,ip,hInstance,
					MAKEINTRESOURCE(IDD_MF_0102_FLECTTYPESPHERE),GetString(typedlgname[flectspherepbd]),0,NULL,hOldRollup);
			 break;
		case flectmeshpbd: 
			 pmap = CreateCPParamMap2(flectmeshpbd, stype->pblock2,ip,hInstance,
					MAKEINTRESOURCE(IDD_MF_0103_FLECTTYPEMESH),GetString(typedlgname[flectmeshpbd]),0,NULL,hOldRollup);
			 pmap->SetUserDlgProc(new MonoFlectorTypeObjDlgProc(stype,stype->bfobj));
			 break;
	}
	if (stype->theParam[pbType_subani]) 
	{ stype->theParam[pbType_subani]=NULL;}
	return pmap;

}

IParamMap2 *GetNewPmapC(MonoFlector *stype, Interface *ip,int num,HWND hOldRollup=NULL)
{	IParamMap2 *pmap = NULL;
	switch(num)
	{
		case flectsimplepbd:
			 pmap = CreateCPParamMap2(flectsimplepbd,stype->pbComplex,ip,hInstance,
					MAKEINTRESOURCE(IDD_MF_0201_FLECTCPLXSIMPLE),GetString(compdlgname[flectsimplepbd]),0,NULL,hOldRollup);
			 break;
		case flectcomplexpbd: 
			 pmap = CreateCPParamMap2(flectcomplexpbd, stype->pbComplex,ip,hInstance,
					MAKEINTRESOURCE(IDD_MF_0202_FLECTCPLXCPLX),GetString(compdlgname[flectcomplexpbd]),0,NULL,hOldRollup);
			 break;
		case flectdynamicpbd: 
			 pmap = CreateCPParamMap2(flectdynamicpbd, stype->pbComplex,ip,hInstance,
					MAKEINTRESOURCE(IDD_MF_0202_FLECTCPLXCPLX),GetString(compdlgname[flectdynamicpbd]),0,NULL,hOldRollup);
			 break;
	}
	if (stype->theParam[pbComplex_subani]) 
	{ stype->theParam[pbComplex_subani] = NULL;}
	return pmap;

}

MonoFlectorParam::MonoFlectorParam(MonoFlector *stype, Interface *intface,int num,int dtype,HWND cwnd) 
{
	theType = stype;
	ip = intface;
	type=dtype;
	switch (dtype)
	{	case pbType_subani:		theType->pmap[dtype]=GetNewPmap(stype,ip,num,cwnd);		break;
		case pbComplex_subani:	theType->pmap[dtype]=GetNewPmapC(stype,ip,num,cwnd);	break;
	}
}

MonoFlectorParam::~MonoFlectorParam()
{
	if(theType)
	{	theType->theParam[type] = NULL;
		DestroyCPParamMap2(theType->pmap[type]);
		theType->pmap[type]=NULL;
	}
}
#define MONOFLECT_BFOBJ_CHUNK		0x0100

IOResult MonoFlector::Save(ISave *isave)
{ ULONG nb;
	int refid;
	refid=isave->GetRefID(bfobj);
	isave->BeginChunk(MONOFLECT_BFOBJ_CHUNK);		
	isave->Write(&refid,sizeof(int),&nb);
	isave->EndChunk();
	return IO_OK;
}

IOResult MonoFlector::Load(ILoad *iload)
{	ULONG nb;
	IOResult res = IO_OK;
	int refid;bfobj=NULL;
	while (IO_OK==(res=iload->OpenChunk())) 
	{	switch (iload->CurChunkID()) {
			case MONOFLECT_BFOBJ_CHUNK: 
			{	res=iload->Read(&refid,sizeof(int),&nb);
			    iload->RecordBackpatch(refid,(void**)&bfobj);
				break; }
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	return IO_OK;
}
