/*===========================================================================*\
 | 
 |  FILE:	MorpherMXS.h
 |			A new MAX Script Plugin that adds Morpher modifier access
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 5-4-99
 | 
\*===========================================================================*/

#include "MAXScrpt.h"

// Various MAX and MXS includes
#include "Numbers.h"
#include "MAXclses.h"
#include "Streams.h"
#include "MSTime.h"
#include "MAXObj.h"
#include "Parser.h"
#include "3DMath.h"
#include "Numbers.h"

#include "max.h"
#include "stdmat.h"

// Morpher header
#include "wm3.h"

// define the new primitives using macros from SDK
#include "definsfn.h"



BOOL check_ValidMorpher(ReferenceTarget* obj,Value** arg_list);
