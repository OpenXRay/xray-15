in vec3 pos;
in vec2 iTex0;
in vec4 oColor;

out vec2 oTex0;
out vec4 oColor;

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
void main ()
{
	gl_Position = pos;
	oTex0 = iTex0;
	oColor = iColor;
}
