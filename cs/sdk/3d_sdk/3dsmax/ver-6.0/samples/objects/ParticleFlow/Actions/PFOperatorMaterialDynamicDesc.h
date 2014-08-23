/**********************************************************************
 *<
	FILE:			PFOperatorMaterialDynamicDesc.h

	DESCRIPTION:	MaterialDynamic Operator Class Descriptor (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORMATERIALDYNAMICDESC_H_
#define  _PFOPERATORMATERIALDYNAMICDESC_H_

#include "max.h"
#include "iparamb2.h"
#include "PFActionStateDesc.h"

namespace PFActions {

//	Descriptor declarations
class PFOperatorMaterialDynamicDesc: public ClassDesc2 {
	public:
	~PFOperatorMaterialDynamicDesc();
	int 			IsPublic();
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName();
	SClass_ID		SuperClassID();
	Class_ID		ClassID();
	Class_ID		SubClassID();
	const TCHAR* 	Category();

	const TCHAR*	InternalName();
	HINSTANCE		HInstance();

	INT_PTR Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3);

	static HBITMAP m_depotIcon;
	static HBITMAP m_depotMask;
};

class PFOperatorMaterialDynamicStateDesc: public PFActionStateDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

} // end of namespace PFActions

#endif // _PFOPERATORMATERIALDYNAMICDESC_H_