//
//	Misc. noise functions from Texturing and Modeling A Procedural Approach
//  Perlin, Musgrave...
//

float bias(float a, float b);
float gain(float a, float b);

float noise1(float arg);
float noise2(float vec[]);
float noise3(float vec[]);

float turbulence(float *v, float freq);
int Perm(int v);

#define MAX_OCTAVES	50

double fBm1(double point, double H, double lacunarity, double octaves);

