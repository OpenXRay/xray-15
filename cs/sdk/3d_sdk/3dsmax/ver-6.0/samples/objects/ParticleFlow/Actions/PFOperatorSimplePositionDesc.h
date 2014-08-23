/**********************************************************************
 *<
	FILE:			PFOperatorSimplePositionDesc.h

	DESCRIPTION:	SimplePosition Operator Class Descriptor (declaration)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSIMPLEPOSITIONDESC_H_
#define  _PFOPERATORSIMPLEPOSITIONDESC_H_

#include "max.h"
#include "iparamb2.h"
#include "PFActionStateDesc.h"

namespace PFActions {

//	Descriptor declarations
class PFOperatorSimplePositionDesc: public ClassDesc2 {
	public:
	~PFOperatorSimplePositionDesc();
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

class PFOperatorSimplePositionStateDesc: public PFActionStateDesc {
public:
	void*	Create(BOOL loading = FALSE);
	Class_ID	ClassID();
	const TCHAR* InternalName();
};

} // end of namespace PFActions

#endif // _PFOPERATORSIMPLEPOSITIONDESC_H_