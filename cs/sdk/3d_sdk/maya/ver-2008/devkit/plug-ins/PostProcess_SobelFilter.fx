//
// Sample Sobel filter using a post-process pixel shader
//


// Texture size in u and v. Send over 1/render-target-width,
// and 1/render-target-height
//
const float duKernel;
const float dvKernel;

// Simple user control on edge thickness
float edgeThickness = 80.0;

// Input color texture which is sampled by the pixel shader
texture textureSourceColor;
sampler2D samplerSourceColor =
sampler_state
{
    Texture = <textureSourceColor>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};

// Simple sobel edge detection. We build the filter
// kernel on the fly here based on the texel width and
// height <duKernel,dvKernel>
//
float4 SobelFilter_PS(half2 tc : TEXCOORD) : COLOR0
{
    half2 tc1 = tc + half2(-duKernel, -dvKernel);
	half4 s00 = tex2D(samplerSourceColor, tc1);  

	tc1 = tc + half2(0, -dvKernel);
	half4 s01 = tex2D(samplerSourceColor, tc1);  

	tc1 = tc + half2( duKernel, -dvKernel);
	half4 s02 = tex2D(samplerSourceColor, tc1);  

	tc1 = tc + half2(-duKernel, 0);
	half4 s10 = tex2D(samplerSourceColor, tc1);  

	tc1 = tc + half2( duKernel, -0);
	half4 s12 = tex2D(samplerSourceColor, tc1);  

	tc1 = tc + half2(-duKernel, dvKernel);
	half4 s20 = tex2D(samplerSourceColor, tc1);  

	tc1 = tc + half2(0, -dvKernel);
	half4 s21 = tex2D(samplerSourceColor, tc1);  

	tc1 = tc + half2( duKernel, dvKernel);
	half4 s22 = tex2D(samplerSourceColor, tc1);  

	half4 sx = -(s00 + 2 * s10 + s20) + (s02 + 2 * s12 + s22);
	half4 sy = -(s00 + 2 * s01 + s02) + (s20 + 2 * s21 + s22);

	half4 color = (sx * sx) + (sy * sy);

	//return 1.0 - mul(color,20);
	//return 1.0 - dot(color,20);
	return 1.0 - dot(color, edgeThickness);
	//return 1.0 - sqrt(color);
}

// Post-process technique. Name must be "PostProcess" to be recognized
// as a post-process effect
//
technique PostProcess
{
	Pass P0
	{	
		Lighting     = false;
		ZWriteEnable = false;
		
		VertexShader = null;
		PixelShader  = compile ps_2_0 SobelFilter_PS();
	}
}
