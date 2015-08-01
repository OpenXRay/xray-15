in  vec3 iP;
in  vec4 iC;
out vec4 oC;

uniform vec4 tfactor;

void main(void)
{
	gl_Position	= i * m_WVP;			// xform, input in world coords
	oC 			= tfactor * iC;
}
