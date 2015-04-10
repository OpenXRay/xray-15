#pragma once

#include "../../Include/xrRender/RenderVisual.h"

class glRender_Visual :
	public IRenderVisual
{
public:
	virtual vis_data&	getVisData() { return vis_data(); };
	virtual u32			getType() { return 0; };

#ifdef DEBUG
	virtual shared_str	getDebugName() { return ""; };
#endif

	glRender_Visual();
};

