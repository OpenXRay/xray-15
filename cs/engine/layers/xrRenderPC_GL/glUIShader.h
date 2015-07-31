#pragma once

#include "..\..\Include\xrRender\UIShader.h"

class glUIShader :
	public IUIShader
{
public:
	virtual void Copy(IUIShader &_in);
	virtual void create(LPCSTR sh, LPCSTR tex = 0);
	virtual bool inited() { return hShader; }
	virtual void destroy();

private:
	ref_shader		hShader;
};
