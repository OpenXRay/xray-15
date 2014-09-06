////////////////////////////////////////////////////////////////////////////
//	Module 		: smart_cast.cpp
//	Created 	: 17.09.2004
//  Modified 	: 17.09.2004
//	Author		: Dmitriy Iassenev
//	Description : Smart dynamic cast
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifndef PURE_DYNAMIC_CAST
#	ifdef XRGAME_EXPORTS
#		include "Include/xrRender/RenderVisual.h"
#		include "alife_space.h"
#		include "hit.h"
#		include "Actor.h"
#		include "custommonster.h"
#		include "ai/stalker/ai_stalker.h"
#		include "ui/uiwindow.h"
#		include "CustomZone.h"
#		include "weapon.h"
#		include "cameraeffector.h"
#	endif
#	include "xrServer_Objects_ALife_Monsters.h"
#endif

#undef   SMART_CAST_H
#include "smart_cast.h"
