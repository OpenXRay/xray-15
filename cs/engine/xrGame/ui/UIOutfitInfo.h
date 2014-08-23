#pragma once

#include "UIWindow.h"
#include "UIDoubleProgressBar.h"
#include "..\..\xrServerEntities\alife_space.h"

class CCustomOutfit;
class CUIStatic;
class CUIDoubleProgressBar;
class CUIXml;

class CUIOutfitImmunity : public CUIWindow
{
public:
					CUIOutfitImmunity	();
	virtual			~CUIOutfitImmunity	();

			void	InitFromXml			( CUIXml& xml_doc, LPCSTR base_str, u32 hit_type );
			void	SetProgressValue	( float cur, float comp );

protected:
	CUIStatic				m_name; // texture + name
	CUIDoubleProgressBar	m_progress;
	CUIStatic				m_value; // 100%
	float					m_magnitude;

}; // class CUIOutfitImmunity

// -------------------------------------------------------------------------------------

class CUIOutfitInfo : public CUIWindow
{
public:
					CUIOutfitInfo		();
	virtual			~CUIOutfitInfo		();

			void 	InitFromXml			( CUIXml& xml_doc );
			void 	UpdateInfo			( CCustomOutfit* cur_outfit, CCustomOutfit* slot_outfit = NULL );	
protected:
	/*enum
	{
		item_burn_immunity = 0,				// ALife::eHitTypeBurn,
//		item_strike_immunity,				// ALife::eHitTypeShock,
		item_shock_immunity,				// ALife::eHitTypeStrike,
//		item_wound_immunity,				// ALife::eHitTypeWound,
		item_radiation_immunity,			// ALife::eHitTypeRadiation,
		item_telepatic_immunity,			// ALife::eHitTypeTelepatic,
		item_chemical_burn_immunity,		// ALife::eHitTypeChemicalBurn,
//		item_explosion_immunit,				// ALife::eHitTypeExplosion,
		item_fire_wound_immunity,			// ALife::eHitTypeFireWound,
		max_item_index,
	};*/
	enum				{ max_count = ALife::eHitTypeMax-2 };
	
	CUIStatic*			m_caption;
	CUIOutfitImmunity*	m_items[max_count];

}; // class CUIOutfitInfo
