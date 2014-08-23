// File:        UIInteractiveBackground.h
// Description: template class designed for UI controls to represent their state;
//              there are 4 states: Enabled, Disabled, Hightlighted and Touched.
//              As a rule you can use one of 3 background types:
//              Normal Texture, String Texture, Frame Texture (CUIStatic, CUIFrameLineWnd, CUIFrameWindow)
// Created:     29.12.2004
// Author:      Serhiy 0. Vynnychenko
// Mial:        narrator@gsc-game.kiev.ua
//
// Copyright 2004 GSC Game World
//

#pragma once


#include "UIFrameWindow.h"
#include "UIFrameLineWnd.h"

enum UIState{
	S_Enabled,
	S_Disabled,
	S_Highlighted,
	S_Touched
};

template <class T>
class CUIInteractiveBackground : public CUIWindow
{
public:
	CUIInteractiveBackground();
	virtual ~CUIInteractiveBackground();

			void InitIB	(Fvector2 pos, Fvector2 size);
			void InitIB	(LPCSTR texture_e, Fvector2 pos, Fvector2 size);
			T*	 CreateE();
			T*	 CreateD();
			T*	 CreateT();
			T*	 CreateH();
			T*	 GetE	();
			T*	 GetD	();
			T*	 GetT	();
			T*	 GetH	();
	virtual void InitEnabledState(LPCSTR texture_e);
	virtual void InitDisabledState(LPCSTR texture_d);
	virtual void InitHighlightedState(LPCSTR texture_h);
	virtual void InitTouchedState(LPCSTR texture_t);
	virtual void SetState(UIState state);
	virtual void Draw();

	virtual void SetWidth(float width);
	virtual void SetHeight(float heigth);

protected:
	T* m_stateCurrent;
	T* m_stateEnabled;
	T* m_stateDisabled;
	T* m_stateHighlighted;
	T* m_stateTouched;
};

template <class T>
CUIInteractiveBackground<T>::CUIInteractiveBackground(){
	m_stateCurrent     = NULL;
	m_stateEnabled     = NULL;
	m_stateDisabled    = NULL;
	m_stateHighlighted = NULL;
	m_stateTouched     = NULL;	
}

template <class T>
CUIInteractiveBackground<T>::~CUIInteractiveBackground(){

}

template <class T>
void CUIInteractiveBackground<T>::InitIB(Fvector2 pos, Fvector2 size)
{
	CUIWindow::SetWndPos	(pos);
	CUIWindow::SetWndSize	(size);
}

template <class T>
void CUIInteractiveBackground<T>::InitIB(LPCSTR texture_e, Fvector2 pos, Fvector2 size){
	CUIWindow::SetWndPos	(pos);
	CUIWindow::SetWndSize	(size);

	InitEnabledState(texture_e);

	m_stateEnabled->Init(texture_e, Fvector2().set(0,0), size);
	m_stateCurrent = this->m_stateEnabled;
}

template <class T>
T*	 CUIInteractiveBackground<T>::CreateE(){
	Fvector2 size = GetWndSize();
	if (!m_stateEnabled)
	{
		m_stateEnabled = xr_new<T>();
		m_stateEnabled->SetAutoDelete(true);
		AttachChild					(m_stateEnabled);
		m_stateEnabled->SetWndPos	(Fvector2().set(0,0));
		m_stateEnabled->SetWndSize	(size);
	}
	SetState(S_Enabled);
	return m_stateEnabled;
}

template <class T>
T*	 CUIInteractiveBackground<T>::CreateD(){
	Fvector2 size = GetWndSize();
	if (!m_stateDisabled)
	{
		m_stateDisabled = xr_new<T>();
		m_stateDisabled->SetAutoDelete(true);
		AttachChild(m_stateDisabled);
		m_stateDisabled->SetWndPos	(Fvector2().set(0,0));
		m_stateDisabled->SetWndSize	(size);
	}
	return m_stateDisabled;
}

template <class T>
T*	 CUIInteractiveBackground<T>::CreateT(){
	Fvector2 size = GetWndSize();
	if (!m_stateTouched)
	{
		m_stateTouched = xr_new<T>();
		m_stateTouched->SetAutoDelete(true);
		AttachChild(m_stateTouched);		
		m_stateTouched->SetWndPos	(Fvector2().set(0,0));
		m_stateTouched->SetWndSize	(size);
	}
	return m_stateTouched;
}

template <class T>
T*	 CUIInteractiveBackground<T>::CreateH(){
	Fvector2 size = GetWndSize();
	if (!m_stateHighlighted)
	{
		m_stateHighlighted = xr_new<T>();
		m_stateHighlighted->SetAutoDelete(true);
		AttachChild(m_stateHighlighted);
		m_stateHighlighted->SetWndPos	(Fvector2().set(0,0));
		m_stateHighlighted->SetWndSize	(size);
	}    
	return m_stateHighlighted;
}

