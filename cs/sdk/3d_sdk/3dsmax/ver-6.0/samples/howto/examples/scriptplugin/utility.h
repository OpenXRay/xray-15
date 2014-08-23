/*===========================================================================*\
 | 
 |  FILE:	Plugin.cpp
 |			Utility that accesses a scripted plugin's parameters
 |			Demonstrates SDK -> MAX Script plugins techniques
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 7-4-99
 | 
\*===========================================================================*/
/*===========================================================================*\
 |	Please read the notes/techniques text in ScP_Read.cpp for more info
\*===========================================================================*/


#ifndef __SCPUTIL__H
#define __SCPUTIL__H

#include "max.h"
#include "iparamm2.h"
#include "iparamb2.h"
#include "utilapi.h"

#include "resource.h"


// Utility class ID
#define	SCPUTIL_CLASSID		Class_ID(0x2cb46e2a, 0x48ba025f)

// GameMtl class ID
#define GAMEMTL_CLASSID		Class_ID(0x67296df6, 0)


TCHAR *GetString(int id);
extern ClassDesc* GetSCPUtilDesc();



/*===========================================================================*\
 |	SCPUtility class defn
\*===========================================================================*/

class SCPUtility : public UtilityObj {
	public:

		IUtil *iu;
		Interface *ip;

		// Windows handle of our UI
		HWND hPanel;
		IColorSwatch *cs;

		//Constructor/Destructor
		SCPUtility();
		~SCPUtility();

		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
};

static SCPUtility theSCPUtil;

#endif