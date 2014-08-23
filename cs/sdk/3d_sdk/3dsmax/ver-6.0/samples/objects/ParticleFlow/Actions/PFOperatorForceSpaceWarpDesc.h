/**********************************************************************
 *<
	FILE:			PFOperatorForceSpaceWarpDesc.h

	DESCRIPTION:	ForceSpaceWarp Operator Class Descriptor (declaration)
											 
	CREATED BY:		Peter Watje

	HISTORY:		created 02-06-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORFORCESPACEWARPDESC_H_
#define  _PFOPERATORFORCESPACEWARPDESC_H_

#include "max.h"
#include "iparamb2.h"

namespace PFActions {

//	Descriptor declarations
class PFOperatorForceSpaceWarpDesc: public ClassDesc2 {
	public:
	~PFOperatorForceSpaceWarpDesc();
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

} // end of namespace PFActions

#endif // _PFOPERATORSIMPLESPEEDDESC_H_