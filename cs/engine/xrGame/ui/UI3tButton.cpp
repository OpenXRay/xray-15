#include "StdAfx.h"
#include "UI3tButton.h"
#include "UIXmlInit.h"
#include "UIHint.h"

CUI3tButton::CUI3tButton()
{
	m_bTextureEnable	= false;
	m_bUseTextColor[D]	= true;
	m_bUseTextColor[H]	= false;
	m_bUseTextColor[T]	= false;	

	m_dwTextColor[E] 	= 0xFFFFFFFF;
	m_dwTextColor[D] 	= 0xFFAAAAAA;
	m_dwTextColor[H] 	= 0xFFFFFFFF;
	m_dwTextColor[T] 	= 0xFFFFFFFF;

	m_background		= NULL;
	m_back_frameline	= NULL;
	m_frameline_mode	= false;
	
	m_bEnableTextHighlighting = false;
	m_bCheckMode		= false;
	SetPushOffset		(Fvector2().set(0.0f,0.0f) );
	m_hint				= NULL;
}

void CUI3tButton::CreateHint()
{
	m_hint				= xr_new<CUIStatic>();
	m_hint->SetAutoDelete(true);
	m_hint->SetCustomDraw(true);
	AttachChild			(m_hint);
}

CUI3tButton::~CUI3tButton()
{
}

void CUI3tButton::OnClick()
{
    CUIButton::OnClick	();
    PlaySoundT			();
}

bool CUI3tButton::OnMouse(float x, float y, EUIMessages mouse_action)
{
	if (m_bCheckMode)
		return CUIWindow::OnMouse(x,y,mouse_action);
	else
		return CUIButton::OnMouse(x,y,mouse_action);
}

bool CUI3tButton::OnMouseDown(int mouse_btn)
{
	if (m_bCheckMode)
	{
		if (mouse_btn==MOUSE_1)
		{
			if (m_eButtonState == BUTTON_NORMAL)
				m_eButtonState = BUTTON_PUSHED;
			else
				m_eButtonState = BUTTON_NORMAL;
		}
		GetMessageTarget()->SendMessage(this, BUTTON_CLICKED, NULL);
		return true;
	}
	else
		return CUIButton::OnMouseDown(mouse_btn);
}

void CUI3tButton::OnFocusLost()
{
	CUIButton::OnFocusLost();
/*	if ( m_eButtonState == BUTTON_PUSHED )
	{
		m_eButtonState = BUTTON_NORMAL;
	}
*/
//.	if(BUTTON_PUSHED == m_eButtonState)
//.		m_eButtonState = BUTTON_NORMAL;
}

void CUI3tButton::OnFocusReceive()
{
	CUIButton::OnFocusReceive	();
	PlaySoundH					();
}

void CUI3tButton::InitSoundH(LPCSTR sound_file)
{
	::Sound->create		(m_sound_h, sound_file,st_Effect,sg_SourceType);
}

void CUI3tButton::InitSoundT(LPCSTR sound_file)
{
	::Sound->create		(m_sound_t, sound_file,st_Effect,sg_SourceType); 
}

void CUI3tButton::PlaySoundT()
{
	if (m_sound_t._handle())
        m_sound_t.play(NULL, sm_2D);
}

void CUI3tButton::PlaySoundH()
{
	if (m_sound_h._handle())
		m_sound_h.play(NULL, sm_2D);
}

void CUI3tButton::InitButton(Fvector2 pos, Fvector2 size)
{
	if ( m_frameline_mode )
	{
		if ( !m_back_frameline )
		{
			m_back_frameline = xr_new<CUI_IB_FrameLineWnd>();
			m_back_frameline->SetAutoDelete	(true);
			AttachChild						(m_back_frameline);
		}
		m_back_frameline->SetWndPos		(Fvector2().set(0,0));
		m_back_frameline->SetWndSize	(size);
	}
	else
	{
		if ( !m_background )
		{
			m_background = xr_new<CUI_IB_Static>();
			m_background->SetAutoDelete		(true);
			AttachChild						(m_background);
		}
		m_background->SetWndPos			(Fvector2().set(0,0));
		m_background->SetWndSize		(size);
	}
    CUIButton::SetWndPos			(pos);
    CUIButton::SetWndSize			(size);
}

