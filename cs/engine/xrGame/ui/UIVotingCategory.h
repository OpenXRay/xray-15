#pragma once

#include "UIDialogWnd.h"

class CUIStatic;
class CUI3tButtonEx;
class CUIKickPlayer;
class CUIChangeMap;
class CUIChangeWeather;
class CUIChangeGameType;
class CUIXml;
class CUITextVote;

class CUIVotingCategory : public CUIDialogWnd 
{
private:
	typedef CUIDialogWnd inherited;
public:
						CUIVotingCategory	();
	virtual				~CUIVotingCategory	();

	virtual bool		OnKeyboard			(int dik, EUIMessages keyboard_action);
	virtual void		SendMessage			(CUIWindow* pWnd, s16 msg, void* pData = 0);

	void				OnBtn				(int i);
	void				OnBtnCancel			();

	virtual void		Update				();

protected:
	void				InitVotingCategory	();

	CUIStatic*			header;
	CUI3tButtonEx*		btn[7];
	CUIStatic*			txt[7];
	CUIStatic*			bkgrnd;
	CUI3tButtonEx*		btn_cancel;

	CUIKickPlayer*		kick;
	CUIChangeMap*		change_map;
	CUIChangeWeather*	change_weather;
	CUIChangeGameType*	change_gametype;
	CUIXml*				xml_doc;
};