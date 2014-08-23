#include "StdAfx.h"

#include "UITrackBar.h"
//.#include "UITrackButton.h"
#include "UIFrameLineWnd.h"
#include "UI3tButton.h"
#include "UITextureMaster.h"
#include "../../xrEngine/xr_input.h"

#define DEF_CONTROL_HEIGHT		21.0f
#define FRAME_LINE_TEXTURE		"ui_slider_e"
#define FRAME_LINE_TEXTURE_D	"ui_slider_d"
#define SLIDER_TEXTURE			"ui_slider_button"

CUITrackBar::CUITrackBar()
	: m_f_min(0),
	  m_f_max(1),
	  m_f_val(0),
	  m_f_back_up(0),
	 m_f_step(0.01f),
	m_b_is_float(true),
	m_b_invert(false)
{	
	m_pFrameLine					= xr_new<CUIFrameLineWnd>();	
	AttachChild						(m_pFrameLine);	
	m_pFrameLine->SetAutoDelete		(true);
	m_pFrameLine_d					= xr_new<CUIFrameLineWnd>(); 
	m_pFrameLine_d->SetVisible		(false);
	AttachChild						(m_pFrameLine_d); 
	m_pFrameLine_d->SetAutoDelete	(true);
	m_pSlider						= xr_new<CUI3tButton>();			
	AttachChild						(m_pSlider);		
	m_pSlider->SetAutoDelete		(true);
//.	m_pSlider->SetOwner				(this);
	m_b_mouse_capturer				= false;
}

bool CUITrackBar::OnMouse(float x, float y, EUIMessages mouse_action)
{
	CUIWindow::OnMouse(x, y, mouse_action);

	switch (mouse_action)
	{
	case WINDOW_MOUSE_MOVE:
		{
			if(m_bCursorOverWindow && m_b_mouse_capturer)
			{
				if (pInput->iGetAsyncBtnState(0))
					UpdatePosRelativeToMouse();
			}
		}break;
	case WINDOW_LBUTTON_DOWN:
		{
			m_b_mouse_capturer = m_bCursorOverWindow;
			if(m_b_mouse_capturer)
				UpdatePosRelativeToMouse();
		}break;

	case WINDOW_LBUTTON_UP:
		{
			m_b_mouse_capturer = false;
		}
	};
	return true;
}

void CUITrackBar::InitTrackBar(Fvector2 pos, Fvector2 size)
{
	string128			buf;
	float				item_height;
	float				item_width;
	CUIWindow::SetWndPos(pos);
	
	CUIWindow::SetWndSize(Fvector2().set(size.x, DEF_CONTROL_HEIGHT) );


	item_height			= CUITextureMaster::GetTextureHeight(strconcat(sizeof(buf),buf,FRAME_LINE_TEXTURE,"_b"));
	m_pFrameLine->SetWndPos( Fvector2().set(0.0f, (size.y - item_height)/2) );
	m_pFrameLine->SetWndSize( Fvector2().set(size.x, item_height) );

	m_pFrameLine->InitTexture(FRAME_LINE_TEXTURE,"hud\\default");

	m_pFrameLine_d->SetWndPos( Fvector2().set(0.0f,(size.y - item_height)/2.0f));
	m_pFrameLine_d->SetWndSize( Fvector2().set(size.x, item_height));
	m_pFrameLine_d->InitTexture(FRAME_LINE_TEXTURE_D,"hud\\default");

	strconcat			(sizeof(buf),buf,SLIDER_TEXTURE,"_e");
	item_width			= CUITextureMaster::GetTextureWidth(buf);
    item_height			= CUITextureMaster::GetTextureHeight(buf);

	if(UI()->is_16_9_mode())
		item_width	/= 1.2f;

	m_pSlider->InitButton(	Fvector2().set(0.0f, (size.y - item_height)/2.0f), //pos
							Fvector2().set(item_width, item_height) );			//size
	m_pSlider->InitTexture(SLIDER_TEXTURE);
}	

void CUITrackBar::SetCurrentValue()
{
	if(m_b_is_float)
		GetOptFloatValue	(m_f_val, m_f_min, m_f_max);
	else
		GetOptIntegerValue		(m_i_val, m_i_min, m_i_max);

	UpdatePos			();
}

void CUITrackBar::Draw()
{
	CUIWindow::Draw();
}

void CUITrackBar::Update()
{
	CUIWindow::Update();

	if(m_b_mouse_capturer)
	{
		if(!pInput->iGetAsyncBtnState(0))
			m_b_mouse_capturer = false;
	}
}

void CUITrackBar::SaveValue()
{
	CUIOptionsItem::SaveValue	();
	if(m_b_is_float)
		SaveOptFloatValue			(m_f_val);
	else
		SaveOptIntegerValue			(m_i_val);
}

