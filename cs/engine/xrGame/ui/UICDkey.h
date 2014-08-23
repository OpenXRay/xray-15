//
#pragma once
#include "UIEditBox.h"

class CUICDkey : public CUIEditBox
{
private:
	typedef			CUIEditBox		inherited;

public:
					CUICDkey		();
	virtual	void	SetText			(LPCSTR str) {}
	virtual	LPCSTR	GetText			();
	// CUIOptionsItem
	virtual void	SetCurrentValue	();
	virtual void	SaveValue		();
	virtual bool	IsChanged		();
	
			void	CreateCDKeyEntry();			

	virtual void	Show			(bool status);
	virtual void	Draw			();
	virtual void	OnFocusLost		();

private:
	bool			m_view_access;

}; // class CUICDkey

class CUIMPPlayerName : public CUIEditBox
{
private:
	typedef			CUIEditBox		inherited;

public:
					CUIMPPlayerName	() {};
	virtual			~CUIMPPlayerName() {};

//	virtual	void	SetText			(LPCSTR str) {}

//	virtual void	SetCurrentValue();
//	virtual void	SaveValue();
//	virtual bool	IsChanged();

	virtual void	OnFocusLost		();

}; // class CUIMPPlayerName

extern	void	GetCDKey_FromRegistry		(char* cdkey);
extern	void	WriteCDKey_ToRegistry		(LPSTR cdkey);
extern	void	GetPlayerName_FromRegistry	(char* name, u32 const name_size);
extern	void	WritePlayerName_ToRegistry	(LPSTR name);
