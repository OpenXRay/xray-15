#pragma once

#include "UIDialogWnd.h"

class CUIStatic;
class CUI3tButtonEx;
class CUIListBox;
class CUIFrameWindow;



class CUIVote : public CUIDialogWnd 
{
public:

					CUIVote		();
			void 	Init		();
	virtual void 	Update		();
	virtual void 	SendMessage	(CUIWindow* pWnd, s16 msg, void* pData = 0);
			void 	OnBtnYes	();
			void 	OnBtnNo		();
			void 	OnBtnCancel	();
			void 	SetVoting	(LPCSTR txt);
protected:
	CUIStatic*		bkgrnd;
	CUIStatic*		msg_back;
	CUIStatic*		msg;
	CUIStatic*		cap[3];
	CUIFrameWindow* frame[3];
	CUIListBox*		list[3];

	CUI3tButtonEx*	btn_yes;
	CUI3tButtonEx*	btn_no;
	CUI3tButtonEx*	btn_cancel;

	u32				m_prev_upd_time;
};