#include "common.h"

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec4 iPos;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	gl_Position = m_WVP * iPos;
}
