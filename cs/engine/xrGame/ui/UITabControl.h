#pragma once

#include "uiwindow.h"
#include "../../xrServerEntities/script_export_space.h"
#include "UIOptionsItem.h"

class CUITabButton;
class CUIButton;

DEF_VECTOR (TABS_VECTOR, CUITabButton*)

class CUITabControl: public CUIWindow, public CUIOptionsItem 
{
	typedef				CUIWindow inherited;
public:
						CUITabControl				();
	virtual				~CUITabControl				();

	// options item
	virtual void		SetCurrentValue				();
	virtual void		SaveValue					();
	virtual bool		IsChanged					();

	virtual bool		OnKeyboard					(int dik, EUIMessages keyboard_action);
	virtual void		OnTabChange					(const shared_str& sCur, const shared_str& sPrev);
	virtual void		OnStaticFocusReceive		(CUIWindow* pWnd);
	virtual void		OnStaticFocusLost			(CUIWindow* pWnd);

	// Добавление кнопки-закладки в список закладок контрола
	bool				AddItem						(LPCSTR pItemName, LPCSTR pTexName, Fvector2 pos, Fvector2 size);
	bool				AddItem						(CUITabButton *pButton);

//.	void				RemoveItem					(const shared_str& Id);
	void				RemoveAll					();

	virtual void		SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);

	const shared_str&	GetActiveId					()								{ return m_sPushedId; }
	LPCSTR				GetActiveId_script			();
	const shared_str&	GetPrevActiveId				()								{ return m_sPrevPushedId; }
			void		SetActiveTab				(const shared_str& sNewTab);
			void		SetActiveTab_script			(LPCSTR sNewTab)				{SetActiveTab(sNewTab);};
	const	u32			GetTabsCount				() const						{ return m_TabsArr.size(); }
	
	// Режим клавилатурных акселераторов (вкл/выкл)
	IC bool				GetAcceleratorsMode			() const						{ return m_bAcceleratorsEnable; }
	void				SetAcceleratorsMode			(bool bEnable)					{ m_bAcceleratorsEnable = bEnable; }


	TABS_VECTOR *		GetButtonsVector			()								{ return &m_TabsArr; }
	CUITabButton*		GetButtonById				(const shared_str& id);
	CUITabButton*		GetButtonById_script		(LPCSTR s)						{ return GetButtonById(s);}
//.	const shared_str	GetCommandName				(const shared_str& id);
//.	CUITabButton*		GetButtonByCommand			(const shared_str& n);

	void		ResetTab					();
protected:
	// Список кнопок - переключателей закладок
	TABS_VECTOR			m_TabsArr;

	shared_str			m_sPushedId;
	shared_str			m_sPrevPushedId;
// Текущая нажатая кнопка. -1 - ни одна, 0 - первая, 1 - вторая, и т.д.
//.	int					m_iPushedIndex;
//.	int					m_iPrevPushedIndex;

	// Цвет неактивных элементов
	u32					m_cGlobalTextColor;
	u32					m_cGlobalButtonColor;

	// Цвет надписи на активном элементе
	u32					m_cActiveTextColor;
	u32					m_cActiveButtonColor;

	bool				m_bAcceleratorsEnable;
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CUITabControl)
#undef script_type_list
#define script_type_list save_type_list(CUITabControl)
