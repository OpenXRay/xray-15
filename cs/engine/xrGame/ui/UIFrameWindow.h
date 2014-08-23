#pragma once

#include "uiwindow.h"

#include "../uiframerect.h"

class CUIStatic;


class CUIFrameWindow: public CUIWindow,
					  public CUIMultiTextureOwner
{
	typedef CUIWindow inherited;
public:
					CUIFrameWindow				();

			void	InitFrameWindow				(Fvector2 pos, Fvector2 size);
			void	UpdateSize					();

	virtual void	InitTexture					(LPCSTR texture);
	virtual void	InitTextureEx				(LPCSTR texture, LPCSTR  shader);
			void	SetTextureColor				(u32 color)										{m_UIWndFrame.SetTextureColor(color);}

	virtual void	SetWndSize					(const Fvector2& size);
	virtual void	SetWidth					(float width);
	virtual void	SetHeight					(float height);
	
			void	SetColor					(u32 cl);

	virtual void	Draw						();
	virtual void	Update						();
	
	void			SetVisiblePart				(CUIFrameRect::EFramePart p, BOOL b)	{m_UIWndFrame.SetVisiblePart(p,b);};
protected:

	CUIFrameRect	m_UIWndFrame;

	void			FrameClip					(const Frect parentAbsR);
	
private:
	inline void		ClampMax_Zero				(Frect &r);

};
