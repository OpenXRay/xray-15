/**********************************************************************
 *<
	FILE: IViewportManager.h

	DESCRIPTION:	Viewport Manager for loading up Effects

	CREATED BY:		Neil Hazzard

	HISTORY:		Created:  02/15/02
					

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef	__IVIEWPORTMANAGER_H__
#define __IVIEWPORTMANAGER_H__

#include "iFnPub.h"
#include "custattrib.h"
#include "IHardwareMaterial.h"

#define VIEWPORT_SHADER_MANAGER_INTERFACE Interface_ID(0x5dbe33d6, 0x2e1b422b)
#define VIEWPORT_SHADER_CLIENT_INTERFACE Interface_ID(0x40c926b8, 0x7c3a66b7)
#define VIEWPORT_SHADER9_CLIENT_INTERFACE Interface_ID(0x40c926b7, 0x7c3a6347)

#define IDX_SHADER_MANAGER Interface_ID(0x6dce7429, 0x200169ac)

class IViewportShaderManager : public FPMixinInterface {
	public:

		enum {	get_num_effects, get_active_effect,	is_effect_active, is_manager_active,
		get_effect_name,set_effect,activate_effect};



		BEGIN_FUNCTION_MAP
			FN_0(get_num_effects,		TYPE_INT,  GetNumEffects);
			FN_0(get_active_effect,		TYPE_REFTARG, GetActiveEffect);
			FN_1(get_effect_name,		TYPE_STRING, GetEffectName, TYPE_INT);
			FN_1(set_effect,			TYPE_REFTARG,SetViewportEffect, TYPE_INT);
			VFN_2(activate_effect,		ActivateEffect,TYPE_MTL,TYPE_BOOL);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    

		virtual int GetNumEffects()=0;
		virtual ReferenceTarget* GetActiveEffect()=0;
		virtual TCHAR * GetEffectName(int i)=0;
		virtual ReferenceTarget * SetViewportEffect(int i)=0;
		virtual void ActivateEffect(MtlBase * mtl, BOOL State)=0;


	};


class IDXDataBridge : public BaseInterface
{
public:
	virtual Interface_ID	GetID() { return VIEWPORT_SHADER_CLIENT_INTERFACE; }

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }
	virtual ParamDlg * CreateEffectDlg(HWND hWnd, IMtlParams * imp)= 0;
	virtual void DisableUI()=0;
	virtual TCHAR * GetName()=0;
	virtual void SetDXData(IHardwareMaterial * pHWMtl, Mtl * pMtl)=0;
};


// If you are creating a DX9 based effect then you need to implement this interface 
class IDX9DataBridge : public IDXDataBridge
{
public:
	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }
	virtual Interface_ID	GetID() { return VIEWPORT_SHADER9_CLIENT_INTERFACE; }
	virtual float GetDXVersion() = 0;	// DX 8.1 or 9.0 etc...
	
};


class IDXShaderManagerInterface : public FPStaticInterface
{
	public:
		virtual CustAttrib* FindViewportShaderManager (MtlBase* mtl)=0;
		virtual CustAttrib* AddViewportShaderManager(MtlBase * mtl)=0;
		
		virtual void	SetVisible(BOOL show=TRUE)=0;
		virtual BOOL	IsVisible()=0;
	
};

inline IDXShaderManagerInterface* GetDXShaderManager() { return (IDXShaderManagerInterface*)GetCOREInterface(IDX_SHADER_MANAGER); }


#endif