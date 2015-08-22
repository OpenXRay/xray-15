#include "common.h"

layout(location = 0) in vec4 iPos;

void main ()
{
	gl_Position = m_WVP * iPos;
}
