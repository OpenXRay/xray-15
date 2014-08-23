#pragma once


#include "ui/UIStatic.h"

class CActor;
class CUICustomMap;
//////////////////////////////////////////////////////////////////////////


class CUIZoneMap
{
public:
	bool						visible;

private:
	CUICustomMap*				m_activeMap;
//	float						m_fScale;

	CUIStatic					m_background;
	CUIStatic					m_center;
	CUIStatic					m_compass;
	CUIStatic					m_clipFrame;
	CUIStatic					m_pointerDistanceText;
	CUIStatic					m_Counter;
	CUIStatic					m_Counter_text;
	u8							m_current_map_idx;

public:
								CUIZoneMap		();
	virtual						~CUIZoneMap		();

	void						Init			();

	void						Render			();
	void						Update			();

//	void						SetScale		(float s)							{m_fScale = s;}
//	float						GetScale		()									{return m_fScale;}

	bool						ZoomIn			();
	bool						ZoomOut			();

	CUIStatic&					Background		()									{return m_background;};
	void						SetupCurrentMap	();
	void						OnSectorChanged	(int sector);
	void						Counter_ResetClrAnimation();

private:
	void						SetHeading		(float angle);
	void						UpdateRadar		(Fvector pos);

};
