#include "stdafx.h"
#include "EliteDetector.h"
#include "player_hud.h"
#include "../Include/xrRender/UIRender.h"
#include "ui/UIXmlInit.h"
#include "ui/xrUIXmlParser.h"
#include "ui/UIStatic.h"

CEliteDetector::CEliteDetector()
{
	m_artefacts.m_af_rank = 3;
}

CEliteDetector::~CEliteDetector()
{}


void CEliteDetector::CreateUI()
{
	R_ASSERT(NULL==m_ui);
	m_ui				= xr_new<CUIArtefactDetectorElite>();
	ui().construct		(this);
}

CUIArtefactDetectorElite&  CEliteDetector::ui()
{
	return *((CUIArtefactDetectorElite*)m_ui);
}

void CEliteDetector::UpdateAf()
{
	ui().Clear							();
	if(m_artefacts.m_ItemInfos.size()==0)	return;

	CAfList::ItemsMapIt it_b	= m_artefacts.m_ItemInfos.begin();
	CAfList::ItemsMapIt it_e	= m_artefacts.m_ItemInfos.end();
	CAfList::ItemsMapIt it		= it_b;

	Fvector						detector_pos = Position();
	for(;it_b!=it_e;++it_b)//only nearest
	{
		CArtefact *pAf			= it_b->first;
		if(pAf->H_Parent())		
			continue;

		ui().RegisterAf			(pAf->Position());

		if(pAf->CanBeInvisible())
		{
			float d = detector_pos.distance_to(pAf->Position());
			if(d<m_fAfVisRadius)
				pAf->SwitchVisibility(true);
		}
	}
}

bool  CEliteDetector::render_item_3d_ui_query()
{
	return IsWorking();
}

void CEliteDetector::render_item_3d_ui()
{
	R_ASSERT(HudItemData());
	inherited::render_item_3d_ui();
	ui().Draw			();
	//	Restore cull mode
	UIRender->CacheSetCullMode	(IUIRender::cmCCW);
}

void fix_ws_wnd_size(CUIWindow* w, float kx)
{
	Fvector2 p		= w->GetWndSize();
	p.x				/= kx;
	w->SetWndSize	(p);

	p				= w->GetWndPos();
	p.x				/= kx;
	w->SetWndPos	(p);

	CUIWindow::WINDOW_LIST::iterator it = w->GetChildWndList().begin();
	CUIWindow::WINDOW_LIST::iterator it_e = w->GetChildWndList().end();

	for(;it!=it_e;++it)
	{
		CUIWindow* w2		= *it;
		fix_ws_wnd_size		(w2, kx);
	}
}

void CUIArtefactDetectorElite::construct(CEliteDetector* p)
{
	m_parent							= p;
	CUIXml								uiXml;
	uiXml.Load							(CONFIG_PATH, UI_PATH, "ui_detector_artefact.xml");

	CUIXmlInit							xml_init;

	xml_init.InitWindow					(uiXml, "elite", 0, this);

	m_wrk_area							= xr_new<CUIWindow>();
	xml_init.InitWindow					(uiXml, "elite:wrk_area", 0, m_wrk_area);
	m_wrk_area->SetAutoDelete			(true);
	AttachChild							(m_wrk_area);

	m_af_sign							= xr_new<CUIStatic>();
	xml_init.InitStatic					(uiXml, "elite:af_sign", 0, m_af_sign);
	m_af_sign->SetAutoDelete			(true);
	m_wrk_area->AttachChild				(m_af_sign);
	m_af_sign->SetCustomDraw			(true);
	

	Fvector _map_attach_p				= pSettings->r_fvector3(m_parent->cNameSect(), "ui_p");
	Fvector _map_attach_r				= pSettings->r_fvector3(m_parent->cNameSect(), "ui_r");
	
	_map_attach_r.mul					(PI/180.f);
	m_map_attach_offset.setHPB			(_map_attach_r.x, _map_attach_r.y, _map_attach_r.z);
	m_map_attach_offset.translate_over	(_map_attach_p);

/*
	float curr_ = (float)Device.dwWidth/(float)Device.dwHeight;
	float kx	= curr_/1.3333333f;
	fix_ws_wnd_size						(this, kx);
*/
}

void CUIArtefactDetectorElite::update()
{
	inherited::update();
	CUIWindow::Update();
}

void CUIArtefactDetectorElite::Draw()
{
	Fmatrix						LM;
	GetUILocatorMatrix			(LM);

	IUIRender::ePointType bk	= UI()->m_currentPointType;

	UI()->m_currentPointType	= IUIRender::pttLIT;

	UIRender->CacheSetXformWorld(LM);
	UIRender->CacheSetCullMode	(IUIRender::cmNONE);

	CUIWindow::Draw				();

	Frect r						= m_wrk_area->GetWndRect();
	Fvector2					rp; 
	m_wrk_area->GetAbsolutePos	(rp);

	Fmatrix						M, Mc;
	float h,p;
	Device.vCameraDirection.getHP(h,p);
	Mc.setHPB					(h,0,0);
	Mc.c.set					(Device.vCameraPosition);
	M.invert					(Mc);

	xr_vector<Fvector>::const_iterator it	 = m_af_to_draw.begin();
	xr_vector<Fvector>::const_iterator it_e  = m_af_to_draw.end();
	for(;it!=it_e;++it)
	{
		Fvector					p = (*it);
		Fvector					pt3d;
		M.transform_tiny		(pt3d,p);
//		float kx				= m_wrk_area->GetWndSize().x / m_parent->m_fAfDetectRadius;
		float kz				= m_wrk_area->GetWndSize().y / m_parent->m_fAfDetectRadius;
		pt3d.x					*= kz;
		pt3d.z					*= kz;

		pt3d.x					+= m_wrk_area->GetWndSize().x/2.0f;	
		pt3d.z					-= m_wrk_area->GetWndSize().y;

		Fvector2				pos;
		pos.set					(pt3d.x, -pt3d.z);
		pos.sub					(rp);
		if( r.in(pos) )
		{
			m_af_sign->SetWndPos	(pos);
			m_af_sign->Draw			();
		}
	}

	UI()->m_currentPointType		= bk;
}

void CUIArtefactDetectorElite::GetUILocatorMatrix(Fmatrix& _m)
{
	Fmatrix	trans					= m_parent->HudItemData()->m_item_transform;
	u16 bid							= m_parent->HudItemData()->m_model->LL_BoneID("cover");
	Fmatrix cover_bone				= m_parent->HudItemData()->m_model->LL_GetTransform(bid);
	_m.mul							(trans, cover_bone);
	_m.mulB_43						(m_map_attach_offset);
}


void CUIArtefactDetectorElite::Clear()
{
	m_af_to_draw.clear();
}

void CUIArtefactDetectorElite::RegisterAf(const Fvector& p)
{
	m_af_to_draw.push_back(p);
}
