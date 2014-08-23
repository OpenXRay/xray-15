
// Texture size in u and v. Send over 1/render-target-width,
// and 1/render-target-height
//
const half duKernel;
const half dvKernel;

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

half4 InvertFilter_PS( half2 Tex : TEXCOORD0 ) : COLOR0
{
    return 1.0f - tex2D( samplerSourceColor, Tex );
}

technique PostProcess
{
    pass p0
    {
    	Lighting     = false;
		ZWriteEnable = false;
    
        VertexShader = null;
        PixelShader = compile ps_2_0 InvertFilter_PS();
    }
}
