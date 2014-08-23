#pragma once
#include "customdetector.h"

class CUIArtefactDetectorSimple;

class CSimpleDetector :public CCustomDetector
{
	typedef CCustomDetector	inherited;
public:
					CSimpleDetector				();
	virtual			~CSimpleDetector			();
	
protected:
//.	virtual void 	UpdateZones					();
	virtual void 	UpdateAf					();
	virtual void 	CreateUI					();
	CUIArtefactDetectorSimple&	ui				();
};

class CUIArtefactDetectorSimple :public CUIArtefactDetectorBase
{
	typedef CUIArtefactDetectorBase	inherited;

	CSimpleDetector*	m_parent;
	u16					m_flash_bone;
	u16					m_on_off_bone;
	u32					m_turn_off_flash_time;
	
	ref_light			m_flash_light;
	ref_light			m_on_off_light;
	CLAItem*			m_pOnOfLAnim;
	CLAItem*			m_pFlashLAnim;
	void				setup_internals			();
public:
	virtual				~CUIArtefactDetectorSimple	();
	void				update						();
	void				Flash						(bool bOn, float fRelPower);

	void				construct					(CSimpleDetector* p);
};
