/**********************************************************************
 
	FILE: iSkinPose.h

	DESCRIPTION:  Public interface for setting and getting a special,
	non-animated, transformation pose, SkinPose.

	CREATED BY: Jianmin Zhao, Discreet

	HISTORY: - created November 09, 2001

 *>	Copyright (c) 1998-2001, All Rights Reserved.
 **********************************************************************/

#ifndef __ISKINPOSE__H
#define __ISKINPOSE__H

#include "iFnPub.h"
#include "maxtypes.h"

#define SKINPOSE_INTERFACE Interface_ID(0x66f226de, 0x37ff3962)

class ISkinPose : public FPMixinInterface {
public:
	// Utilities:
	static ISkinPose* GetISkinPose(INode& n) {
		return static_cast<ISkinPose*>(n.GetInterface(SKINPOSE_INTERFACE)); }

	virtual Point3	SkinPos() const =0;
	virtual RotationValue SkinRot() const=0;
	virtual ScaleValue SkinScale() const =0;
	virtual void	SetSkinPos(const Point3&) =0;
	virtual void	SetSkinRot(const RotationValue&) =0;
	// Set the skin rotation via Euler angles:
	virtual void	SetSkinRot(const Point3&) =0;
	virtual void	SetSkinScaleFactors(const Point3&) =0;
	virtual void	SetSkinScaleOrient(const Quat&) =0;

	virtual bool	IsSkinPosEnabled() const =0;
	virtual bool	IsSkinRotEnabled() const =0;
	virtual bool	IsSkinScaleEnabled() const =0;
	virtual bool	SkinPoseMode() const =0;
	virtual void	EnableSkinPos(bool) =0;
	virtual void	EnableSkinRot(bool) =0;
	virtual void	EnableSkinScale(bool) =0;
	virtual void	SetSkinPoseMode(bool) =0;

	virtual void	SetSkinPose(TimeValue) =0;
	virtual void	AssumeSkinPose(TimeValue) =0;
	virtual void	TMSetValue(TimeValue, SetXFormPacket&) =0;

	// Derived methods. They are all inline'd at the end of the file.
	//
	// SkinRotAngles() returns Euler angles of order XYZ.
	Point3			SkinRotAngles() const;
	Point3			SkinScaleFactors() const;
	Quat			SkinScaleOrient() const;
	void			SetSkinScale(const ScaleValue& sv);
	void			SetSkinRotAngles(const Point3&);
	bool			ShowSkinPos() const;
	bool			ShowSkinRot() const;
	bool			ShowSkinScale() const;
	bool			IsACompEnabled() const;

	virtual bool	 	NeedToSave() const =0;
	virtual IOResult	Save(ISave*) const =0;
	virtual IOResult	Load(ILoad*) =0;
	virtual void	 	Copy(const ISkinPose&) =0;

	// Used by implementation class:
	virtual const void* ObjectOf(void*) const =0;
	virtual		  void*	ObjectOf(void*) =0;

	// Due to FPMixinInterface:
	FPInterfaceDesc* GetDesc() { return GetDescByID(SKINPOSE_INTERFACE); }

	// Function publishings:
	//
	enum FuncID {
		kSkinPosGet, kSkinPosSet,
		kSkinRotGet, kSkinRotSet,
		kSkinScaleGet, kSkinScaleSet,
		kSkinScaleOrientGet, kSkinScaleOrientSet,
		// When SkinPosEnabled is false, SkinPoseMode, SetSkinPose, and
		// AssumeSkinPose will disregard the position component.
		// Similar to SkinRotEnabled and SkinScaleEnabled.
		kSkinPosEnabledGet, kSkinPosEnabledSet,
		kSkinRotEnabledGet, kSkinRotEnabledSet,
		kSkinScaleEnabledGet, kSkinScaleEnabledSet,
		kSkinPoseModeGet, kSkinPoseModeSet,
		kSetSkinPose,
		kAssumeSkinPose
	};

BEGIN_FUNCTION_MAP
	PROP_FNS(kSkinPosGet, SkinPos, kSkinPosSet, SetSkinPos, TYPE_POINT3_BV)
	PROP_FNS(kSkinRotGet, SkinRotAngles, kSkinRotSet, SetSkinRotAngles, TYPE_POINT3_BV)
	PROP_FNS(kSkinScaleGet, SkinScaleFactors, kSkinScaleSet, SetSkinScaleFactors, TYPE_POINT3_BV)
	PROP_FNS(kSkinScaleOrientGet, SkinScaleOrient, kSkinScaleOrientSet, SetSkinScaleOrient, TYPE_QUAT_BV)
	PROP_FNS(kSkinPosEnabledGet, IsSkinPosEnabled, kSkinPosEnabledSet, EnableSkinPos, TYPE_bool)
	PROP_FNS(kSkinRotEnabledGet, IsSkinRotEnabled, kSkinRotEnabledSet, EnableSkinRot, TYPE_bool)
	PROP_FNS(kSkinScaleEnabledGet, IsSkinScaleEnabled, kSkinScaleEnabledSet, EnableSkinScale, TYPE_bool)
	PROP_FNS(kSkinPoseModeGet, SkinPoseMode, kSkinPoseModeSet, SetSkinPoseMode, TYPE_bool)
	VFNT_0(kSetSkinPose, SetSkinPose)
	VFNT_0(kAssumeSkinPose, AssumeSkinPose)
END_FUNCTION_MAP
};

// inlines for derived methods
//
inline Point3 ISkinPose::SkinRotAngles() const
{
	return SkinRot().Euler() * RAD_TO_DEG;
}

inline Point3 ISkinPose::SkinScaleFactors() const
{
	return SkinScale().s;
}

inline Quat ISkinPose::SkinScaleOrient() const
{
	return SkinScale().q;
}

inline void ISkinPose::SetSkinScale(const ScaleValue& sv)
{
	SetSkinScaleFactors(sv.s);
	SetSkinScaleOrient(sv.q);
}

inline void	ISkinPose::SetSkinRotAngles(const Point3& p)
{
	SetSkinRot(p * DEG_TO_RAD);
}

inline bool ISkinPose::ShowSkinPos() const
{
	return SkinPoseMode() && IsSkinPosEnabled();
}

inline bool ISkinPose::ShowSkinRot() const
{
	return SkinPoseMode() && IsSkinRotEnabled();
}

inline bool ISkinPose::ShowSkinScale() const
{
	return SkinPoseMode() && IsSkinScaleEnabled();
}

inline bool ISkinPose::IsACompEnabled() const
{
	return (IsSkinPosEnabled() || IsSkinRotEnabled() || IsSkinScaleEnabled());
}

#endif
