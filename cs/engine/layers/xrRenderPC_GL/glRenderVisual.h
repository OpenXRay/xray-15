#pragma once

#include "../../Include/xrRender/RenderVisual.h"

class		ECORE_API			glRender_Visual : public IRenderVisual
{
public:
	// Common data for rendering
	u32							Type;				// visual's type
	vis_data					vis;				// visibility-data
	ref_shader					shader;				// pipe state, shared

	virtual vis_data&	getVisData() { return vis; };
	virtual u32			getType() { return Type; };

#ifdef DEBUG
	virtual shared_str	getDebugName() { return ""; };
#endif

	glRender_Visual();
};

