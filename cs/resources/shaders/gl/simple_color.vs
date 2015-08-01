in  vec3 P;
out vec4 C;

uniform vec4 tfactor;

void main(void)
{
    gl_Position = vec4(P.x, P.y, P.z, 1.0);
    C = tfactor;
}
