/*******************************************************************************
 *
 * Maya_fixedFunction
 *
 *******************************************************************************/

// Matrices
float4x4 wvIT : WorldViewInverseTranspose;
float4x4 wvp : WorldViewProjection;
float4x4 wv : WorldView;

void mainVS( float4 vertex : POSITION0,
             float3 normal : NORMAL0,
             float4 tex : TEXCOORD0,
             out float4 position: POSITION0, out float3 eyeNormal : TEXCOORD0, 
             out float3 eyeView : TEXCOORD1)
{ 

	position = mul( vertex, wvp);
    eyeNormal = mul( normal, (float3x3)wvIT);
	eyeView = -mul( vertex, wv).xyz;
}

void mainTexVS( float4 vertex : POSITION0,
             float3 normal : NORMAL0,
             float4 tex : TEXCOORD0,
             out float4 position: POSITION0, out float3 eyeNormal : TEXCOORD0, 
             out float3 eyeView : TEXCOORD1, out float2 texCoord : TEXCOORD2)
{ 

	position = mul( vertex, wvp);
    eyeNormal = mul( normal, (float3x3)wvIT);
	eyeView = -mul( vertex, wv).xyz;
    texCoord = tex.xy;

}

// Material uniforms
float3 lightDir = float3( 0.57735, 0.57735, 0.57735);
float3 lightColor = float3( 1.0, 1.0, 1.0);
float3 diffuseMaterial = float3( 0.8, 0.2, 0.0);
float  diffuseCoeff = 1.0;
float3 ambientLight = float3( 0.2, 0.2, 0.2);
float shininess = 16.0;
float transparency = 1.0;
float3 specularColor = float3( 1.0, 1.0, 1.0 );

float4 mainPS( float3 eyeNormal : TEXCOORD0, float3 eyeView : TEXCOORD1) : COLOR0
{
  
  //
  //	Since the EyeNormal is getting interpolated, we
  //	have to first restore it by normalizing it.
  //
  float3 norm = normalize( eyeNormal );
  float3 view = normalize(eyeView);
  float3 refl = normalize( -reflect(view,norm));

  float nDotL = saturate( dot( norm, lightDir));
  float rDotL = saturate( dot( refl, lightDir));
  rDotL = (nDotL > 0.0) ? pow( rDotL, shininess) : 0.0;

  float3 color = lightColor*nDotL*diffuseMaterial + diffuseMaterial*ambientLight + 
               rDotL*lightColor;

  return float4( color, 1.0);

}

// Material textured uniforms
texture diffuseTexture;
sampler2D diffuseSampler  =
sampler_state
{
    Texture = <diffuseTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

float4 mainTexPS( float3 eyeNormal : TEXCOORD0, float3 eyeView : TEXCOORD1,
                float2 texCoord : TEXCOORD2) : COLOR0
{
  
  //
  //	Since the EyeNormal is getting interpolated, we
  //	have to first restore it by normalizing it.
  //
  float3 norm = normalize( eyeNormal );
  float3 view = normalize(eyeView);
  float3 refl = normalize( -reflect(view,norm));
  float3 diffuse = tex2D( diffuseSampler, texCoord).rgb;

  float nDotL = saturate( dot( norm, lightDir));
  float rDotL = saturate( dot( refl, lightDir));
  rDotL = (nDotL > 0.0) ? pow( rDotL, shininess) : 0.0;

  float3 color = (lightColor * nDotL * diffuse * diffuseCoeff) + (diffuse * ambientLight) + 
					(rDotL * lightColor * specularColor);

  return float4( color, transparency);

}


//////// techniques ////////////////////////////

technique MayaPhong
{
	pass p0
	{
		VertexShader = compile vs_2_0 mainVS();
		
		ZEnable = true;
		ZWriteEnable = true;
        ZFunc = LessEqual;
		//CullMode = None;

		PixelShader = compile ps_2_0 mainPS();
	}
}

technique MayaPhongTextured
{
	pass p0
	{
		VertexShader = compile vs_2_0 mainTexVS();
		
		ZEnable = true;
		ZWriteEnable = true;
        ZFunc = LessEqual;
		//CullMode = None;

		PixelShader = compile ps_2_0 mainTexPS();
	}
}




