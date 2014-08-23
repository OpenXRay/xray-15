///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////
//
// exrdisplay pipeline
//
// Simon Green <SGreen@nvidia.com>
// Drew Hess <dhess@ilm.com>
//
// [ Maya notice: The origin Cg program has been ported to HLSL,
//   some arguments are no longer globallly made controllable ]
//

// Texture size in u and v. Send over 1/render-target-width,
// and 1/render-target-height
//
const float duKernel;
const float dvKernel;

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


// Knee function
float3 knee (float3 x, float f)
{
    return log (x * f + 1) / f;
}

// Note only some controls are exposed to basically match what
// is done for Maya's file texture node.
//
float exposure = 1.0f; // -10 -> 10
const float3 defog = 0.0f; // 0 -> 0.01 
const float kneeLow = 0.0f; // -3 -> 3
const float kneeF = 5.0f; // 3.5 -> 7.0
const float grayTarget = 0.5f;
const float gamma = 1.0/2.2f;
const float3 zerovec = {0,0,0};

float4 OpenEXRFilter_PS( float2 Tex : TEXCOORD0 ) : COLOR0
{
    float3 color;

    color = tex2D( samplerSourceColor, Tex ).xyz;

	// Defog
    color = max(zerovec, color - defog );

	// Exposure
    color  *= pow(2.0, exposure + 2.47393);

    // Knee
    color  = (color  > kneeLow) ? kneeLow + knee (color  - kneeLow, kneeF) : color;

    // Gamma correction
    color  = pow(color, gamma);

    // scale middle gray to the target framebuffer value
    //color  *= grayTarget;
    
    return float4( color, 1.0);
}

technique PostProcess
{
	Pass P0
	{	
		Lighting     = false;
		ZWriteEnable = false;

        VertexShader = null;
        PixelShader = compile ps_2_0 OpenEXRFilter_PS();
    }
}
