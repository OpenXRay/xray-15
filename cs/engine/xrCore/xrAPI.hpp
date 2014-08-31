#pragma once
#include "xrCore.h"

#ifndef _EDITOR
class IRender_interface;
extern XRCORE_API IRender_interface* Render;

class IRenderFactory;
extern XRCORE_API IRenderFactory* RenderFactory;

class CDUInterface;
extern XRCORE_API CDUInterface* DU;

struct xr_token;
extern XRCORE_API xr_token* vid_mode_token;

class IUIRender;
extern XRCORE_API IUIRender* UIRender;

#ifdef DEBUG
class IDebugRender;
extern XRCORE_API IDebugRender* DRender;
#endif

#else
extern ENGINE_API CRender* Render;

class IRenderFactory;
extern ENGINE_API IRenderFactory* RenderFactory;
#endif
