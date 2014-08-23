/**********************************************************************
 *<
	FILE: MatCustAttrib.cpp

	DESCRIPTION:	Defines the Material Custom Attribute

	CREATED BY:		Nikolai Sander

	HISTORY:		Created:  5/26/00
					Turnd into Sample Neil Hazzard - DCG: 12/5/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "max.h"
#include "DynPBlock.h"
#include "CASample.h"

static MatCustAttribClassDesc theMatCustAttribDesc;

ClassDesc2* GetMatCustAttribDesc(){ return &theMatCustAttribDesc;}



static ParamBlockDesc2 param_blk ( cust_attrib_params, _T("parameters"),  0, &theMatCustAttribDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_CUSTATTRIB, IDS_CUSTOM_ATTRIBUTES, 0, 0, NULL, 
	// params

	ca_color,	 _T("CustAttribColor"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_COLOR,	
		p_default,		Color(1.0,1.0,1.0), 
		p_ui,			TYPE_COLORSWATCH, IDC_COLOR, 
		end,

	ca_submap,	 _T("CustAttribSubMap"),	TYPE_TEXMAP,		P_OWNERS_REF,	IDS_SUBMAP,	
		p_refno,		MAP_REF,
		p_subtexno,		0,
		p_ui,			TYPE_TEXMAPBUTTON, IDC_SUBMAP, 
		end,

		end
		);

MatCustAttrib::MatCustAttrib()
{
	theMatCustAttribDesc.MakeAutoParamBlocks(this); 
	submap = NULL;
}

MatCustAttrib::~MatCustAttrib()
{
	DebugPrint("MAT Destructor\n");
}



ReferenceTarget *MatCustAttrib::Clone(RemapDir &remap)
{
	MatCustAttrib *pnew = new MatCustAttrib;
	pnew->MakeRefByID(FOREVER,0,remap.CloneRef(pblock));
	BaseClone(this, pnew, remap);
	return pnew;
}


ParamDlg *MatCustAttrib::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
{
	return theMatCustAttribDesc.CreateParamDlgs(hwMtlEdit, imp, this);
}

void MatCustAttrib::SetReference(int i, RefTargetHandle rtarg) 
{
	switch(i)
	{
		case PBLOCK_REF: pblock = (IParamBlock2 *)rtarg;
			break;
		case MAP_REF : submap = (Texmap *) rtarg;
			break;
	}
}

RefTargetHandle MatCustAttrib::GetReference(int i)
{
	switch(i)
	{
		case PBLOCK_REF: return pblock;
		case MAP_REF : return submap;
		default: return NULL;
	}
}	
void MatCustAttrib::SetSubTexmap(int i, Texmap *m)
{
	if (i==0)
	{
		ReplaceReference(MAP_REF,m);
		param_blk.InvalidateUI(ca_color);
}	}

int MatCustAttrib::NumSubs()  
{
	return pblock->NumSubs()+1; 
}

Animatable* MatCustAttrib::SubAnim(int i)
{
	if(i < pblock->NumSubs())
		return pblock->SubAnim(i); 
	else if(i == pblock->NumSubs())
		return submap;
	else
		return NULL;
}

TSTR MatCustAttrib::SubAnimName(int i)
{
	if(i < pblock->NumSubs())
		return pblock->SubAnimName(i);
	else if(i == pblock->NumSubs())
		return GetSubTexmapTVName(0);
	else
		return TSTR(_T("????"));


}
