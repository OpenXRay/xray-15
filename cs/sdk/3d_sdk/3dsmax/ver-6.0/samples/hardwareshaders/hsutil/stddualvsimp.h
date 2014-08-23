/**********************************************************************
 *<
	FILE: StdDualVSImp.h

	DESCRIPTION: Standard Dual VertexShader helper class implementation

	CREATED BY: Nikolai Sander, Discreet

	HISTORY: created 10/11/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __STDDUALVSIMP__H
#define __STDDUALVSIMP__H

#include"HSUtil.h"
#include "IStdDualVS.h"
#include "iparamb2.h"


class StdDualVSImp : public IStdDualVS
{
protected:
	Tab<VertexShaderCache*> caches;
	ReferenceTarget *rtarg;
	IStdDualVSCallback *callb;

public:
	~StdDualVSImp();
	void SetCallback(IStdDualVSCallback *callback){ callb = callback;}

	virtual HRESULT Initialize(Mesh *mesh, INode *node);
	virtual HRESULT Initialize(MNMesh *mnmesh, INode *node);
	int FindNodeIndex(INode *node);
	// ReferenceMaker
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message);

	int NumRefs(){ return caches.Count();}
	void SetReference(int i, RefTargetHandle rtarg);
	RefTargetHandle GetReference(int i);
};


class StdDualVSImpClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new StdDualVSImp;}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return REF_MAKER_CLASS_ID;}
	Class_ID		ClassID() {return STD_DUAL_VERTEX_SHADER;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("StdDualVS"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};


extern ClassDesc2* GetStdDualVSImpDesc();

#endif