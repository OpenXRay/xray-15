/**********************************************************************
 *<
	FILE: meditutils.h


	DESCRIPTION:	
	-- Multi Material Clean
	clean unused sub materials in scene multi-materials
	-- Instance Duplicate Map
	substitude duplicates scene material maps with instances

	CREATED BY:		Alex Zadorozhny

	HISTORY:		Created 6/17/03

	*>	Copyright (c) 2003, All Rights Reserved.
	**********************************************************************/

#ifndef __MEDITUTILS__H
#define __MEDITUTILS__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "icurvctl.h"

#include "utilapi.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;


// class to find all node dependents 
class FindNodesProc : public DependentEnumProc {
public:
	FindNodesProc(INodeTab* tab) {
		nodetab = tab;
	}
	int proc(ReferenceMaker *ref) {
		switch (ref->SuperClassID()) {
				case BASENODE_CLASS_ID:
					INode* n = (INode*)ref;
					nodetab->Append(1, &n, 5);
					break;
		}
		return 0;
	}
private:
	INodeTab* nodetab;
};
#endif // __MEDITUTILS__H
