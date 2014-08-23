/**********************************************************************
 *<
	FILE: IEmissionColor.h

	DESCRIPTION: Emission Color Extension Interface

	CREATED BY: Norbert Jeske

	HISTORY: Created 08/22/01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _EMISSION_COLOR_H_
#define _EMISSION_COLOR_H_

#define EMISSION_COLOR_INTERFACE_ID Interface_ID(0x4f803aa8, 0x71611798)

class Point3;

class IEmissionColor : public BaseInterface
{
public:
	virtual Interface_ID	GetID() { return EMISSION_COLOR_INTERFACE_ID; }

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	virtual void SetColor(Point3 color)=0;
	virtual Point3 GetColor()=0;

	virtual void SetAlpha(float alpha)=0;
	virtual float GetAlpha()=0;
};

#endif