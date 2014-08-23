#pragma once

#include "UIFrameLine.h"
#include "UIWindow.h"
#include "UIStatic.h"

class CUIFrameLineWnd: public CUIWindow
{
	typedef CUIWindow inherited;
public:
				 CUIFrameLineWnd	();
			void InitFrameLineWnd	(LPCSTR base_name, Fvector2 pos, Fvector2 size, bool horizontal = true);
			void InitFrameLineWnd	(Fvector2 pos, Fvector2 size, bool horizontal = true);
			void InitTexture		(LPCSTR tex_name,LPCSTR sh_name="hud\\default", bool horizontal = true);
	virtual void Draw				();
	virtual void SetWidth			(float width);
	virtual void SetHeight			(float height);

	virtual void SetWndPos			(const Fvector2& pos);
	virtual void SetWndSize			(const Fvector2& size);

	virtual void SetOrientation		(bool horizontal);
			float GetTextureHeight	();
			void SetColor			(u32 cl);
			
			void SetStretchTexture	( bool bStretch )	{ m_bStretchTexture = bStretch; }
			bool GetStretchTexture	() const			{ return m_bStretchTexture; }
protected:
	bool			bHorizontal;
	bool			m_bTextureAvailable;
	CUIFrameLine	UIFrameLine;	
	bool			m_bStretchTexture;
};
