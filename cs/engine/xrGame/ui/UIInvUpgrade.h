////////////////////////////////////////////////////////////////////////////
//	Module 		: UIInvUpgrade.h
//	Created 	: 08.11.2007
//  Modified 	: 27.11.2007
//	Author		: Evgeniy Sokolov
//	Description : inventory upgrade UI class
////////////////////////////////////////////////////////////////////////////

#ifndef UI_INVENTORY_UPGRADE_H_INCLUDED
#define UI_INVENTORY_UPGRADE_H_INCLUDED

#include "UIStatic.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"


namespace inventory { namespace upgrade {
	class Upgrade;
	class Property;
} } // namespace upgrade, inventory

class CUIStatic;
class CUIInventoryUpgradeWnd;
class CInventoryItem;

class UIUpgrade : public CUIWindow
{
private:
	typedef inventory::upgrade::Upgrade 	Upgrade_type;
	typedef inventory::upgrade::Property 	Property_type;
	typedef	CUIWindow						inherited;

public:
	enum ButtonState
	{ 
		BUTTON_FREE = 0,
		BUTTON_PRESSED,
		BUTTON_DPRESSED,
		BUTTON_FOCUSED
	};

public:
	enum ViewState
	{
		STATE_ENABLED = 0,
		STATE_FOCUSED,   // HIGHLIGHT
		STATE_TOUCHED, 
		STATE_SELECTED,  // set = install
		STATE_UNKNOWN,
		
		STATE_DISABLED_PARENT,
		STATE_DISABLED_GROUP,
		STATE_DISABLED_PREC_MONEY,
		STATE_DISABLED_PREC_QUEST,

		STATE_COUNT
	};

	enum Layer
	{
		LAYER_ITEM = 0,
		LAYER_COLOR,
		LAYER_BORDER,
		LAYER_INK,
		LAYER_COUNT
	};

public:
	Fvector2		offset;

private:
	CUIInventoryUpgradeWnd*	m_parent_wnd;

	CUIStatic*		m_item;
	CUIStatic*		m_color;
	CUIStatic*		m_border;
	CUIStatic*		m_ink;

//	CUIStatic*		m_prop;

//	CInventoryItem*	m_inv_item;
	shared_str		m_upgrade_id;

protected:
	Ivector2		m_scheme_index; // [column,row]

//	bool			m_bButtonClicked;
	ButtonState		m_button_state;

	ViewState		m_state;
	ViewState		m_prev_state;

	bool			m_state_lock;

public:
						UIUpgrade( CUIInventoryUpgradeWnd* parent_wnd );
	virtual				~UIUpgrade();

			void		init_upgrade( LPCSTR upgrade_id, CInventoryItem& item );

			void		load_from_xml( CUIXml& ui_xml, int i_column, int i_cell, Frect const& t_cell_border, Frect const& t_cell_item );
			void		init_property( Fvector2 const& pos, Fvector2 const& size );
			void		set_texture( Layer layer, LPCSTR texture );
			
	virtual	void		Draw();
	virtual	void		Update();
	virtual	void		Reset();

			void		update_upgrade_state();
			void		update_mask();
			void		update_item( CInventoryItem* inv_item );

	virtual bool		OnMouse( float x, float y, EUIMessages mouse_action );
	virtual void		OnFocusReceive();
	virtual void		OnFocusLost();
	virtual void		OnClick();
	virtual bool 		OnDbClick();
			void		OnRClick();

			void		on_over_window();
			
			void		highlight_relation( bool enable );

	IC ButtonState		get_button_state() const { return m_button_state; }
	IC Ivector2 const&	get_scheme_index() const  { return m_scheme_index; }
//	IC LPCSTR			get_upgrade_name() const  { return m_upgrade_name; }

	Upgrade_type*		get_upgrade();
//	Property_type*		get_property();

};

#endif // UI_INVENTORY_UPGRADE_H_INCLUDED
