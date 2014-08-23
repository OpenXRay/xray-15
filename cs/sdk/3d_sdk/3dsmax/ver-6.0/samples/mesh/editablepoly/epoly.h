/**********************************************************************
 *<
	FILE: EPoly.h

	DESCRIPTION:

	CREATED BY: Steve Anderson

	HISTORY:

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#ifndef __EPOLY__H
#define __EPOLY__H

#include "Max.h"
#include "IParamm2.h"
#include "evrouter.h"
#include "resource.h"
#include "resourceOverride.h"

TCHAR *GetString(int id);

extern ClassDesc2 *GetPolySelectDesc();
extern ClassDesc2 *GetEditablePolyDesc();
extern ClassDesc2 *GetEditNormalsDesc();

extern HINSTANCE hInstance;
extern int enabled;

// Polymesh selection toolbar icons - used in select and edit tools.
class PolySelImageHandler {
public:
	HIMAGELIST images, hPlusMinus;

	PolySelImageHandler () : images(NULL), hPlusMinus(NULL) { }
	~PolySelImageHandler () { if (images) ImageList_Destroy (images); if (hPlusMinus) ImageList_Destroy (hPlusMinus); }
	HIMAGELIST LoadImages ();
	HIMAGELIST LoadPlusMinus ();
};

extern PolySelImageHandler *GetPolySelImageHandler();
extern bool CheckNodeSelection (Interface *ip, INode *inode);

#endif
