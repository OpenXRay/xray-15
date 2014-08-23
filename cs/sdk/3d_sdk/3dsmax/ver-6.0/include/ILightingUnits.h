/**********************************************************************
 *<
	FILE: ILightingUnits.h

	DESCRIPTION: Function-published interface to access the lighting units display system.

	CREATED BY: David Cunningham

	HISTORY: October 15, 2001 file created

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef __ILIGHTUNIT__
#define __ILIGHTUNIT__


#include "iFnPub.h"
#define ILIGHT_UNITS_FO_INTERFACE Interface_ID(0x7246dc, 0x4384dc)

typedef struct	{
	int lightingSystem;		// Lighting unit display system {LIGHTING_UNIT_DISPLAY_INTERNATIONAL, 
							//											LIGHTING_UNIT_DISPLAY_AMERICAN }
	TSTR luminanceUnits;	// The string describing the luminance units
	TSTR illuminanceUnits;	// The string describing the illuminance units
}	LightDispInfo;

/**
 * The interface used to access the lighting system.
 */



class ILightingUnits : public FPStaticInterface {

public:

    enum LightingSystems {
    DISPLAY_INTERNATIONAL,
    DISPLAY_AMERICAN,
    DISPLAY_TYPE_COUNT
    };

    virtual TCHAR   *GetFirstLightingSystemName() = 0;
    virtual TCHAR   *GetNextLightingSystemName() = 0;   

    virtual int		GetLightingSystem() = 0;
    virtual void	SetLightingSystem(int system) = 0;
    virtual TCHAR	*GetLuminanceUnits() = 0;
    virtual TCHAR 	*GetIlluminanceUnits() = 0;
    virtual double	ConvertLuminanceToCurrSystem(double luminanceValue) = 0;
    virtual double	ConvertLuminanceToCurrSystem(double luminanceValue, int inputType) = 0;
    virtual double  ConvertLuminanceToSI(double AS_LuminanceValue) = 0;
    virtual double  ConvertLuminanceToAS(double SI_LuminanceValue) = 0;
    virtual double	ConvertIlluminanceToCurrSystem(double illuminanceValue) = 0;
    virtual double	ConvertIlluminanceToCurrSystem(double illuminanceValue, int inputType) = 0;
    virtual double  ConvertIlluminanceToSI(double AS_IlluminanceValue) = 0;
    virtual double  ConvertIlluminanceToAS(double SI_IlluminanceValue) = 0;
    virtual void	GetLightingUnitDisplayInfo(LightDispInfo* info) = 0;
    
};



#endif

