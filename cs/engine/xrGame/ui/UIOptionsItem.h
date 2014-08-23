#pragma once
#include "UIOptionsManager.h"

class CUIOptionsItem
{
	friend class CUIOptionsManager;
public:
	enum ESystemDepends		{sdNothing, sdVidRestart, sdSndRestart, sdSystemRestart};

public:
							CUIOptionsItem		();
	virtual					~CUIOptionsItem		();
	virtual void			AssignProps			(const shared_str& entry, const shared_str& group);
	void					SetSystemDepends	(ESystemDepends val) {m_dep = val;}

	static CUIOptionsManager* GetOptionsManager	() {return &m_optionsManager;}

protected:
	virtual void			SetCurrentValue		()	=0;	
	virtual void			SaveValue			();

	virtual bool			IsChanged			()			=0;
	virtual void			SeveBackUpValue		()	{};
	virtual void			Undo				()				{SetCurrentValue();};
			
			void			SendMessage2Group	(LPCSTR group, LPCSTR message);
	virtual	void			OnMessage			(LPCSTR message);


			// string
			LPCSTR			GetOptStringValue	();
			void			SaveOptStringValue	(LPCSTR val);
			// integer
			void			GetOptIntegerValue	(int& val, int& min, int& max);
			void			SaveOptIntegerValue	(int val);
			// float
			void			GetOptFloatValue	(float& val, float& min, float& max);
			void			SaveOptFloatValue	(float val);
			// bool
			bool			GetOptBoolValue		();
			void			SaveOptBoolValue	(bool val);
			// token
			LPCSTR			GetOptTokenValue	();
			xr_token*		GetOptToken			();
			void			SaveOptTokenValue	(LPCSTR val);

	shared_str				m_entry;
	ESystemDepends			m_dep;

	static CUIOptionsManager m_optionsManager;
};
