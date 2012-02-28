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

#include <sstream>
#include <windows.h>
#include <D3DU.h>
#include <ComUtils.hpp>
#include <xnamath.h>
#include "Resource.h"

typedef struct
{
  FLOAT x, y, z, u, v;
} Vertex;

typedef struct
{
  XMMATRIX worldViewProj;
} VsBuffer;

typedef struct
{
  XMVECTOR color1, color2;
} PsBuffer;

class D3DU_NOVTABLE CMandelbrotCube :
  public virtual ID3DUFrameSink,
  public virtual ID3DUKeySink,
  public virtual ID3DUSink,
  public virtual IUnknown
{
public:
  BEGIN_INTERFACE_MAP
    INTERFACE_MAP_ENTRY(ID3DUSink)
    INTERFACE_MAP_ENTRY(ID3DUFrameSink)
    INTERFACE_MAP_ENTRY(ID3DUKeySink)
  END_INTERFACE_MAP

  CMandelbrotCube()
  {
    _initialized = FALSE;
  }

  STDMETHOD_(void, Attach)(ID3DUTarget *target)
  {
    if(_initialized)
      return;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> dc;
    ComPtr<ID3DBlob> blob;
    UINT width;
    UINT height;
    HRESULT hr;
    D3D11_BUFFER_DESC bd = {0};
    D3D11_SUBRESOURCE_DATA sd = {0};
    target->GetDevice(&device);
    target->GetDC(&dc);
    target->GetSize(&width, &height);

    Vertex vertices[] =
    {
      // front
      { -1, -1, -1, 0, 1 },
      { -1, 1, -1, 0, 0  },
      { 1, 1, -1, 1, 0   },
      { 1, -1, -1, 1, 1  },

      //back
      { 1, -1, 1, 0, 1  },
      { 1, 1, 1, 0, 0   },
      { -1, 1, 1, 1, 0  },
      { -1, -1, 1, 1, 1 },

      //top
      { -1, 1, -1, 0, 1 },
      { -1, 1, 1, 0, 0  },
      { 1, 1, 1, 1, 0   },
      { 1, 1, -1, 1, 1  },

      //bottom
      { -1, -1, 1, 0, 1  },
      { -1, -1, -1, 0, 0 },
      { 1, -1, -1, 1, 0  },
      { 1, -1, 1, 1, 1   },

      //left
      { -1, -1, 1, 0, 1 },
      { -1, 1, 1, 0, 0  },
      { -1, 1, -1, 1, 0   },
      { -1, -1, -1, 1, 1  },

      //right
      { 1, -1, -1, 0, 1 },
      { 1, 1, -1, 0, 0  },
      { 1, 1, 1, 1, 0   },
      { 1, -1, 1, 1, 1  },
    };

    DWORD indices[] =
    {
      //front
      0, 1, 2,
      2, 3, 0,

      //back
      4, 5, 6,
      6, 7, 4,

      //top
      8, 9, 10,
      10, 11, 8,

      //bottom
      12, 13, 14,
      14, 15, 12,

      //left
      16, 17, 18,
      18, 19, 16,

      //right
      20, 21, 22,
      22, 23, 20,
    };

    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = sizeof(vertices);
    sd.pSysMem = vertices;
    hr = device->CreateBuffer(&bd, &sd, &_vb);
    if(FAILED(hr)) return;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = sizeof(indices);
    sd.pSysMem = indices;
    if(FAILED(hr)) return;
    hr = device->CreateBuffer(&bd, &sd, &_ib);
    if(FAILED(hr)) return;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.ByteWidth = sizeof(VsBuffer);
    hr = device->CreateBuffer(&bd, NULL, &_vsCb);
    if(FAILED(hr)) return;
    bd.ByteWidth = sizeof(PsBuffer);
    hr = device->CreateBuffer(&bd, NULL, &_psCb);
    if(FAILED(hr)) return;

    DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL2 | D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef D3DU_DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG;
#endif    
    hr = D3DUCompileFromResource(
      GetModuleHandle(NULL),
      MAKEINTRESOURCE(ID_SHADER),
      MAKEINTRESOURCE(RT_SHADER),
      "VS",
      "vs_4_0",
      shaderFlags,
      &blob);
    if(FAILED(hr)) return;
    hr = device->CreateVertexShader(
      blob->GetBufferPointer(),
      blob->GetBufferSize(),
      NULL,
      &_vs);
    if(FAILED(hr)) return;
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(FLOAT)*3, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    hr = device->CreateInputLayout(
      layout,
      ARRAYSIZE(layout),
      blob->GetBufferPointer(),
      blob->GetBufferSize(),
      &_il);
    if(FAILED(hr)) return;
    blob.Release();
    hr = D3DUCompileFromResource(
      GetModuleHandle(NULL),
      MAKEINTRESOURCE(ID_SHADER),
      MAKEINTRESOURCE(RT_SHADER),
      "PS",
      "ps_4_0",
      shaderFlags,
      &blob);
    if(FAILED(hr)) return;

    hr = device->CreatePixelShader(
      blob->GetBufferPointer(),
      blob->GetBufferSize(),
      NULL,
      &_ps);
    if(FAILED(hr)) return;
    
    _vp.Width = (FLOAT)width;
    _vp.Height = (FLOAT)height;
    _vp.TopLeftX = 0.0f;
    _vp.TopLeftY = 0.0f;
    _vp.MinDepth = 0.0f;
    _vp.MaxDepth = 1.0f;
    dc->RSSetViewports(1, &_vp);
    _world = XMMatrixIdentity();
    XMVECTOR eye = XMVectorSet(1.0f, 1.0f, -3.0f, 1.0f);
    XMVECTOR at = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
    _view = XMMatrixLookAtLH(eye, at, up);
    _proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, _vp.Width/_vp.Height, 0.001f, 100.0f);
    
    hr = D3DUCreateFloatAnimation(0, 1, 1, TRUE, TRUE, &_colorAnimation);
    if(FAILED(hr)) return;
    _colorAnimation->Start();

    hr = D3DUCreateFloatAnimation(0, XM_2PI, 10, TRUE, FALSE, &_cubeAnimation);
    if(FAILED(hr)) return;
    _cubeAnimation->Start();

    _initialized = TRUE;
  }

  STDMETHOD_(void, Detach)(ID3DUTarget *target)
  {
    if(!_initialized)
      return;
    _colorAnimation->Stop();
    _colorAnimation.Release();
    _vb.Release();
    _ib.Release();
    _vsCb.Release();
    _psCb.Release();
    _vs.Release();
    _ps.Release();
    _il.Release();
    _initialized = FALSE;
  }

  STDMETHOD_(void, RenderFrame)(ID3DUTarget *target)
  {
    if(!_initialized)
      return;
    ComPtr<ID3D11DeviceContext> dc;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11DepthStencilView> dsv;
    VsBuffer vsCb;
    PsBuffer psCb;
    FLOAT clearColor[4] = {0.0f, 0.4f, 1.0f, 1.0f};

    FLOAT colorScale;
    _colorAnimation->Query(&colorScale);
    XMVECTOR color1 = XMVectorLerp(XMVectorSet(1, 0, 0, 1), XMVectorSet(0, 0, 1, 1), colorScale),
             color2 = XMVectorLerp(XMVectorSet(1, 1, 0, 1), XMVectorSet(0, 1, 0, 1), colorScale);

    FLOAT angle;
    _cubeAnimation->Query(&angle);
    _world = XMMatrixRotationX(angle) * XMMatrixRotationY(angle);

    target->GetDC(&dc);
    target->GetFrameRTV(&rtv);
    target->GetFrameDSV(&dsv);
    dc->ClearRenderTargetView(rtv, clearColor);
    dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 255);
    dc->OMSetRenderTargets(1, &rtv, dsv);
    vsCb.worldViewProj = _world * _view * _proj;
    psCb.color1 = color1;
    psCb.color2 = color2;
    dc->UpdateSubresource(_vsCb, 0, NULL, &vsCb, 0, 0);
    dc->UpdateSubresource(_psCb, 0, NULL, &psCb, 0, 0);
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dc->IASetInputLayout(_il);
    UINT offset = 0, stride = sizeof(Vertex);
    dc->IASetVertexBuffers(0, 1, &_vb, &stride, &offset);
    dc->IASetIndexBuffer(_ib, DXGI_FORMAT_R32_UINT, 0);
    dc->VSSetConstantBuffers(0, 1, &_vsCb);
    dc->PSSetConstantBuffers(0, 1, &_psCb);
    dc->VSSetShader(_vs, NULL, 0);
    dc->PSSetShader(_ps, NULL, 0);
    
    D3D11_BUFFER_DESC ibd;
    _ib->GetDesc(&ibd);
    dc->DrawIndexed(ibd.ByteWidth / sizeof(DWORD), 0, 0);
  }

  STDMETHOD_(void, Resize)(ID3DUTarget *target, UINT width, UINT height)
  {
    if(!_initialized)
      return;
    ComPtr<ID3D11DeviceContext> dc;
    _vp.Width = (FLOAT)width;
    _vp.Height = (FLOAT)height;
    target->GetDC(&dc);
    dc->RSSetViewports(1, &_vp);
    _proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, _vp.Width/_vp.Height, 0.001f, 100.0f);
  }

  STDMETHOD_(void, KeyDown)(ID3DUWindowTarget *target, DWORD key, DWORD sks)
  {
  }

  STDMETHOD_(void, KeyUp)(ID3DUWindowTarget *target, DWORD key, DWORD sks)
  {
    if(VK_SPACE == key)
    {
      BOOL started;
      ID3DUFloatAnimation *animation = sks & D3DU_SKS_CTRL ? _colorAnimation : _cubeAnimation ;
      animation->GetStatus(&started);
      if(started)
        animation->Stop();
      else
        animation->Start();
    }
  }

