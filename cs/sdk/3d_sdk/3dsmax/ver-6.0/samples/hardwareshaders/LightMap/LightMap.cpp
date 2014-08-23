/**********************************************************************
 *<
	FILE: LightMap.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "LightMap.h"
#include <d3dx8.h>
#include "IViewportManager.h"
#include "IHardwareShader.h"
#include "IHardwareRenderer.h"
#include "StdMat.h"
#include "gamma.h"


#define LIGHTMAP_CLASS_ID	Class_ID(0x727d33be, 0x3255c000)
#define PBLOCK_REF	0


enum { lightmap_params };


//TODO: Add enums for various parameters
enum { 
	pb_light_texture,
	pb_diffuse_texture,
	pb_diffuse_mapping,
	pb_lightmap_filename,
	pb_diffuse_filename,
	pb_lightmap_on,
	pb_diffuse_on,
	pb_lightmap_mapping,
};

class LightMapDAD;

class LightMap : public ReferenceTarget, public IDX9DataBridge {
	public:

		LightMapDAD * lmDnD;
		LightMap();
		~LightMap();	

		IParamBlock2	*pblock;	//ref 0

		DWORD_PTR lighttex;
		DWORD_PTR diffusetex;

		PBBitmap * light;
		PBBitmap * diffuse;

		BOOL reInit;		//small optimization for textures
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return LIGHTMAP_CLASS_ID;}		
		SClass_ID SuperClassID() { return REF_TARGET_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);

		int	NumParamBlocks() { return 1; }			
		IParamBlock2* GetParamBlock(int i) { if(i == 0) return pblock; else return NULL;} 
		IParamBlock2* GetParamBlockByID(short id) { if(id == lightmap_params ) return pblock; else return NULL;} 

		int NumRefs(){return 1;}
		void SetReference(int i, RefTargetHandle  targ);
		ReferenceTarget * GetReference(int i);
		void DeleteThis() { delete this; }		
		ReferenceTarget * Clone(RemapDir &remap);
		void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags);

		void SetLightmap(TCHAR * name);
		void SetDiffusemap(TCHAR * name);

		// From IDXDataBridge
		ParamDlg * CreateEffectDlg(HWND hWnd, IMtlParams * imp);
		void DisableUI();
		void SetDXData(IHardwareMaterial * pHWMtl, Mtl * pMtl);
		TCHAR * GetName(){return GetString(IDS_EFFECT_NAME);}
		float GetDXVersion() { return 1.0f;}	//universal

		BaseInterface* GetInterface(Interface_ID id);


};

class LightMapDAD : public DADMgr
{

	public:
		LightMap * lm;
		LightMapDAD(LightMap * map){lm = map;}

		SClass_ID		GetDragType(HWND hwnd, POINT p) { return BITMAPDAD_CLASS_ID; }
		BOOL			OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
		int				SlotOwner() { return OWNER_MTL_TEX;	}
		ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
		void			Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type);
		BOOL			LetMeHandleLocalDAD() { return 0; } 

		void			Update(TimeValue t);
		BOOL			DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void			DeleteThis(){ delete this;  }	

};


class LightMapClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new LightMap(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID		ClassID() { return LIGHTMAP_CLASS_ID; }
	// The Viewport Manager checks the category to decide whether the ReferenceTarget is an Effect .  This 
	// must not be changed
	const TCHAR* 	Category() { return GetString(IDS_DX_VIEWPORT_EFFECT); }

	const TCHAR*	InternalName() { return _T("LightMap"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static LightMapClassDesc LightMapDesc;
ClassDesc2* GetLightMapDesc() { return &LightMapDesc; }

class LightMapDlgProc : public ParamMap2UserDlgProc 
{
	public:
		LightMap * lightmap;

		LightMapDlgProc() {}
		LightMapDlgProc(LightMap *dl) { lightmap = dl; }

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() { }


};


class PSCM_Accessor : public PBAccessor
{
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		LightMap *map = (LightMap*) owner;
		IParamMap2* pmap = map->pblock->GetMap();
		TSTR p,f,e,name;
		switch(id)
		{
			case pb_light_texture: 
			{
				if(pmap)
				{
					TSTR lightname(v.bm->bi.Name());
					SplitFilename(lightname, &p, &f, &e);
					name = f+e;
					pmap->SetText(pb_light_texture, name.data());
//					map->pblock->SetValue(pb_lightmap_filename,0,(TCHAR*)v.bm->bi.Name());
				}
				break;
			}
			case pb_diffuse_texture: 
			{
				if(pmap)
				{
					TSTR diffusename(v.bm->bi.Name());
					SplitFilename(diffusename, &p, &f, &e);
					name = f+e;
					pmap->SetText(pb_diffuse_texture, name.data());
//					map->pblock->SetValue(pb_diffuse_filename,0,(TCHAR*)v.bm->bi.Name());
				}
				break;
			}
			case pb_lightmap_filename:
				if(pmap)
				{
					TSTR lightname(v.s);
					SplitFilename(lightname, &p, &f, &e);
					name = f+e;
					pmap->SetText(pb_light_texture, name.data());
					
				}
				map->SetLightmap(v.s);
				break;
			case pb_diffuse_filename:
				if(pmap)
				{
					TSTR diffusename(v.s);
					SplitFilename(diffusename, &p, &f, &e);
					name = f+e;
					pmap->SetText(pb_diffuse_texture, name.data());
				}
				map->SetDiffusemap(v.s);
				break;
			default: break;
		}

		map->reInit = true;
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());		//this forces the effect to be re-evaluated
	}
};

BOOL LightMapDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TSTR p,f,e,name;

	ICustButton * But;
	
	switch (msg) 
	{
		case WM_INITDIALOG:{
//			TCHAR *lightname;
//			TCHAR *diffusename;

			lightmap->pblock->GetValue(pb_light_texture,0, lightmap->light,FOREVER);
			lightmap->pblock->GetValue(pb_diffuse_texture,0, lightmap->diffuse,FOREVER);
			But = GetICustButton(GetDlgItem(hWnd,IDC_DIFFUSE_TEXTURE));
			But->SetDADMgr(lightmap->lmDnD);
			ReleaseICustButton(But);

			But = GetICustButton(GetDlgItem(hWnd,IDC_LIGHTMAP_TEXTURE));
			But->SetDADMgr(lightmap->lmDnD);
			ReleaseICustButton(But);

			if(!lightmap->light)
				map->SetText(pb_light_texture, GetString(IDS_NONE));
			else
			{
				TSTR light(lightmap->light->bi.Name());
				SplitFilename(light, &p, &f, &e);
				name = f+e;
				map->SetText(pb_light_texture, name.data());
			}

			if(!lightmap->diffuse)
				map->SetText(pb_diffuse_texture, GetString(IDS_NONE));
			else
			{
				TSTR diffuse(lightmap->diffuse->bi.Name());
				SplitFilename(diffuse, &p, &f, &e);
				name = f+e;
				map->SetText(pb_diffuse_texture, name.data());
			}
			break;

		}
	}
	return FALSE;
}


static PSCM_Accessor pscm_accessor;


static ParamBlockDesc2 lightmap_param_blk ( lightmap_params, _T("params"),  0, &LightMapDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_CUSTATTRIB, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_light_texture,	_T("lightmap_texture"),	TYPE_BITMAP,	P_NO_AUTO_LABELS,	IDS_LIGHTMAP_TEXTURE,
		p_ui,			TYPE_BITMAPBUTTON, IDC_LIGHTMAP_TEXTURE,
		p_accessor,		&pscm_accessor,
		end,
	pb_diffuse_texture,	_T("diffuse_texture"),	TYPE_BITMAP,	P_NO_AUTO_LABELS,	IDS_DIFFUSE_TEXTURE,
		p_ui,			TYPE_BITMAPBUTTON, IDC_DIFFUSE_TEXTURE,
		p_accessor,		&pscm_accessor,
		end,
	pb_diffuse_mapping,		_T("diffuse_mapping"), TYPE_INT,0,IDS_DIFFUSE_MAPPING,
		p_default,		3,
		p_range, 		1,100, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_DIFFUSE_MAPCHAN_EDIT,	IDC_DIFFUSE_MAPCHAN_SPIN,  SPIN_AUTOSCALE,
		p_accessor,		&pscm_accessor,
		end,
	pb_lightmap_filename,		_T("lightmap_filename"), TYPE_FILENAME,		0,	IDS_LIGHT_FILENAME,
		p_accessor,		&pscm_accessor,
		end,
	pb_diffuse_filename,		_T("diffuse_filename"), TYPE_FILENAME,		0,	IDS_DIFFUSE_FILENAME,
		p_accessor,		&pscm_accessor,
		end,
	pb_lightmap_on,			_T("lightmap_on"), TYPE_BOOL,	0,	IDS_LIGHTMAP_ON,
		p_ui,			TYPE_SINGLECHEKBOX,		IDC_LIGHTMAP_ON,
		p_default,		TRUE,
		p_accessor,		&pscm_accessor,
		end,
	pb_diffuse_on,			_T("diffuse_on"), TYPE_BOOL,	0,	IDS_DIFFUSE_ON,
		p_ui,			TYPE_SINGLECHEKBOX,		IDC_DIFFUSE_ON,
		p_default,		TRUE,
		p_accessor,		&pscm_accessor,
		end,
	pb_lightmap_mapping,		_T("lightmap_mapping"), TYPE_INT,0,IDS_LIGHTMAP_MAPPING,
		p_default,		3,
		p_range, 		1,100, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_INT, IDC_LIGHT_MAPCHAN_EDIT,	IDC_LIGHT_MAPCHAN_SPIN,  SPIN_AUTOSCALE,
		p_accessor,		&pscm_accessor,
		end,



	end
	);



LightMap::LightMap()
{
	pblock = NULL;
	LightMapDesc.MakeAutoParamBlocks(this);
	lighttex = diffusetex = NULL;
	light = diffuse = NULL;
	reInit = true;	//get the initial update
	lmDnD = new LightMapDAD(this);


}

LightMap::~LightMap()
{
	pblock = NULL;
}

void LightMap::SetReference(int i, RefTargetHandle  targ)
{
	if(i==0)
		pblock = (IParamBlock2 *)targ;

}

RefTargetHandle LightMap::GetReference(int i)
{
	if(i==0)
		return pblock;
	else
		return NULL;
}

ReferenceTarget *LightMap::Clone(RemapDir &remap)
{
	LightMap *pnew = new LightMap;
	pnew->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, pnew, remap);
	return pnew;
}


IOResult LightMap::Load(ILoad *iload)
{
	//TODO: Standard loading of data
	return IO_OK;
}


IOResult LightMap::Save(ISave *isave)
{
	//TODO: standard saving of data
	return IO_OK;
}

RefResult LightMap::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}

void LightMap::SetLightmap(TCHAR * name)
{
	BitmapInfo bi;
	if (name) {
		bi.SetName(name); 
		PBBitmap bt(bi);
		pblock->SetValue(pb_light_texture, 0, &bt);
	}
}
void LightMap::SetDiffusemap(TCHAR * name)
{
	BitmapInfo bi;
	if (name) {
		bi.SetName(name); 
		PBBitmap bt(bi);
		pblock->SetValue(pb_diffuse_texture, 0, &bt);
	}
}


BaseInterface* LightMap::GetInterface(Interface_ID id)
{
	if (id == VIEWPORT_SHADER_CLIENT_INTERFACE) {
		return static_cast<IDXDataBridge*>(this);
	}
	else if(id == VIEWPORT_SHADER9_CLIENT_INTERFACE){
		return static_cast<IDX9DataBridge*>(this);
	}
	else 
	{
		return BaseInterface::GetInterface(id);
	}
}

ParamDlg * LightMap::CreateEffectDlg(HWND hWnd, IMtlParams * imp)
{
	ParamDlg * dlg = LightMapDesc.CreateParamDlgs(hWnd, imp, this);
	lightmap_param_blk.SetUserDlgProc(new LightMapDlgProc(this));

	return dlg;
}

void LightMap::DisableUI()
{
	HWND dlg , hWnd;
	hWnd = pblock->GetMap()->GetHWnd();
	dlg = GetDlgItem(hWnd,IDC_DIFFUSE_TEXTURE);
	EnableWindow(dlg,false);
	dlg = GetDlgItem(hWnd,IDC_LIGHTMAP_TEXTURE);
	EnableWindow(dlg,false);
}



void LightMap::SetDXData(IHardwareMaterial * pHWMtl, Mtl * pMtl)
{
	int diffuseMapping, lightmapMapping;
	DWORD lightformat = D3DFMT_R8G8B8;
	DWORD diffuseformat = D3DFMT_R8G8B8;
	BITMAPINFO * diffuseBM = NULL;
	BITMAPINFO * lightBM = NULL;


	BOOL lightmapOn;
	BOOL diffuseOn;

	int numStages =0;
	int lightStage =0;
	int diffuseStage=0;

	pblock->GetValue(pb_diffuse_mapping,0,diffuseMapping,FOREVER);
	pblock->GetValue(pb_lightmap_mapping,0,lightmapMapping,FOREVER);

	pblock->GetValue(pb_lightmap_on,0,lightmapOn,FOREVER);
	pblock->GetValue(pb_diffuse_on,0,diffuseOn,FOREVER);

	if(!lightmapOn && !diffuseOn)
	{
		Color black(0,0,0);
		pHWMtl->SetNumTexStages(0);
		pHWMtl->SetDiffuseColor(black);
		pHWMtl->SetAmbientColor(black);
		return;
	}


	if(reInit)
	{
		pblock->GetValue(pb_diffuse_texture,0,diffuse,FOREVER);
		pblock->GetValue(pb_light_texture,0,light,FOREVER);


		if(diffuse==NULL ||light==NULL)
		{
			Color black(0,0,0);
			pHWMtl->SetNumTexStages(0);
			pHWMtl->SetDiffuseColor(black);
			pHWMtl->SetAmbientColor(black);

			return;
		}

		// load the bitmap
		diffuse->Load();
		light->Load();

		if(!diffuse->bm || ! light->bm)
		{
			Color black(0,0,0);
			pHWMtl->SetNumTexStages(0);
			pHWMtl->SetDiffuseColor(black);
			pHWMtl->SetAmbientColor(black);

			return;
		}

		// convert the bitmap
		diffuseBM = ConvertBitmap(diffuse->bm);
		lightBM = ConvertBitmap(light->bm);

		if(diffuse->bm->HasAlpha())
			diffuseformat = D3DFMT_A8R8G8B8;

		if(light->bm->HasAlpha())
			lightformat = D3DFMT_A8R8G8B8;


		


		ViewExp *pview = GetCOREInterface()->GetActiveViewport();;
		GraphicsWindow *gw = pview->getGW();

		IHardwareRenderer * phr = (IHardwareRenderer *)gw->GetInterface(HARDWARE_RENDERER_INTERFACE_ID);

		// Create the Texture
		if(lighttex)
			phr->FreeTexture(lighttex);
		lighttex = phr->BuildTexture(lightBM,0,0,lightformat);
		
		if(diffusetex)
			phr->FreeTexture(diffusetex);

		diffusetex = phr->BuildTexture(diffuseBM,0,0,diffuseformat);
		reInit = false;

	}

	numStages = lightmapOn + diffuseOn;
	if(numStages == 2)
		lightStage = 1;
	
	pHWMtl->SetNumTexStages(numStages);

	if(diffuseOn)
	{
		pHWMtl->SetTexture(diffuseStage,diffusetex);
		pHWMtl->SetTextureMapChannel(diffuseStage, diffuseMapping);
		pHWMtl->SetTextureTransform(diffuseStage, NULL);
		pHWMtl->SetTextureAddressMode(diffuseStage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		pHWMtl->SetTextureAddressMode(diffuseStage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		pHWMtl->SetTextureColorArg(diffuseStage, 1, D3DTA_TEXTURE);
		pHWMtl->SetTextureColorOp(diffuseStage,  D3DTOP_SELECTARG1);
		pHWMtl->SetTextureAlphaOp(diffuseStage, D3DTOP_SELECTARG1);
		pHWMtl->SetTextureAlphaArg(diffuseStage,2,D3DTA_TEXTURE);
		pHWMtl->SetTextureUVWSource(diffuseStage, UVSOURCE_MESH);
		pHWMtl->SetTextureTransformFlag(diffuseStage, D3DTTFF_COUNT2);
	}
	if(lightmapOn)
	{
		pHWMtl->SetTexture(lightStage,lighttex);
		pHWMtl->SetTextureTransform(lightStage, NULL);
		pHWMtl->SetTextureMapChannel(lightStage, lightmapMapping);
		pHWMtl->SetTextureAddressMode(lightStage, 0, D3DTADDRESS_WRAP);
		pHWMtl->SetTextureAddressMode(lightStage, 1, D3DTADDRESS_WRAP);
		pHWMtl->SetTextureColorArg(lightStage, 1, D3DTA_TEXTURE);
		pHWMtl->SetTextureColorOp(lightStage,   D3DTOP_MODULATE);
		pHWMtl->SetTextureColorArg(lightStage, 2, D3DTA_CURRENT);
		pHWMtl->SetTextureAlphaOp(lightStage, D3DTOP_DISABLE);
		pHWMtl->SetTextureUVWSource(lightStage, UVSOURCE_MESH);
		pHWMtl->SetTextureTransformFlag(lightStage, D3DTTFF_COUNT2);

		if(!diffuseOn)
		{
			pHWMtl->SetTextureColorArg(lightStage, 1, D3DTA_TEXTURE);
			pHWMtl->SetTextureColorOp(lightStage,  D3DTOP_SELECTARG1);
			pHWMtl->SetTextureAlphaOp(lightStage, D3DTOP_SELECTARG1);

		}
	}
	if(diffuseBM)
		free(diffuseBM);

	if(lightBM)
		free(lightBM);


/*
   pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
   pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
   pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
   pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

   pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
   pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );
   pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
   pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
*/
}


