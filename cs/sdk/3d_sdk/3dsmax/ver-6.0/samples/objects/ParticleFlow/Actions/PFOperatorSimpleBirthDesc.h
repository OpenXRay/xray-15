/**********************************************************************
 *<
	FILE:			PFOperatorSimpleBirthDesc.h

	DESCRIPTION:	SimpleBirth Operator Class Descriptor (declaration)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-12-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSIMPLEBIRTHDESC_H_
#define  _PFOPERATORSIMPLEBIRTHDESC_H_

#include "max.h"
#include "iparamb2.h"
#include "PFActionStateDesc.h"

namespace PFActions {

//	Descriptor declarations
class PFOperatorSimpleBirthDesc: public ClassDesc2 {
	public:
	~PFOperatorSimpleBirthDesc();
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

class PFOperatorSimpleBirthStateDesc: public PFActionStateDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

} // end of namespace PFActions

#endif // _PFOPERATORSIMPLEBIRTHDESC_H_