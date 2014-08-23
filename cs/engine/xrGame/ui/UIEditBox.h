#pragma once
#include "../../xrServerEntities/script_export_space.h"
#include "UIOptionsItem.h"
#include "UIColorAnimatorWrapper.h"
#include "UICustomEdit.h"
#include "UIFrameLineWnd.h"

class CUIEditBox : /*public CUIMultiTextureOwner,*/ public CUIOptionsItem, public CUICustomEdit
{
public:
					CUIEditBox		();
	virtual			~CUIEditBox		();

	virtual void	InitCustomEdit	(Fvector2 pos, Fvector2 size);

	// CUIOptionsItem
	virtual void	SetCurrentValue();
	virtual void	SaveValue();
	virtual bool	IsChanged();

	// CUIMultiTextureOwner
	virtual void	InitTexture					(LPCSTR texture);
	virtual void	InitTextureEx				(LPCSTR texture, LPCSTR  shader);
protected:
	CUIFrameLineWnd	m_frameLine;
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CUIEditBox)
#undef script_type_list
#define script_type_list save_type_list(CUIEditBox)
