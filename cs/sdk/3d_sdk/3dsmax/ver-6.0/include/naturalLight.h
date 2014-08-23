/**********************************************************************
 
	FILE: naturalLight.h

	DESCRIPTION:  Natural Light definitions

	CREATED BY: Cleve Ard, Discreet

	HISTORY: - created July 24, 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef __INATURALLIGHT__H
#define __INATURALLIGHT__H

// Natural light is supplied by the interaction of several objects:

// SUN LIGHTS:

// Direct light
// IES sunlight
// CIE sunlight

// SKY LIGHTS:

// Standard Skylight
// IES skylight
// CIE skylight
// Isotropic skylight (why)
// Environmemnt map skylight
// HDR environment map skylight

// SUN POSITION CONTROLLERS

// MAX Sun position
// IES Sun position
// CIE Sun position

// NATURAL LIGHT SYSTEM

// The natural light system combines these three objects to form natural
// light. It uses a 

/*===========================================================================*\
 | Natural Light Class Interface:
\*===========================================================================*/

// This interface is used to tell the natural light assembly that the
// objects created by the class are natural light objects. This interface
// is used to decide when the classes are to be displayed in the
// drop down lists for the natural light assembly.

#define NATURAL_LIGHT_CLASS_INTERFACE_ID Interface_ID(0x75985ea5, 0x115c2791)

// A sun light is just a direct light. The intensities and
class INaturalLightClass : public FPStaticInterface {
public:
	// Function publishing enum
	enum {
		natLightIsSun = 0,
		natLightIsSky = 1
	};

	DECLARE_DESCRIPTOR(INaturalLightClass)

	BEGIN_FUNCTION_MAP
		FN_0(natLightIsSun, TYPE_BOOL, IsSun)
		FN_0(natLightIsSky, TYPE_BOOL, IsSky)
	END_FUNCTION_MAP

	// Is this class a sun?
	virtual BOOL IsSun() const = 0;

	// Is this class a sky?
	virtual BOOL IsSky() const = 0;
};

inline INaturalLightClass* GetNaturalLightClass(SClass_ID s, const Class_ID& id)
{
	return static_cast<INaturalLightClass*>(GetInterface(s, id,
		NATURAL_LIGHT_CLASS_INTERFACE_ID));
}

inline INaturalLightClass* GetNaturalLightClass(Animatable* a)
{
	return a == NULL ? NULL
		: GetNaturalLightClass(a->SuperClassID(), a->ClassID());
}

// This class is used to set up the function publishing
class NaturalLightClassBaseImp : public INaturalLightClass {
public:
	NaturalLightClassBaseImp(ClassDesc* cd, int classStrID, int isSunStrID, int isSkyStrID)
		: INaturalLightClass(
			NATURAL_LIGHT_CLASS_INTERFACE_ID, _T("NaturalLightClass"),
				classStrID, cd, FP_STATIC_METHODS,
			natLightIsSun, _T("isSun"), isSunStrID, TYPE_BOOL, 0, 0,
			natLightIsSky, _T("isSky"), isSkyStrID, TYPE_BOOL, 0, 0,
			end) {}
};

inline INaturalLightClass* GetNaturalLightClassInterface(ClassDesc* c)
{
	return static_cast<INaturalLightClass*>(c->GetInterface(NATURAL_LIGHT_CLASS_INTERFACE_ID));
}

#define SUNLIGHT_INTERFACE_ID Interface_ID(0x43b76ff2, 0x60ae0d61)

// A sun light is just a direct light with a constant intensity.
class ISunLight : public BaseInterface {
public:
	// Return whether the intensity is in MAX Units. If it isn't
	// in MAX units it must be in international units.
	virtual bool IsIntensityInMAXUnits() const = 0;
};

inline ISunLight* GetSunLightInterface(Animatable* o)
{
	return static_cast<ISunLight*>(o->GetInterface(SUNLIGHT_INTERFACE_ID));
}

// This class is used to evaluate the sky for Flow radiosity.
class SkyLightEval : public BaseInterface {
public:
	// Delete yourself
	virtual void DeleteThis() = 0;

	// Returns the spectral radiance along a ray of given origin
	// and direction in world space. Return true if the intensity is in
	// MAX units, or false if intensity is in international units
	virtual bool Gather(
		const Point3&		origin,
		const Point3&		direction,
		Point3&				radiance
	) = 0;

	// Return the class of the sky that created you.
	virtual Class_ID ClassID() const = 0;

	// Compare two skys. Return true if the Gather method
	// will return the same result.
	virtual bool IsSameSky( const SkyLightEval* sky ) const = 0;
};

#define SKYLIGHT_INTERFACE_ID Interface_ID(0x47056297, 0x7f8b06e3)

// This interface is used to create the
class ISkyLight : public BaseInterface {
public:
	// Create a SkyLightEval that can be used to evaluate
	// this skylight for the radiosity solution. Tm transforms
	// (0,0,1) to the zenith direction for the scene.
	virtual SkyLightEval* CreateSkyEval(
		TimeValue		t,
		INode*			node,
		const Matrix3&	tm
	) = 0;

	// Return whether the intensity is in MAX Units. If it isn't
	// in MAX units it must be in international units.
	virtual bool IsIntensityInMAXUnits() const = 0;
};

inline ISkyLight* GetSkyLightInterface(Animatable* o)
{
	return static_cast<ISkyLight*>(o->GetInterface(SKYLIGHT_INTERFACE_ID));
}


#define SUNLIGHT_POSITION_INTERFACE_ID Interface_ID(0x6fa56707, 0x4ebe3d73)

class ISunLightPosition : public BaseInterface {
public:
};

inline ISunLightPosition* GetSunLightPositionInterface(Animatable* o)
{
	return static_cast<ISunLightPosition*>(o->GetInterface(SUNLIGHT_POSITION_INTERFACE_ID));
}

#endif
