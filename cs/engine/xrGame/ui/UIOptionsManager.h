#pragma once

class CUIOptionsItem;

class CUIOptionsManager{
	friend class CUIOptionsItem;
public:	
		CUIOptionsManager					();

	void SeveBackupValues					(const shared_str& group);
	void SetCurrentValues					(const shared_str& group);
	void SaveValues							(const shared_str& group);
	bool IsGroupChanged						(const shared_str& group);
	void UndoGroup							(const shared_str& group);

	void OptionsPostAccept					();
	void DoVidRestart						();
	void DoSndRestart						();
	void DoSystemRestart					();

	bool NeedSystemRestart					()	{return m_b_system_restart;}
	void SendMessage2Group					(const shared_str& group, const char* message);

protected:	
	void RegisterItem						(CUIOptionsItem* item, const shared_str& group);
	void UnRegisterGroup					(const shared_str& group);
	void UnRegisterItem						(CUIOptionsItem* item);


	typedef	shared_str									group_name;
	typedef xr_vector<CUIOptionsItem*>					items_list;
    typedef xr_map<group_name, items_list>				groups;
	typedef xr_map<group_name, items_list>::iterator	groups_it;

	groups	m_groups;

	bool	m_b_vid_restart;
	bool	m_b_snd_restart;
	bool	m_b_system_restart;
};