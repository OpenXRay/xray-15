/**********************************************************************
 *<
	FILE:			PFOperatorSpeedKeepApartDesc.h

	DESCRIPTION:	SpeedKeepApart Operator Class Descriptor (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSPEEDKEEPAPARTDESC_H_
#define  _PFOPERATORSPEEDKEEPAPARTDESC_H_

#include "max.h"
#include "iparamb2.h"

namespace PFActions {

//	Descriptor declarations
class PFOperatorSpeedKeepApartDesc: public ClassDesc2 {
	public:
	~PFOperatorSpeedKeepApartDesc();
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

#endif // _PFOPERATORSPEEDKEEPAPARTDESC_H_