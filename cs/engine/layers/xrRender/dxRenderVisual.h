#ifndef RenderVisualH
#define RenderVisualH
#pragma once

#include "../../xrEngine/vis_common.h"

#include "../../Include/xrRender/RenderVisual.h"

#define VLOAD_NOVERTICES		(1<<0)
#define VLOAD_NOINDICES			(1<<1)
#define VLOAD_FORCESOFTWARE		(1<<2)

// The class itself
class					CKinematicsAnimated;
class					CKinematics;
class					IParticleCustom;

// The class itself
class		ECORE_API			dxRender_Visual : public IRenderVisual
{
public:
#ifdef _EDITOR
    ogf_desc					desc		;
#endif
#ifdef DEBUG
	shared_str					dbg_name	;
	virtual shared_str			getDebugName() { return dbg_name; }
#endif
public:
	// Common data for rendering
	u32							Type		;				// visual's type
	vis_data					vis			;				// visibility-data
	ref_shader					shader		;				// pipe state, shared

	virtual void				Render						(float LOD)		{};		// LOD - Level Of Detail  [0..1], Ignored
	virtual void				Load						(const char* N, IReader *data, u32 dwFlags);
	virtual void				Release						();						// Shared memory release
	virtual void				Copy						(dxRender_Visual* from);
	virtual void				Spawn						()				{};
	virtual void				Depart						()				{};

//	virtual	CKinematics*		dcast_PKinematics			()				{ return 0;	}
//	virtual	CKinematicsAnimated*dcast_PKinematicsAnimated	()				{ return 0;	}
//	virtual IParticleCustom*	dcast_ParticleCustom		()				{ return 0;	}

	virtual vis_data&	getVisData() { return vis;}
	virtual u32			getType()	 { return Type;}

	dxRender_Visual				();
	virtual ~dxRender_Visual		();
};

#endif // !RenderVisualH