bool CUITrackBar::IsChanged()
{
	if(m_b_is_float)
	{
		return !fsimilar(m_f_back_up, m_f_val); 
	}else
	{
		return (m_i_back_up != m_i_val);
	}
}

void CUITrackBar::SetStep(float step)
{
	if(m_b_is_float)
		m_f_step	= step;
	else
		m_i_step	= iFloor(step);
}

void CUITrackBar::SeveBackUpValue()
{
	if(m_b_is_float)
		m_f_back_up		= m_f_val;
	else
		m_i_back_up		= m_i_val;
}

void CUITrackBar::Undo()
{
	if(m_b_is_float)
		m_f_val			= m_f_back_up;
	else
		m_i_val			= m_i_back_up;

	SaveValue			();
	SetCurrentValue		();
}

void CUITrackBar::Enable(bool status)
{
	m_bIsEnabled				= status;
	m_pFrameLine->SetVisible	(status);
	m_pSlider->Enable			(status);
	m_pFrameLine_d->SetVisible	(!status);
}

void CUITrackBar::UpdatePosRelativeToMouse()
{
	float _bkf		= 0.0f;
	int _bki		= 0;
	if(m_b_is_float)
	{
		_bkf = m_f_val; 
	}else
	{
		_bki = m_i_val; 
	}


	float btn_width				= m_pSlider->GetWidth();
	float window_width			= GetWidth();		
	float fpos					= cursor_pos.x;

	if( GetInvert() )
		fpos					= window_width - fpos;

	if (fpos < btn_width/2)
		fpos = btn_width/2;
	else 
	if (fpos > window_width - btn_width/2)
		fpos = window_width - btn_width/2;

	float __fval;
	float __fmax	= (m_b_is_float)?m_f_max:(float)m_i_max;
	float __fmin	= (m_b_is_float)?m_f_min:(float)m_i_min;
	float __fstep	= (m_b_is_float)?m_f_step:(float)m_i_step;

	__fval						= (__fmax - __fmin)*(fpos - btn_width/2)/(window_width - btn_width)+ __fmin;
	
	float _d	= (__fval-__fmin);
	
	float _v	= _d/__fstep;
	int _vi		= iFloor(_v);
	float _vf	= __fstep*_vi;
	
	if(_d-_vf>__fstep/2.0f)	
		_vf		+= __fstep;

	__fval		= __fmin+_vf;
	
	clamp		(__fval, __fmin, __fmax);

	if(m_b_is_float)
		m_f_val	= __fval;
	else
		m_i_val	= iFloor(__fval);
	

	bool b_ch = false;
	if(m_b_is_float)
	{
		b_ch  = !fsimilar(_bkf, m_f_val); 
	}else
	{
		b_ch  =  (_bki != m_i_val);
	}

	if(b_ch)
		GetMessageTarget()->SendMessage(this, BUTTON_CLICKED, NULL);

	UpdatePos	();
}

void CUITrackBar::UpdatePos()
{
#ifdef DEBUG
	
	if(m_b_is_float)
		R_ASSERT2(m_f_val >= m_f_min && m_f_val <= m_f_max, "CUITrackBar::UpdatePos() - m_val >= m_min && m_val <= m_max" );
	else
		R_ASSERT2(m_i_val >= m_i_min && m_i_val <= m_i_max, "CUITrackBar::UpdatePos() - m_val >= m_min && m_val <= m_max" );

#endif

	float btn_width				= m_pSlider->GetWidth();
	float window_width			= GetWidth();		
	float free_space			= window_width - btn_width;
	Fvector2 pos				= m_pSlider->GetWndPos();
    
	float __fval	= (m_b_is_float)?m_f_val:(float)m_i_val;
	float __fmax	= (m_b_is_float)?m_f_max:(float)m_i_max;
	float __fmin	= (m_b_is_float)?m_f_min:(float)m_i_min;


	pos.x						= (__fval - __fmin)*free_space/(__fmax - __fmin);
	if( GetInvert() )
		pos.x					= free_space-pos.x;

	m_pSlider->SetWndPos		(pos);
	SaveValue					();
}

void CUITrackBar::OnMessage(LPCSTR message)
{
	if (0 == xr_strcmp(message,"set_default_value"))
	{
		if(m_b_is_float)
			m_f_val = m_f_min + (m_f_max - m_f_min)/2.0f;
		else
			m_i_val = m_i_min + iFloor((m_i_max - m_i_min)/2.0f);

		UpdatePos();
	}
}

bool CUITrackBar::GetCheck()
{
	VERIFY(!m_b_is_float);
	return !!m_i_val;
}

void CUITrackBar::SetCheck(bool b)
{
	VERIFY(!m_b_is_float);
	m_i_val = (b)?m_i_max:m_i_min;
}
