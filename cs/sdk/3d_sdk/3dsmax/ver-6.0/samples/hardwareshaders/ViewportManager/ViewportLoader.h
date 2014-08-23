/**********************************************************************
 *<
	FILE: ViewportLoader.cpp

	DESCRIPTION:	Viewport Manager for loading up Effects

	CREATED BY:		Neil Hazzard

	HISTORY:		Created:  02/15/02
					

 *>	Copyright (c) 2002, All Rights Reserved.
************************************************************************/

#ifndef	__VIEWPORTLOADER_H__
#define __VIEWPORTLOADER_H__

#include "CustAttrib.h"
#include "IViewportManager.h"


#define VIEWPORTLOADER_CLASS_ID Class_ID(0x5a06293c, 0x30420c1e)
#define PBLOCK_REF 0
#define NOT_FOUND -1


enum { viewport_manager_params };  // pblock ID

enum 
{ 
	pb_enabled,
	pb_effect,
	
};

class ViewportLoader : public CustAttrib, public IViewportShaderManager
{
public:
	
	int effectNum;
	int effectIndex;
	IParamBlock2 *pblock;
	IAutoMParamDlg* masterDlg;
	ParamDlg * clientDlg;
	ReferenceTarget * effect;
	ReferenceTarget * oldEffect;
	bool undo;
	HWND mEdit;
	IMtlParams *mparam;

	

	ViewportLoader();
	~ViewportLoader();
	Tab < ClassDesc *> effectsList;

	virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,  PartID& partID,  RefMessage message);
	
	virtual ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);


	int	NumParamBlocks() { return 1; }			
	IParamBlock2* GetParamBlock(int i) { if(i == 0) return pblock; else return NULL;} 
	IParamBlock2* GetParamBlockByID(short id) { if(id == viewport_manager_params ) return pblock; else return NULL;} 

	int NumRefs() { return 2;}
	void SetReference(int i, RefTargetHandle rtarg);
	RefTargetHandle GetReference(int i);
	
	virtual	int NumSubs();
	virtual	Animatable* SubAnim(int i);
	virtual TSTR SubAnimName(int i);

	// Override CustAttrib::SvTraverseAnimGraph
	SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);


	SClass_ID		SuperClassID() {return CUST_ATTRIB_CLASS_ID;}
	Class_ID 		ClassID() {return VIEWPORTLOADER_CLASS_ID;}	
	


	ReferenceTarget *Clone(RemapDir &remap = NoRemap());
	virtual bool CheckCopyAttribTo(ICustAttribContainer *to) { return true; }
	
	virtual TCHAR* GetName(){ return GetString(IDS_VIEWPORT_MANAGER);}
	
	void DeleteThis() { delete this;}

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);



	int FindManagerPosition();
	ClassDesc * FindandLoadDeferedEffect(ClassDesc * defered);
	void LoadEffectsList();
	ClassDesc* GetEffectCD(int i);
	int FindEffectIndex(ReferenceTarget * e);
	
	int NumShaders();
	void LoadEffect(ClassDesc * pd);
	void SwapEffect(ReferenceTarget *e);

	int GetNumEffects();
	ReferenceTarget* GetActiveEffect();

	TCHAR * GetEffectName(int i);
	ReferenceTarget * SetViewportEffect(int i);
	void ActivateEffect (MtlBase * mtl, BOOL state);

	BaseInterface* GetInterface(Interface_ID id) ;
};


class EffectsDlgProc : public ParamMap2UserDlgProc 
{
	public:
		ViewportLoader *vl;

		EffectsDlgProc() {}
		EffectsDlgProc(ViewportLoader *loader) { vl = loader; }

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() { }


};

// the following is for the Undo system.....



class AddEffectRestore : public RestoreObj {
	public:
		ViewportLoader *vpLoader;
		SingleRefMaker  undoEffect;
		SingleRefMaker  redoEffect;

		AddEffectRestore(ViewportLoader *c,ReferenceTarget *oldEffect, ReferenceTarget * newEffect) 
		{
			vpLoader = c;
			undoEffect.SetRef(oldEffect);
			redoEffect.SetRef(newEffect);
			
		}   		
		void Restore(int isUndo) 
		{
			vpLoader->SwapEffect(undoEffect.GetRef());
			
		}
		void Redo()
		{
			vpLoader->SwapEffect(redoEffect.GetRef());

		}		
		void EndHold() 
		{ 
			vpLoader->ClearAFlag(A_HELD);
		}
	};


#endif