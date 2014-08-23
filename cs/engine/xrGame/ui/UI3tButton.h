// File:        UI3tButton.cpp
// Description: Button with 3 texutres (for <enabled>, <disabled> and <touched> states)
// Created:     07.12.2004
// Author:      Serhiy 0. Vynnychenk0
// Mail:        narrator@gsc-game.kiev.ua
//
// copyright 2004 GSC Game World
//

#pragma once
#include "UIButton.h"
#include "UI_IB_Static.h"

class CUI3tButton : public CUIButton 
{
	typedef CUIButton	inherited;
	friend class CUIXmlInit;
	using CUIButton::SetTextColor;
public:
					CUI3tButton					();
	virtual			~CUI3tButton				();
	// appearance

	virtual	void 	InitButton					(Fvector2 pos, Fvector2 size);
	virtual void 	InitTexture					(LPCSTR tex_name);
	virtual void 	InitTexture					(LPCSTR tex_enabled, LPCSTR tex_disabled, LPCSTR tex_touched, LPCSTR tex_highlighted);

			void 	SetTextColor				(u32 color);
			void 	SetTextColorH				(u32 color);
			void 	SetTextColorD				(u32 color);
			void 	SetTextColorT				(u32 color);
	virtual void 	SetTextureOffset			(float x, float y);	
	virtual void 	SetWidth					(float width);
	virtual void 	SetHeight					(float height);
			void 	InitSoundH					(LPCSTR sound_file);
			void 	InitSoundT					(LPCSTR sound_file);

	virtual void 	OnClick						();
	virtual void 	OnFocusReceive				();
	virtual void	OnFocusLost					();

	/*// check button
	IC bool			GetCheck					()
	{
		return m_eButtonState == BUTTON_PUSHED;
	}
	IC void			SetCheck					(bool ch)
	{
		m_eButtonState = ch ? BUTTON_PUSHED : BUTTON_NORMAL;
		SeveBackUpValue();
	}*/
	
	// behavior
	virtual void	DrawTexture					();
	virtual void	Update						();
	virtual void 	Draw						();
	
	//virtual void Enable(bool bEnable);	
	virtual bool 	OnMouse						(float x, float y, EUIMessages mouse_action);
	virtual bool 	OnMouseDown					(int mouse_btn);
			void 	SetCheckMode				(bool mode) {m_bCheckMode = mode;}


	CUIStatic*				m_hint;
	bool					m_frameline_mode;
	CUI_IB_Static*			m_background;
	CUI_IB_FrameLineWnd*	m_back_frameline;
	void					CreateHint				();
protected:
	bool				m_bCheckMode;
private:	
			void		PlaySoundH					();
			void		PlaySoundT					();

	ref_sound			m_sound_h;
	ref_sound			m_sound_t;	

}; // class CUI3tButton

// ---------------------------------------------------------------------
class UIHintWindow;
class UIHint;

class CUI3tButtonEx : public CUI3tButton
{
	typedef CUI3tButton inherited;
public:
					CUI3tButtonEx		();
	virtual			~CUI3tButtonEx		();

			void	init_from_xml		(CUIXml& xml, LPCSTR path);
			void	set_hint_wnd		(UIHint* hint_wnd);

	virtual void 	OnFocusReceive		();
	virtual void	OnFocusLost			();

	virtual void	Update				();
	virtual void	Show				( bool status );
	virtual bool	OnMouseDown			( int mouse_btn );

protected:
	UIHintWindow*	m_hint_owner;

}; // class CUI3tButtonEx
