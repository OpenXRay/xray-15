#pragma once
#include "UIWindow.h"
#include "UIWndCallback.h"
#include "../../xrServerEntities/associative_vector.h"
#include "../GameTaskDefs.h"

class CUIMapWnd;
class CUIStatic;
class CGameTask;
class CUIXml;
class CUITaskItem;
class CUI3tButtonEx;
class CUIFrameLineWnd;
class UISecondTaskWnd;
class UIMapLegend;
class UIHint;

class CUITaskWnd			:	public CUIWindow, 
								public CUIWndCallback
{
private:
	typedef CUIWindow		inherited;

	CUIFrameLineWnd*		m_background;
	CUIStatic*				m_center_background;
	CUIStatic*				m_right_bottom_background;
	CUIFrameLineWnd*		m_task_split;

	CUIMapWnd*				m_pMapWnd;
	CUITaskItem*			m_pStoryLineTaskItem;
	CUITaskItem*			m_pSecondaryTaskItem;

	CUI3tButtonEx*			m_BtnSecondaryTaskWnd;
	CUIStatic*				m_second_task_index;
	u32						m_actual_frame;

	CUI3tButtonEx*			m_btn_focus;
	CUI3tButtonEx*			m_btn_focus2;

	UISecondTaskWnd*		m_second_task_wnd;
	bool					m_second_task_wnd_show;
	UIMapLegend*			m_map_legend_wnd;

public:
	UIHint*					hint_wnd;

public:
								CUITaskWnd				();
	virtual						~CUITaskWnd				();
	virtual void				SendMessage				(CUIWindow* pWnd, s16 msg, void* pData);
			void				Init					();
	virtual void				Update					();
	virtual void				Draw					();
			void				DrawHint				();
	virtual void				Show					(bool status);
	virtual void				Reset					();

			void				ReloadTaskInfo			();
			void				ShowMapLegend			(bool status);
			void				Switch_ShowMapLegend	();

			void				Show_SecondTasksWnd		(bool status);

private:
	void						TaskSetTargetMap		(CGameTask* task);
	void						TaskShowMapSpot			(CGameTask* task, bool show);

	void						OnNextTaskClicked		();
	void 						OnPrevTaskClicked		();
	void xr_stdcall				OnShowSecondTaskWnd		(CUIWindow* w, void* d);

	void xr_stdcall				OnTask1DbClicked		(CUIWindow*, void*);
	void xr_stdcall				OnTask2DbClicked		(CUIWindow*, void*);

};

class CUITaskItem : public CUIWindow
{
private:
	typedef CUIWindow			inherited;

	associative_vector<shared_str, CUIStatic*>			m_info;
	CGameTask*											m_owner;
public:
								CUITaskItem				();
	virtual						~CUITaskItem			();

	virtual void 				OnFocusReceive			();
	virtual void	 			OnFocusLost				();
	virtual void				Update					();
	virtual void				OnMouseScroll			(float iDirection);
	virtual bool				OnMouse					(float x, float y, EUIMessages mouse_action);
	virtual void				SendMessage				(CUIWindow* pWnd, s16 msg, void* pData);

	void						Init					(CUIXml& uiXml, LPCSTR path);
	void						InitTask				(CGameTask* task);
	CGameTask*					OwnerTask				()							{return m_owner;}

public:
	bool						show_hint_can;
	bool						show_hint;

protected:
	u32							m_hint_wt;
};