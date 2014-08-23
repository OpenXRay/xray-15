#pragma once
#include "CustomDetector.h"

class CUIArtefactDetectorElite;

class CEliteDetector :public CCustomDetector
{
	typedef CCustomDetector	inherited;
public:
					CEliteDetector				();
	virtual			~CEliteDetector				();
	virtual void	render_item_3d_ui			();
	virtual bool	render_item_3d_ui_query		();
protected:
	virtual void 	UpdateAf					();
	virtual void 	CreateUI					();
	CUIArtefactDetectorElite& ui				();
};

class CUIArtefactDetectorElite :public CUIArtefactDetectorBase, public CUIWindow
{
	typedef CUIArtefactDetectorBase	inherited;

	CUIWindow*			m_wrk_area;
	CUIStatic*			m_af_sign;
	xr_vector<Fvector>	m_af_to_draw;
	CEliteDetector*		m_parent;
	Fmatrix				m_map_attach_offset;

	void				GetUILocatorMatrix			(Fmatrix& _m);
public:

	virtual void	update		();
	virtual void	Draw		();

	void		construct		(CEliteDetector* p);
	void		Clear			();
	void		RegisterAf		(const Fvector& p);
};
