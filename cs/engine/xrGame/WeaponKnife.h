#pragma once

#include "WeaponCustomPistol.h"
#include "script_export_space.h"

class CWeaponKnife: public CWeapon {
private:
	typedef CWeapon inherited;

protected:

	virtual void		switch2_Idle				();
	virtual void		switch2_Hiding				();
	virtual void		switch2_Hidden				();
	virtual void		switch2_Showing				();
			void		switch2_Attacking			(u32 state);

	virtual void		OnAnimationEnd				(u32 state);
	virtual void		OnMotionMark				(u32 state, const motion_marks&);
	virtual void		OnStateSwitch				(u32 S);

	void				state_Attacking				(float dt);

	virtual void		KnifeStrike					(const Fvector& pos, const Fvector& dir);

	float				fWallmarkSize;
	u16					knife_material_idx;

protected:	
	ALife::EHitType		m_eHitType;

	ALife::EHitType		m_eHitType_1;
	Fvector4			fvHitPower_1;
	Fvector4			fvHitPowerCritical_1;
	float				fHitImpulse_1;

	ALife::EHitType		m_eHitType_2;
	Fvector4			fvHitPower_2;
	Fvector4			fvHitPowerCritical_2;
	float				fHitImpulse_2;

	float				fCurrentHit;
	float				fCurrentHitCritical;

	float				fHitImpulse_cur;

protected:
	virtual void		LoadFireParams					(LPCSTR section);
public:
						CWeaponKnife(); 
	virtual				~CWeaponKnife(); 

	void				Load							(LPCSTR section);

	virtual bool		IsZoomEnabled					()	const	{return false;}

			void		Fire2Start						();
	virtual void		FireStart						();


	virtual bool		Action							(s32 cmd, u32 flags);

	virtual void		GetBriefInfo					(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count, string16& fire_mode);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CWeaponKnife)
#undef script_type_list
#define script_type_list save_type_list(CWeaponKnife)
