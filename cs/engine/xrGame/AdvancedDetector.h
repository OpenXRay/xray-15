#pragma once
#include "CustomDetector.h"
class CUIArtefactDetectorAdv;

class CAdvancedDetector :public CCustomDetector
{
	typedef CCustomDetector	inherited;
public:
					CAdvancedDetector			();
	virtual			~CAdvancedDetector			();
	virtual void	on_a_hud_attach				();
	virtual void	on_b_hud_detach				();
protected:
	virtual void 	UpdateAf					();
	virtual void 	CreateUI					();
	CUIArtefactDetectorAdv& ui					();

};

class CUIArtefactDetectorAdv :public CUIArtefactDetectorBase
{
	typedef CUIArtefactDetectorBase	inherited;

	CAdvancedDetector*		m_parent;
	Fvector					m_target_dir;
	float					m_cur_y_rot;
	float					m_curr_ang_speed;
	u16						m_bid;

public:
	virtual					~CUIArtefactDetectorAdv			();
	virtual void			update							();
	void					construct						(CAdvancedDetector* p);
	void					SetValue						(const float v1, const Fvector& v2);
	float					CurrentYRotation				()	const;
	static void 			BoneCallback					(CBoneInstance *B);
	void					ResetBoneCallbacks				();
	void					SetBoneCallbacks				();
};
