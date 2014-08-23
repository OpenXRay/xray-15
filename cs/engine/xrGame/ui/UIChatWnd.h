#pragma once
#include "UIDialogWnd.h"
#include "UIEditBox.h"
#include "xrUIXmlParser.h"

class CUIGameLog;
class game_cl_GameState;

class CUIChatWnd: public CUIDialogWnd
{
	typedef CUIDialogWnd inherited;

public:
						CUIChatWnd			(CUIGameLog *pList);
	virtual				~CUIChatWnd			();
	virtual void		Show				();
	virtual void		Hide				();
	virtual void		SetKeyboardCapture	(CUIWindow* pChildWindow, bool capture_status);
	virtual bool		NeedCursor			() {return false;}
	void				Init				(CUIXml& uiXml);
	void				SetEditBoxPrefix	(const shared_str &prefix);
	void				TeamChat			() { sendNextMessageToTeam = true; }
	void				AllChat				() { sendNextMessageToTeam = false; }
	void				PendingMode			(bool const is_pending_mode);
	void				SetOwner			(game_cl_GameState *pO) { pOwner = pO; }
	virtual bool		NeedCursor			()const {return false;}

	CUIEditBox			UIEditBox;

protected:
	CUIGameLog			*pUILogList;
	CUIStatic			UIPrefix;
	bool				sendNextMessageToTeam;
	bool				pendingGameMode;
	
	Fvector2			pending_prefix_pos;
	Fvector2			pending_prefix_wnd_size;
	Fvector2			pending_edit_pos;
	Fvector2			pending_edit_wnd_size;
	
	Fvector2			inprogress_prefix_pos;
	Fvector2			inprogress_prefix_wnd_size;
	Fvector2			inprogress_edit_pos;
	Fvector2			inprogress_edit_wnd_size;



	game_cl_GameState	*pOwner;
};
