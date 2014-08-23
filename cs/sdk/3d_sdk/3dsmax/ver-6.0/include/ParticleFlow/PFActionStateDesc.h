/*! \file PFActionStateDesc.h
    \brief	Class Descriptor for PF ActionState objects (declaration)
*/
/**********************************************************************
 *<
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 28-10-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFACTIONSTATEDESC_H_
#define  _PFACTIONSTATEDESC_H_

#include "max.h"
#include "iparamb2.h"
#include "PFExport.h"

//	ActionState-generic Descriptor declarations
class PFActionStateDesc: public ClassDesc2 {
public:
	PFExport virtual int IsPublic();
	virtual void*	Create(BOOL loading = FALSE) = 0;
	PFExport virtual const TCHAR *	ClassName();
	PFExport virtual SClass_ID SuperClassID();
	PFExport virtual Class_ID SubClassID();
	virtual Class_ID	ClassID() = 0;
	PFExport const TCHAR* Category();
	virtual const TCHAR* InternalName() = 0;
};

#endif // _PFACTIONSTATEDESC_H_
