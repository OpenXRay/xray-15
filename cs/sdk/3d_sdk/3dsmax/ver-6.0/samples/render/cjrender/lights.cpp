//***************************************************************************
// CJRender - [lights.cpp] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// Description:
// Implementation of the RenderLights and Light/Shadow support classes
//
//***************************************************************************


#include "maxincl.h"
#include "cjrender.h"
#include "rendutil.h"
#include "refenum.h"



//***************************************************************************
// This is a class designed purely for convenience.
// We need to keep track of the LightObjects, the light nodes
// and the LightObject descriptors. In order to keep them synched
// I'm collecting them in a class for themselves.
//***************************************************************************

//***************************************************************************
// Constructor of RenderLight
// The constructor will get the light objects and the
// light object descriptor from the light node we pass in.
//***************************************************************************

RenderLight::RenderLight(INode* node, MtlBaseLib* mtls)
{
	pNode = node;

	ObjectState ostate = pNode->EvalWorldState(0);

	pLight = (LightObject*)ostate.obj;
	pDesc = pLight->CreateLightDesc(pNode);

	// Process maps
	GetMaps getmaps(mtls);
	EnumRefs(pLight,getmaps);

	DebugPrint("Created RenderLight for: %s\n", node->GetName());
}


//***************************************************************************
// Alternative constructor used for default lights
//***************************************************************************

RenderLight::RenderLight(DefaultLight* l)
{
	pNode = NULL;
	pLight = NULL;
	pDesc = new DefObjLight(l);
	DebugPrint("Created RenderLight for default light.\n");
}

//***************************************************************************
// Nothing to do in the destructor
//***************************************************************************

RenderLight::~RenderLight()
{
}

//***************************************************************************
// Call update on the Light descriptor
//***************************************************************************

void RenderLight::Update(TimeValue t, CJRenderer* renderer)
{
	RContext rc(renderer);

	// TRUE means use shadows, but we only have a dummy implementation of
	// the ShadowBuffer in this renderer. This dummy implementation always
	// reports that the point is not shadowed.
	// To use an implementation similar to the Max default scanline
	// renderer you need to fully implement the ShadowBuffer class
	// see cjshade.cpp for the dummy implementation.
	pDesc->Update(t, rc, &renderer->rendParams, TRUE, FALSE);
}


//***************************************************************************
// Update view dependent parameters
//***************************************************************************

void RenderLight::UpdateViewDepParams(Matrix3 world2cam)
{
	pDesc->UpdateViewDepParams(world2cam);
}


/****************************************************************************
// RContext
 ***************************************************************************/

Color RContext::GlobalLightLevel() const
{
	return renderer->rendParams.pFrp->globalLightLevel;
}


//***************************************************************************
// Light Descriptor for the default lights
//***************************************************************************

DefObjLight::DefObjLight(DefaultLight *l) : ObjLightDesc(NULL)
{
	inode = NULL;
	bViewOriented = true;
	ls = l->ls;

	// New for R3
	// If the TM is all 0, then the light is view dependent and is always
	// supposed to be located at the camera position, looging at the scene.
	// This happens when the user has the "one light" viewport options turned on btw.
	for (int i=0; i<4; i++) {
		Point3 p = l->tm.GetRow(i);
		if (p.x != 0 || p.y != 0 || p.z != 0)
			bViewOriented = false;
		}

	if (!bViewOriented) {
		lightToWorld = l->tm;
		worldToLight = Inverse(lightToWorld);
		}
	}


//***************************************************************************
// Update
//***************************************************************************

int DefObjLight::Update(TimeValue t, const RendContext& rc, RenderGlobalContext* rgc, BOOL shadows, BOOL shadowGeomChanged)
{
	if (bViewOriented) {
		if (rgc) {
			lightToWorld = rgc->camToWorld;
			worldToLight = Inverse(lightToWorld);
			}
		}

	intensCol  = ls.intens*ls.color*rc.GlobalLightLevel();
	return 1;
}


//***************************************************************************
// Update viewdependent parameters
//***************************************************************************

int DefObjLight::UpdateViewDepParams(const Matrix3& worldToCam)
	{
	lightToCam = lightToWorld * worldToCam;
	camToLight = Inverse(lightToCam);
	lightPos   = lightToCam.GetRow(3);  // light pos in camera space
	lightDir   = Normalize(lightToCam.GetRow(2));
	return 1;
	}


//***************************************************************************
// Illuminate method for default lights
//***************************************************************************

BOOL DefObjLight::Illuminate(ShadeContext& sc, Point3& normal, Color& color, Point3 &dir, float &dot_nl, float& diffuseCoef)
	{
	color = intensCol;

	if (ls.type==DIRECT_LGT) {
		diffuseCoef = dot_nl = DotProd(normal, lightDir);
		dir = lightDir;
		}
	else {
		dir = Normalize(lightPos-sc.P());
		diffuseCoef = dot_nl = DotProd(normal,dir);
		}

	return (dot_nl<=0.0f)?0:1;
}