// LAM - 4/21/03 - added
void LightMap::EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) 
{
	if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; 
	if (!(flags&FILE_ENUM_INACTIVE)) return; // not needed by renderer

	if (reInit) // need to do update
	{	
		pblock->GetValue(pb_diffuse_texture,0,diffuse,FOREVER);
		pblock->GetValue(pb_light_texture,0,light,FOREVER);
	}

	if(diffuse)
		diffuse->bi.EnumAuxFiles(nameEnum,flags);
	if(light)
		light->bi.EnumAuxFiles(nameEnum,flags);

	ReferenceTarget::EnumAuxFiles( nameEnum, flags );
}

// Drag and Drop Support
static Class_ID bmTexClassID(BMTEX_CLASS_ID,0);

BOOL LightMapDAD::OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew)
{
	if(hfrom == hto)
	{
		return(FALSE);
	}

	if(type == BITMAPDAD_CLASS_ID || (type==TEXMAP_CLASS_ID && dropThis->ClassID()==bmTexClassID))	{
		return(true);
	}	 	

	return(false);

}



ReferenceTarget *LightMapDAD::GetInstance(HWND hwnd, POINT p, SClass_ID type) 
{
	DADBitmapCarrier *Bmc;
	PBBitmap * bm;
	TimeValue t = GetCOREInterface()->GetTime();
	Bmc = GetDADBitmapCarrier();
	IParamMap2 * pmap = lm->pblock->GetMap();
	HWND ownerHwnd = pmap->GetHWnd();
	ICustButton * texBut, *lmBut;
	
	texBut = GetICustButton(GetDlgItem(ownerHwnd,IDC_DIFFUSE_TEXTURE));
	lmBut = GetICustButton(GetDlgItem(ownerHwnd,IDC_LIGHTMAP_TEXTURE));


	if(Bmc)
	{
		if(texBut->GetHwnd()==hwnd)
		{
			lm->pblock->GetValue(pb_diffuse_texture ,t,bm,FOREVER);
			if(bm)
			{

				Bmc->SetName(TSTR(bm->bi.Name()));
			}
			else
				Bmc->SetName(TSTR(GetString(IDS_NONE)));

		}
		if(lmBut->GetHwnd()==hwnd)
		{
			lm->pblock->GetValue(pb_light_texture ,t,bm,FOREVER);
			if(bm)
			{

				Bmc->SetName(TSTR(bm->bi.Name()));
			}
			else
				Bmc->SetName(TSTR(GetString(IDS_NONE)));

		}

	}

	return(Bmc);
}




