/**********************************************************************
 *<
	FILE:  utilint.h

	DESCRIPTION:  Utility Interface (namespace)

	CREATED BY: Pete Samson

	HISTORY: created 13 September 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _UTILINT_H_
#define _UTILINT_H_

namespace UtilityInterface {
	UtilExport TCHAR* GetRegistryKeyBase();	// get string locating
											// product sub-keys in
											// Windows Registry
};

#endif // _UTILINT_H_

