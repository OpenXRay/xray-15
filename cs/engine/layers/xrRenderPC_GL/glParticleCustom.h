#pragma once

#include "../../Include/xrRender/ParticleCustom.h"
#include "../xrRender/FBasicVisual.h"

class glParticleCustom :
	public glRender_Visual, public IParticleCustom
{
public:
	// geometry-format
	ref_geom		geom;
public:
	glParticleCustom();
	virtual ~glParticleCustom();

	virtual IParticleCustom*	dcast_ParticleCustom()				{ return this; }
};
