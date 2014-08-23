#pragma once

#include "ui_base.h"
//.#include "../Include/xrRender/FactoryPtr.h"
//. class IUIShader;
class CUIStatic;

class CUICursor:public pureRender
{
	bool			bVisible;
	Fvector2		vPos;
	Fvector2		vPrevPos;

	CUIStatic*		m_static;
	void			InitInternal	();
public:
					CUICursor		();
	virtual			~CUICursor		();
	virtual void	OnRender		();
	
	Fvector2		GetCursorPositionDelta();

	Fvector2		GetCursorPosition		();
	void			SetUICursorPosition		(Fvector2 pos);
	void			UpdateCursorPosition	();
	void			OnScreenRatioChanged	();

	bool			IsVisible		() {return bVisible;}
	void			Show			() {bVisible = true;}
	void			Hide			() {bVisible = false;}
};