void CUI3tButton::SetWidth(float width)
{
	CUIButton::SetWidth			(width);
	if ( m_background )				{	m_background->SetWidth		(width);	}
	else if ( m_back_frameline )	{	m_back_frameline->SetWidth	(width);	}
}

void CUI3tButton::SetHeight(float height)
{
	CUIButton::SetHeight		(height);
	if ( m_background )	{		m_background->SetHeight		(height);	}
	else if ( m_back_frameline )	{	m_back_frameline->SetHeight	(height);	}
}

void CUI3tButton::InitTexture(LPCSTR tex_name)
{
	string_path 		tex_enabled;
	string_path 		tex_disabled;
	string_path 		tex_touched;
	string_path 		tex_highlighted;

	// enabled state texture
	strcpy_s				(tex_enabled,    tex_name);
	strcat				(tex_enabled,   "_e");

	// pressed state texture
	strcpy_s				(tex_disabled,   tex_name);
	strcat				(tex_disabled,   "_d");

	// touched state texture
	strcpy_s				(tex_touched, tex_name);
	strcat				(tex_touched, "_t");

	// touched state texture
	strcpy_s				(tex_highlighted, tex_name);
	strcat				(tex_highlighted, "_h");

	this->InitTexture	(tex_enabled, tex_disabled, tex_touched, tex_highlighted);		
}

void CUI3tButton::InitTexture(LPCSTR tex_enabled, 
							  LPCSTR tex_disabled, 
							  LPCSTR tex_touched, 
							  LPCSTR tex_highlighted)
{
	if ( m_background )
	{
		m_background->InitEnabledState		(tex_enabled);
		m_background->InitDisabledState		(tex_disabled);
		m_background->InitTouchedState		(tex_touched);
		m_background->InitHighlightedState	(tex_highlighted);
	}
	else if ( m_back_frameline )
	{
		m_back_frameline->InitEnabledState		(tex_enabled);
		m_back_frameline->InitDisabledState		(tex_disabled);
		m_back_frameline->InitTouchedState		(tex_touched);
		m_back_frameline->InitHighlightedState	(tex_highlighted);
	}

	this->m_bTextureEnable = true;
}

void CUI3tButton::SetTextColor(u32 color)
{
    m_dwTextColor[E] = color;
}

void CUI3tButton::SetTextColorD(u32 color)
{
	SetTextColor(color, CUIStatic::D);
}

void CUI3tButton::SetTextColorH(u32 color)
{
	SetTextColor(color, CUIStatic::H);
}

void CUI3tButton::SetTextColorT(u32 color)
{
	SetTextColor(color, CUIStatic::T);
}

void CUI3tButton::SetTextureOffset(float x, float y)
{
	if ( m_background )
	{
		this->m_background->SetTextureOffset(x, y);
	}
}

void  CUI3tButton::Draw()
{
	inherited::Draw();

	if(m_hint)
		m_hint->Draw		();
}

void CUI3tButton::DrawTexture()
{
	if ( m_bTextureEnable )
	{
		if ( m_background )				
		{
			m_background->SetStretchTexture(true/*GetStretchTexture()*/);
			m_background->Draw();		
		}else if ( m_back_frameline )	
		{	
			m_back_frameline->Draw();	
		}
	}
}

