#include "stdafx.h"
#include "glRenderFactory.h"

//#include "glStatGraphRender.h"
#ifndef _EDITOR
	//#include "glLensFlareRender.h"
#endif
//#include "glConsoleRender.h"
#ifndef _EDITOR
	//#include "glThunderboltRender.h"
	//#include "glThunderboltDescRender.h"
	//#include "glRainRender.h"
	//#include "glLensFlareRender.h"
	//#include "glEnvironmentRender.h"

	//#include "glRenderDeviceRender.h"
	//#include "glObjectSpaceRender.h"
#endif // _EDITOR

//#include "glFontRender.h"
//#include "glApplicationRender.h"
//#include "glWallMarkArray.h"
//#include "glStatsRender.h"
//#include "glUISequenceVideoItem.h"
//#include "glUIShader.h"

glRenderFactory RenderFactoryImpl;

//# include "gl##Class.h" \

#define RENDER_FACTORY_IMPLEMENT(Class) \
	I##Class* glRenderFactory::Create##Class() \
{ \
	return new gl##Class(); \
} \
	void glRenderFactory::Destroy##Class(I##Class *pObject)\
{ \
	xr_delete((gl##Class*&)pObject); \
} \

#define RENDER_FACTORY_UNIMPLEMENT(Class) \
	I##Class* glRenderFactory::Create##Class() \
{ \
	return nullptr; \
} \
	void glRenderFactory::Destroy##Class(I##Class *pObject)\
{ \
} \

#ifndef _EDITOR
	RENDER_FACTORY_UNIMPLEMENT(UISequenceVideoItem)
	RENDER_FACTORY_UNIMPLEMENT(UIShader)
	RENDER_FACTORY_UNIMPLEMENT(StatGraphRender)
	RENDER_FACTORY_UNIMPLEMENT(ConsoleRender)
	RENDER_FACTORY_UNIMPLEMENT(RenderDeviceRender)
#	ifdef DEBUG
		RENDER_FACTORY_UNIMPLEMENT(ObjectSpaceRender)
#	endif // DEBUG
	RENDER_FACTORY_UNIMPLEMENT(ApplicationRender)
	RENDER_FACTORY_UNIMPLEMENT(WallMarkArray)
	RENDER_FACTORY_UNIMPLEMENT(StatsRender)
#endif // _EDITOR

#ifndef _EDITOR
	RENDER_FACTORY_UNIMPLEMENT(ThunderboltRender)
	RENDER_FACTORY_UNIMPLEMENT(ThunderboltDescRender)
	RENDER_FACTORY_UNIMPLEMENT(RainRender)
	RENDER_FACTORY_UNIMPLEMENT(LensFlareRender)
	RENDER_FACTORY_UNIMPLEMENT(EnvironmentRender)
	RENDER_FACTORY_UNIMPLEMENT(EnvDescriptorMixerRender)
	RENDER_FACTORY_UNIMPLEMENT(EnvDescriptorRender)
	RENDER_FACTORY_UNIMPLEMENT(FlareRender)
#endif
RENDER_FACTORY_UNIMPLEMENT(FontRender)
