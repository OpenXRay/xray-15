#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = POSITION) in vec4 iPos;

void main ()
{
	gl_Position = m_WVP * iPos;
}
