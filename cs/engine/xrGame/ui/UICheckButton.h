#pragma once

#include "ui3tbutton.h"
#include "UIOptionsItem.h"

class UIHint;

class CUICheckButton : public CUI3tButton, public CUIOptionsItem
{
	typedef CUI3tButton			inherited;

public:
					CUICheckButton			();
	virtual			~CUICheckButton			();

	virtual void	Update					();

	// CUIOptionsItem
	virtual void	SetCurrentValue			();
	virtual void	SaveValue				();
	virtual bool	IsChanged				();
	virtual void 	SeveBackUpValue			();
	virtual void 	Undo					();

	virtual void 	OnFocusReceive		();
	virtual void	OnFocusLost			();
	virtual void	Show				( bool status );
	virtual bool	OnMouseDown			( int mouse_btn );

			void InitCheckButton		(Fvector2 pos, Fvector2 size, LPCSTR texture_name);
			void init_hint_wnd_xml		( CUIXml& xml, LPCSTR path );

//	virtual void SetTextX(float x) {/*do nothing*/}

			void	set_hint_wnd			(UIHint* hint_wnd);

	//состояние кнопки
	IC	bool	GetCheck()
	{
		return m_eButtonState == BUTTON_PUSHED;
	}
	IC	void	SetCheck(bool ch)
	{
		m_eButtonState = ch ? BUTTON_PUSHED : BUTTON_NORMAL;
		SeveBackUpValue();
	}

	void SetDependControl(CUIWindow* pWnd);

private:
	bool			b_backup_val;
	void InitTexture2				(LPCSTR texture_name);
	CUIWindow* m_pDependControl;

protected:
	UIHintWindow*	m_hint_owner;
};