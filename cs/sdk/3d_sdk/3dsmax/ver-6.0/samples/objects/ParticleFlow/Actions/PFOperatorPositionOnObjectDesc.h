/**********************************************************************
 *<
	FILE:			PFOperatorPositionOnObjectDesc.h

	DESCRIPTION:	PositionOnObject Operator Class Descriptor (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORPOSITIONONOBJECTDESC_H_
#define  _PFOPERATORPOSITIONONOBJECTDESC_H_

#include "max.h"
#include "iparamb2.h"
#include "PFActionStateDesc.h"

namespace PFActions {

//	Descriptor declarations
class PFOperatorPositionOnObjectDesc: public ClassDesc2 {
	public:
	~PFOperatorPositionOnObjectDesc();
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

class PFOperatorPositionOnObjectStateDesc: public PFActionStateDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

} // end of namespace PFActions

#endif // _PFOPERATORPOSITIONONOBJECTDESC_H_