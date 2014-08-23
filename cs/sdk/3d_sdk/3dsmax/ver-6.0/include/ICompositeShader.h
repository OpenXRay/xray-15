//////////////////////////////////////////////////////////////////////////////
//
//		Composite Shader Interface
//
//		Created: 9/27/01 Cleve Ard
//
#ifndef	ICOMPOSITESHADERS_H
#define ICOMPOSITESHADERS_H

#include "iFnPub.h"

// ISpecularCompositeShader is used to let a shader choose the manner
// that it combines specular and diffuse lighting. The shader can use
// any information in the RenderGlobalContext to make this decision.

// Several shaders, include anisotropic, translucent, and multilayer
// combine specular and diffuse lighting using this equation:
//   L = specular + (1 - specular) * diffuse
// This only works when 0 <= specular <= 1. These shaders use this
// interface to determine whether we are rendering with a tone operator.
// If the render is using a tone operator, then the specular and diffuse
// lighting is simply added for the total lighting.

// In order for a StdMtl2 to ask for and call this interface for a shader
// it must return MTLREQ_PREPRO in it's LocalRequirements method.

class ISpecularCompositeShader : public BaseInterface {
public:

	// Choose specular method
	virtual void ChooseSpecularMethod(TimeValue t, RenderGlobalContext* rgc) = 0;
};


#define ISPECULAR_COMPOSITE_SHADER_ID Interface_ID(0x5e2117d0, 0x327e2f73)

// Get the ISpecularCompositeShader method, if there is one.
inline ISpecularCompositeShader* GetSpecularCompositeShader(InterfaceServer* s)
{
	return static_cast<ISpecularCompositeShader*>(s->GetInterface(
		ISPECULAR_COMPOSITE_SHADER_ID));
}

// Choose the method for combining specular and diffuse lighting.
inline void ChooseSpecularMethod(InterfaceServer* s, TimeValue t, RenderGlobalContext* rgc)
{
	ISpecularCompositeShader* scs = GetSpecularCompositeShader(s);
	if (scs != NULL)
		scs->ChooseSpecularMethod(t, rgc);
}

#endif
