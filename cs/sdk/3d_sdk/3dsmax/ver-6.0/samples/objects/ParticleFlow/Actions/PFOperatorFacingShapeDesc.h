/**********************************************************************
 *<
	FILE:			PFOperatorFacingShapeDesc.h

	DESCRIPTION:	FacingShape Operator Class Descriptor (declaration)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORFACINGSHAPEDESC_H_
#define  _PFOPERATORFACINGSHAPEDESC_H_

#include "max.h"
#include "iparamb2.h"
#include "PFActionStateDesc.h"

namespace PFActions {

//	Descriptor declarations
class PFOperatorFacingShapeDesc: public ClassDesc2 {
	public:
	~PFOperatorFacingShapeDesc();
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

class PFOperatorFacingShapeStateDesc: public PFActionStateDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

} // end of namespace PFActions

#endif // _PFOPERATORFACINGSHAPEDESC_H_