template <class T>
T*	 CUIInteractiveBackground<T>::GetE(){
    return m_stateEnabled;
}

template <class T>
T*	 CUIInteractiveBackground<T>::GetD(){
	return m_stateDisabled;
}

template <class T>
T*	 CUIInteractiveBackground<T>::GetT(){
	return m_stateTouched;
}

template <class T>
T*	 CUIInteractiveBackground<T>::GetH(){
	return m_stateHighlighted;
}

template <class T>
void CUIInteractiveBackground<T>::InitEnabledState(LPCSTR texture_e){
	Fvector2 size = GetWndSize();

	if (!m_stateEnabled)
	{
		m_stateEnabled = xr_new<T>();
		m_stateEnabled->SetAutoDelete(true);
		AttachChild(m_stateEnabled);
	}

	this->m_stateEnabled->InitTexture(texture_e);
	this->m_stateEnabled->SetWndPos(Fvector2().set(0,0));
	this->m_stateEnabled->SetWndSize(size);

	SetState(S_Enabled);
}

template <class T>
void CUIInteractiveBackground<T>::InitDisabledState(LPCSTR texture_d){
	Fvector2 size = GetWndSize();

	if (!m_stateDisabled)
	{
		m_stateDisabled = xr_new<T>();
		m_stateDisabled->SetAutoDelete(true);
		AttachChild(m_stateDisabled);
	}

	this->m_stateDisabled->InitTexture(texture_d);
	this->m_stateDisabled->SetWndPos(Fvector2().set(0,0));
	this->m_stateDisabled->SetWndSize(size);

}

template <class T>
void CUIInteractiveBackground<T>::InitHighlightedState(LPCSTR texture_h){
	Fvector2 size = GetWndSize();
    
	if (!m_stateHighlighted)
	{
		m_stateHighlighted = xr_new<T>();
		m_stateHighlighted->SetAutoDelete(true);
		AttachChild(m_stateHighlighted);		
	}

	this->m_stateHighlighted->InitTexture(texture_h);
	this->m_stateHighlighted->SetWndPos(Fvector2().set(0,0));
	this->m_stateHighlighted->SetWndSize(size);
}

template <class T>
void CUIInteractiveBackground<T>::InitTouchedState(LPCSTR texture_t){
	Fvector2 size = GetWndSize();

    if (!m_stateTouched)
	{
		m_stateTouched = xr_new<T>();
		m_stateTouched->SetAutoDelete(true);
		AttachChild(m_stateTouched);		
	}

	this->m_stateTouched->InitTexture(texture_t);
	this->m_stateTouched->SetWndPos(Fvector2().set(0,0));
	this->m_stateTouched->SetWndSize(size);
}

template <class T>
void CUIInteractiveBackground<T>::SetState(UIState state){
	switch (state)
	{
	case S_Enabled:
		this->m_stateCurrent = this->m_stateEnabled;
		break;
	case S_Disabled:
		this->m_stateCurrent = this->m_stateDisabled ? this->m_stateDisabled : this->m_stateEnabled;
		break;
	case S_Highlighted:
		this->m_stateCurrent = this->m_stateHighlighted ? this->m_stateHighlighted : this->m_stateEnabled;
		break;
	case S_Touched:
		this->m_stateCurrent = this->m_stateTouched ? this->m_stateTouched : this->m_stateEnabled;
	}
}

template <class T>
void CUIInteractiveBackground<T>::Draw(){
	if (m_stateCurrent)
        m_stateCurrent->Draw();
}

template <class T>
void CUIInteractiveBackground<T>::SetWidth(float width){
	if (m_stateEnabled)
		m_stateEnabled->SetWidth(width);
	if (m_stateDisabled)
		m_stateDisabled->SetWidth(width);
	if (m_stateHighlighted)
		m_stateHighlighted->SetWidth(width);
	if (m_stateTouched)
		m_stateTouched->SetWidth(width);
}

template <class T>
void CUIInteractiveBackground<T>::SetHeight(float heigth){
	if (m_stateEnabled)
		m_stateEnabled->SetHeight(heigth);
	if (m_stateDisabled)
		m_stateDisabled->SetHeight(heigth);
	if (m_stateHighlighted)
		m_stateHighlighted->SetHeight(heigth);
	if (m_stateTouched)
		m_stateTouched->SetHeight(heigth);
}

typedef CUIInteractiveBackground<CUIFrameWindow> CUI_IB_FrameWindow;
typedef CUIInteractiveBackground<CUIFrameLineWnd> CUI_IB_FrameLineWnd;
