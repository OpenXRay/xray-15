/**********************************************************************
 *<
	FILE:  impexpintf.h

	DESCRIPTION:  Import/Export Interface (namespace)

	CREATED BY: Pete Samson

	HISTORY: created 7 January 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _IMPEXPINT_H_
#define _IMPEXPINT_H_

namespace ImportExportInterface {
	UtilExport void SetCanConvertUnits(bool ifSo);	// plug-in calls to
			// say whether plug-in can _both_ convert to/from current
			// system units _and_ ignore them (no conversion)
	UtilExport bool GetCanConvertUnits();			// max calls to
			// ask how plug-in set canConvertUnits
	UtilExport int AskUserConvertUnits(HWND hWnd, bool isExport);
			// max calls (if plug-in can convert units) to ask user
			// whether to do so
	UtilExport bool GetShouldConvertUnits();		// plug-in calls to
			// ask whether user has indicated to convert to/from system
			// units (only should be called if last call to
			// SetCanConvertUnits(ifSo) had arg value true)
};

#endif // _IMPEXPINT_H_

