#pragma once

#include "UIStatic.h"
#include "UIOptionsItem.h"

struct _action;
struct _keyboard;
class CUIColorAnimatorWrapper;

class CUIEditKeyBind : public CUIStatic, public CUIOptionsItem 
{
	bool			m_bPrimary;
	_action*		m_action;
	_keyboard*		m_keyboard;
public:
					CUIEditKeyBind			(bool bPrim);
	virtual			~CUIEditKeyBind			();
	// options item
	virtual void	AssignProps				(const shared_str& entry, const shared_str& group);
	virtual void	SetCurrentValue			();
	virtual void	SaveValue				();
	virtual	void	OnMessage				(LPCSTR message);
	virtual bool	IsChanged				();

	// CUIWindow methods
			void	InitKeyBind				(Fvector2 pos, Fvector2 size);
	virtual void	Update					();
	virtual bool	OnMouseDown				(int mouse_btn);
	virtual void	OnFocusLost				();
	virtual bool	OnKeyboard				(int dik, EUIMessages keyboard_action);
	// IUITextControl
	virtual void	SetText					(LPCSTR text);
			void	SetEditMode				(bool b);
protected:
	void			BindAction2Key			();

	bool		m_bIsEditMode;
	bool		m_bChanged;

	CUIColorAnimatorWrapper*				m_pAnimation;
};