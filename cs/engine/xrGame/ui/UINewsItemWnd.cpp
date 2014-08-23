#include "stdafx.h"
#include "UINewsItemWnd.h"
#include "UIXmlInit.h"
#include "UIStatic.h"
#include "../game_news.h"
#include "../date_time.h"
#include "UIInventoryUtilities.h"
#include "UIHelper.h"

CUINewsItemWnd::CUINewsItemWnd()
{}

CUINewsItemWnd::~CUINewsItemWnd()
{}

void CUINewsItemWnd::Init(CUIXml& uiXml, LPCSTR start_from)
{
	CUIXmlInit::InitWindow		(uiXml,start_from,0,this);

	XML_NODE* stored_root		= uiXml.GetLocalRoot();
	XML_NODE* node				= uiXml.NavigateToNode( start_from, 0 );
	uiXml.SetLocalRoot			( node );

	m_UIImage		= UIHelper::CreateStatic( uiXml, "image", this );
	m_UICaption		= UIHelper::CreateStatic( uiXml, "caption_static", this );
	m_UIText		= UIHelper::CreateStatic( uiXml, "text_static", this );
	m_UIDate		= UIHelper::CreateStatic( uiXml, "date_static", this );

	CUIXmlInit::InitAutoStaticGroup(uiXml, "auto", 0, this);

	uiXml.SetLocalRoot( stored_root );
}

void CUINewsItemWnd::Setup			(GAME_NEWS_DATA& news_data)
{
//	string128						time_str;
//	u32								years, months, days, hours, minutes, seconds, milliseconds;
//	split_time						(news_data.receive_time, years, months, days, hours, minutes, seconds, milliseconds);
//	sprintf_s						(time_str, "%%c[255,225,225,250]%02i/%02i/%04i %02i:%02i  ", days, months, years, hours, minutes );

	shared_str time_str				= InventoryUtilities::GetTimeAndDateAsString( news_data.receive_time );
	u32    sz  = (time_str.size() + 5) * sizeof(char);
	PSTR   str = (PSTR)_alloca( sz );
	strcpy_s( str, sz, time_str.c_str() );
	strcat_s( str, sz, " -" );
	m_UIDate->SetText(str);
	m_UIDate->AdjustWidthToText();

	m_UICaption->SetTextST	(news_data.news_caption.c_str());
	Fvector2 pos			= m_UICaption->GetWndPos();
	pos.x					= m_UIDate->GetWndPos().x + m_UIDate->GetWndSize().x + 5.0f;
	m_UICaption->SetWndPos	(pos);
	m_UICaption->SetWidth	( _min( m_UIText->GetWidth() - m_UIDate->GetWidth() - 5.0f, m_UICaption->GetWidth() ) );
	
	m_UIText->SetTextST				(news_data.news_text.c_str());
	m_UIText->AdjustHeightToText	();
	float h1						= m_UIText->GetWndPos().y + m_UIText->GetHeight() + 6.0f;

	m_UIImage->InitTexture			(news_data.texture_name.c_str());
//	m_UIImage->SetOriginalRect		(news_data.tex_rect.x1,news_data.tex_rect.y1,news_data.tex_rect.x2,news_data.tex_rect.y2);
	float h3						= m_UIImage->GetWndPos().y + m_UIImage->GetHeight();
//	h1								= _max(h1,h2);
	h1								= _max(h1,h3);
	SetHeight						(h1);
}