void CUI3tButton::Update()
{
	CUIButton::Update();

	if ( m_bTextureEnable )
	{
		if ( !m_bIsEnabled )
		{
			if ( m_background )				{	m_background->SetState( S_Disabled );	}
			else if ( m_back_frameline )	{	m_back_frameline->SetState( S_Disabled ); }
		}
		else if ( CUIButton::BUTTON_PUSHED == m_eButtonState )
		{
			if ( m_background )				{	m_background->SetState( S_Touched );		}
			else if ( m_back_frameline )	{	m_back_frameline->SetState( S_Touched );	}
		}
		else if ( m_bCursorOverWindow )
		{
			if ( m_background )				{	m_background->SetState( S_Highlighted );		}
			else if ( m_back_frameline )	{	m_back_frameline->SetState( S_Highlighted );	}
		}
		else
		{
			if ( m_background )				{	m_background->SetState( S_Enabled );		}
			else if ( m_back_frameline )	{	m_back_frameline->SetState( S_Enabled );	}
		}
	}

	u32 textColor;
	u32 hintColor = 0;

	if (!m_bIsEnabled)
	{
		textColor = m_bUseTextColor[D] ? m_dwTextColor[D] : m_dwTextColor[E];
		if(m_hint)
			hintColor = m_hint->m_bUseTextColor[D] ? m_hint->m_dwTextColor[D] : m_hint->m_dwTextColor[E];
	}
	else if (CUIButton::BUTTON_PUSHED == m_eButtonState)
	{
		textColor = m_bUseTextColor[T] ? m_dwTextColor[T] : m_dwTextColor[E];
		if(m_hint)
			hintColor = m_hint->m_bUseTextColor[T] ? m_hint->m_dwTextColor[T] : m_hint->m_dwTextColor[E];
	}
	else if (m_bCursorOverWindow)
	{
		textColor = m_bUseTextColor[H] ? m_dwTextColor[H] : m_dwTextColor[E];
		if(m_hint)
			hintColor = m_hint->m_bUseTextColor[H] ? m_hint->m_dwTextColor[H] : m_hint->m_dwTextColor[E];
	}
	else
	{
		textColor = m_dwTextColor[E];
		if(m_hint)
			hintColor = m_hint->m_dwTextColor[E];
	}

	CUIStatic::SetTextColor		(textColor);
	if(m_hint)
		m_hint->SetTextColor	(hintColor);
}

// =================================================================================================
CUI3tButtonEx::CUI3tButtonEx()
{
	m_hint_owner = NULL;
}

CUI3tButtonEx::~CUI3tButtonEx()
{
}

void CUI3tButtonEx::init_from_xml( CUIXml& xml, LPCSTR path )
{
	CUIXmlInit::Init3tButton( xml, path, 0, this );
	
	XML_NODE* stored_root = xml.GetLocalRoot();
	XML_NODE* new_root    = xml.NavigateToNode( path, 0 );
	xml.SetLocalRoot( new_root );

	if ( xml.NavigateToNode( "hint_text", 0 ) )
	{
		m_hint_owner = xr_new<UIHintWindow>();
		m_hint_owner->SetAutoDelete( true );
		AttachChild( m_hint_owner );
		CUIXmlInit::InitHintWindow( xml, "hint_text", 0, m_hint_owner );
		m_hint_owner->SetWndPos( Fvector2().set( 0.0f, 0.0f ) );
		m_hint_owner->SetWndSize( GetWndSize() );
	}
	xml.SetLocalRoot( stored_root );
}

void CUI3tButtonEx::set_hint_wnd( UIHint* hint_wnd )
{
	VERIFY( m_hint_owner );
	if ( m_hint_owner )
	{
		m_hint_owner->set_hint_wnd( hint_wnd );
	}
}

void CUI3tButtonEx::OnFocusLost()
{
	inherited::OnFocusLost();
	if ( m_hint_owner ) m_hint_owner->OnFocusLost();
	if ( m_eButtonState == BUTTON_PUSHED )
	{
		m_eButtonState = BUTTON_NORMAL;
	}
}

void CUI3tButtonEx::OnFocusReceive()
{
	inherited::OnFocusReceive();
	if ( m_hint_owner ) m_hint_owner->OnFocusReceive();
}

void CUI3tButtonEx::Update()
{
	inherited::Update();
	if ( m_hint_owner ) m_hint_owner->Update();
}

void CUI3tButtonEx::Show( bool status )
{
	inherited::Show( status );
	if ( m_hint_owner ) m_hint_owner->Show( status );
}

bool CUI3tButtonEx::OnMouseDown( int mouse_btn )
{
	if ( m_hint_owner ) m_hint_owner->disable_hint();
	return inherited::OnMouseDown( mouse_btn );
}
