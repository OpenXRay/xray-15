#pragma once
#include "UIFrameLineWnd.h"

class CUIStatic;
class CUIFrameLineWnd;
class CUIDetectorWave;
class CSimpleDetector;
class CAdvancedDetector;
class CEliteDetector;

class CUIArtefactDetectorBase
{
public:
	virtual			~CUIArtefactDetectorBase	()	{};
	virtual void	update						()	{};
};

class CUIDetectorWave :public CUIFrameLineWnd
{
	typedef CUIFrameLineWnd inherited;
protected:
	float			m_curr_v;
	float			m_step;
public:
					CUIDetectorWave		():m_curr_v(0.0f),m_step(0.0f){};
			void	InitFromXML			(CUIXml& xml, LPCSTR path);
			void	SetVelocity			(float v);
	virtual void	Update				();
};