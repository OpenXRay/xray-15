//-----------------------------------------------------------------------------
// -------------------------
// File ....: BitmapInfo.cpp
// -------------------------
// Author...: Gus J Grubba
// Date ....: September 1998
//
// Implementation of CMaxBitmapInfo
//
//-----------------------------------------------------------------------------
      
#include "stdafx.h"
#include "Comsrv.h"
#include "BitmapInfo.h"

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::CMaxBitmapInfo()

CMaxBitmapInfo::CMaxBitmapInfo() {
	width			= 720;
	height			= 486;
	gamma			= 1.2f;
	aspect			= 1.0f;
	channels		= CHAN_NONE;
	process_gamma	= FALSE;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::get_Width

STDMETHODIMP CMaxBitmapInfo::get_Width(short *pVal) {
	*pVal = width;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::put_Width

STDMETHODIMP CMaxBitmapInfo::put_Width(short newVal) {
	width = newVal;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::get_Height

STDMETHODIMP CMaxBitmapInfo::get_Height(short *pVal) {
	*pVal = height;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::put_Height

STDMETHODIMP CMaxBitmapInfo::put_Height(short newVal) {
	height = newVal;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::get_ProcessGamma

STDMETHODIMP CMaxBitmapInfo::get_ProcessGamma(BOOL *pVal) {
	*pVal = process_gamma;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::put_ProcessGamma

STDMETHODIMP CMaxBitmapInfo::put_ProcessGamma(BOOL newVal) {
	process_gamma = newVal;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::get_Gamma

STDMETHODIMP CMaxBitmapInfo::get_Gamma(float *pVal) {
	*pVal = gamma;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::put_Gamma

STDMETHODIMP CMaxBitmapInfo::put_Gamma(float newVal) {
	gamma = newVal;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::get_Aspect

STDMETHODIMP CMaxBitmapInfo::get_Aspect(float *pVal) {
	*pVal = aspect;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::put_Aspect

STDMETHODIMP CMaxBitmapInfo::put_Aspect(float newVal) {
	aspect = newVal;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::get_Channels

STDMETHODIMP CMaxBitmapInfo::get_Channels(MAXGBufferFlags *pVal) {
	*pVal = (MAXGBufferFlags)channels;
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxBitmapInfo::put_Channels

STDMETHODIMP CMaxBitmapInfo::put_Channels(MAXGBufferFlags newVal) {
	channels = newVal;
	return S_OK;
}

//-- EOF: BitmapInfo.cpp ------------------------------------------------------