private:  
  BOOL _initialized;
  ComPtr<ID3DUFloatAnimation> _colorAnimation;
  ComPtr<ID3DUFloatAnimation> _cubeAnimation;
  ComPtr<ID3D11Buffer> _vb;
  ComPtr<ID3D11Buffer> _ib;
  ComPtr<ID3D11Buffer> _vsCb;
  ComPtr<ID3D11Buffer> _psCb;
  ComPtr<ID3D11VertexShader> _vs;
  ComPtr<ID3D11PixelShader> _ps;
  ComPtr<ID3D11InputLayout> _il;
  D3D11_VIEWPORT _vp;
  XMMATRIX _world;
  XMMATRIX _view;
  XMMATRIX _proj;
};

INT WINAPI WinMain(
  HINSTANCE instance,
  HINSTANCE prevInstance,
  LPSTR cmdLine,
  INT cmdShow)
{
  HRESULT hr;
  ComPtr<ID3DUWindowTarget> target;
  hr = D3DUCreateWindowTarget(
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    640,
    480,
    NULL,
    D3D_FEATURE_LEVEL_10_0,
    TRUE,
    &target);
  if(FAILED(hr))
  {
    std::wstringstream s;
    s << "Failed to create Window Target: " << hr;
    MessageBox(NULL, s.str().c_str(), L"Error", MB_ICONERROR);
    return 1;
  }
  ComPtr<ComObject<CMandelbrotCube> > sink = new ComObject<CMandelbrotCube>();
  target->SetFrameSink(sink);
  target->SetKeySink(sink);
  MSG msg;
  D3DU_WINDOW_STATE state;
  do
  {
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
      target->Render();
    target->GetWindowState(&state);
  } while(D3DU_WINDOW_CLOSED != state);
  return 0;
}