void LightMapDAD::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type) 
{
	char				Name[256];
	TSTR path,file,ext;
	PBBitmap			Bitmap;

	BitmapInfo			Info;
	DADBitmapCarrier	*Bmc;
	TimeValue t = GetCOREInterface()->GetTime();


	IParamMap2 * pmap = lm->pblock->GetMap();
	HWND ownerHwnd = pmap->GetHWnd();
	ICustButton * texBut,*lmBut;
	
	texBut = GetICustButton(GetDlgItem(ownerHwnd,IDC_DIFFUSE_TEXTURE));
	lmBut = GetICustButton(GetDlgItem(ownerHwnd,IDC_LIGHTMAP_TEXTURE));

	if(texBut->GetHwnd()==hwnd)


	
	Name[0] = '\0';

	if(dropThis)
	{

		if(dropThis->SuperClassID() == BITMAPDAD_CLASS_ID)
		{
			Bmc = (DADBitmapCarrier *)dropThis;
			strcpy(Name,Bmc->GetName());
		}
		else if(dropThis->SuperClassID() == TEXMAP_CLASS_ID)
		{
			BitmapTex * tex = (BitmapTex*) dropThis;
			if(tex)
				strcpy(Name,tex->GetMapName());
			
		}
		else
			return;
	
	}

	if(!strlen(Name))
	{
		strcpy(Name,GetString(IDS_NONE));
	}

	if(texBut->GetHwnd()==hwnd)
	{
		TSTR mapName;
		Bitmap.bi.SetName(Name);
		Bitmap.bm = NULL;
		lm->pblock->SetValue(pb_diffuse_texture,t,&Bitmap);
		TSTR diffusename(Bitmap.bi.Name());
		SplitFilename(diffusename, &path, &file, &ext);
		mapName = file+ext;
		texBut->SetText(mapName.data());
	}

	if(lmBut->GetHwnd()==hwnd)
	{
		TSTR mapName;
		Bitmap.bi.SetName(Name);
		Bitmap.bm = NULL;
		lm->pblock->SetValue(pb_light_texture,t,&Bitmap);
		TSTR lightname(Bitmap.bi.Name());
		SplitFilename(lightname, &path, &file, &ext);
		mapName = file+ext;
		lmBut->SetText(mapName.data());

	}
}
