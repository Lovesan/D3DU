// Copyright (C) 2012, Dmitry Ignatiev <lovesan.ru at gmail.com>
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#define MAX_ITERS 80

cbuffer VsBuffer : register(b0)
{
    float4x4 worldViewProj;
};

void VS(float3 pos : POSITION,
        float2 tex : TEXCOORD,
        out float4 oPos : SV_POSITION,
        out float2 oTex : TEXCOORD)
{
    oPos = mul(worldViewProj, float4(pos, 1));
    oTex = tex;
}

cbuffer PsBuffer : register(b0)
{
    float3 color1, color2;
};

float4 PS(float4 pos : SV_POSITION,
          float2 tex : TEXCOORD)
     : SV_TARGET
{
    float x0 = tex.x * 2.5 - 1.75,
          y0 = tex.y * 2 - 1,
          x = 0,
          y = 0,
          xtmp = 0,
          v = 0,
          lenSq = 0;
    int n = 0;
    while((lenSq = (x*x + y*y)) < 4 && n < MAX_ITERS)
    {
        xtmp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtmp;
        ++n;
    }
    v = n - log2(log(sqrt(lenSq))/log(n));
    if(v > MAX_ITERS)
        return float4(0, 0, 0, 1);
    else if(v > MAX_ITERS/4)
        return float4(lerp(color2, float3(1, 1, 1), 4*v/MAX_ITERS - 1), 1);
    else if(v > MAX_ITERS/8)
        return float4(lerp(color1, color2, 8*v/MAX_ITERS - 1), 1);
    else
        return float4(lerp(float3(0, 0, 0), color1, 8*v/MAX_ITERS), 1);
}
