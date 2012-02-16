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

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <D3DU.h>
#include <ComUtils.hpp>
#include <sstream>

const CHAR shaders[] =
  "float4 VS(float3 pos : POSITION) : SV_POSITION { return float4(pos, 1); } \n"
  "float4 PS(float4 pos : SV_POSITION) : SV_TARGET { return float4(1, 1, 0, 1); } \n";

class D3DU_NOVTABLE CTriangle :
  public ID3DUFrameSink
{
public:

  BEGIN_INTERFACE_MAP
    INTERFACE_MAP_ENTRY(ID3DUSink)
    INTERFACE_MAP_ENTRY(ID3DUFrameSink)
  END_INTERFACE_MAP

  CTriangle() { }

  virtual ~CTriangle() { }

  STDMETHOD_(void, Attach)(ID3DUTarget *target)
  {
    HRESULT hr;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11Buffer> vb;
    ComPtr<ID3D11VertexShader> vs;
    ComPtr<ID3D11PixelShader> ps;
    ComPtr<ID3D11InputLayout> il;
    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> err;
    XMFLOAT3 vertices[] =
    {
      XMFLOAT3(-0.7f, -0.7f, 0.0f),
      XMFLOAT3( 0.0f,  0.7f, 0.0f),
      XMFLOAT3( 0.7f, -0.7f, 0.0f),
    };
    D3D11_BUFFER_DESC bd = {0};
    D3D11_SUBRESOURCE_DATA sd = {0};
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = sizeof(vertices);
    sd.pSysMem = vertices;
    hr = target->GetDevice(&device);
    if(FAILED(hr))
    {
#ifdef D3DU_DEBUG
      OutputDebugString(L"Triangle: Failed to obtain device");
#endif
      return;
    }
    hr = device->CreateBuffer(&bd, &sd, &vb);
    if(FAILED(hr)) return;
    hr = D3DCompile(
      shaders,
      sizeof(shaders),
      "Vertex shader",
      NULL,
      NULL,
      "VS",
      "vs_4_0",
      0,
      0,
      &blob,
      &err);
    if(FAILED(hr))
    {
#ifdef D3DU_DEBUG
      if(err)
        OutputDebugStringA((LPCSTR)err->GetBufferPointer());
#endif
      return;
    }
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), blob->GetBufferPointer(), blob->GetBufferSize(), &il);
    if(FAILED(hr)) return;
    hr = device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &vs);    
    blob.Release();
    err.Release();
    hr = D3DCompile(
      shaders,
      sizeof(shaders),
      "Pixel shader",
      NULL,
      NULL,
      "PS",
      "ps_4_0",
      0,
      0,
      &blob,
      &err);
    if(FAILED(hr))
    {
#ifdef D3DU_DEBUG
      if(err)
        OutputDebugStringA((LPCSTR)err->GetBufferPointer());
#endif
      return;
    }
    hr = device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &ps);
    if(FAILED(hr)) return;
    _vb = vb;
    _vs = vs;
    _ps = ps;
    _il = il;
    ComPtr<ID3D11RenderTargetView> rtv;
    hr = target->GetFrameRTV(&rtv);
    if(FAILED(hr)) return;
    ComPtr<ID3D11Texture2D> backBuffer;
    rtv->GetResource((ID3D11Resource**)&backBuffer);
    D3D11_TEXTURE2D_DESC td;
    backBuffer->GetDesc(&td);
    vp.Width = (FLOAT)td.Width;
    vp.Height = (FLOAT)td.Height;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
  }

  STDMETHOD_(void, RenderFrame)(ID3DUTarget *target)
  {
    FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    ComPtr<ID3D11DeviceContext> dc;
    ComPtr<ID3D11RenderTargetView> rtv;
    target->GetDC(&dc);
    target->GetFrameRTV(&rtv);
    
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dc->IASetInputLayout(_il);
    UINT strides[] = { sizeof(XMFLOAT3) };
    UINT offsets[] = { 0 };
    dc->IASetVertexBuffers(0, 1, &_vb, strides, offsets);
    dc->OMSetRenderTargets(1, &rtv, NULL);
    dc->RSSetViewports(1, &vp);
    dc->ClearRenderTargetView(rtv, clearColor);
    dc->VSSetShader(_vs, NULL, 0);
    dc->PSSetShader(_ps, NULL, 0);

    dc->Draw(3, 0);
  }

  STDMETHOD_(void, Detach)(ID3DUTarget *target)
  {
    _vb.Release();
    _vs.Release();
    _ps.Release();
    _il.Release();
  }

  STDMETHOD_(void, Resize)(ID3DUTarget *target, UINT width, UINT height)
  {
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
  }

private:
  ComPtr<ID3D11Buffer> _vb;
  ComPtr<ID3D11VertexShader> _vs;
  ComPtr<ID3D11PixelShader> _ps;
  ComPtr<ID3D11InputLayout> _il;
  D3D11_VIEWPORT vp;
};

INT WINAPI wWinMain(
  HINSTANCE instance,
  HINSTANCE prevInstance,
  LPWSTR cmdLine,
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
    FALSE,
    &target);
  if(FAILED(hr))
  {
    std::wstringstream s;
    s << "Failed to create Window Target: " << hr;
    MessageBox(NULL, s.str().c_str(), L"Error", MB_ICONERROR);
    return 1;
  }
  
  ComPtr< ComObject<CTriangle> > sink = new ComObject<CTriangle>();
  target->SetFrameSink(sink);
  
  MSG msg;
  D3DU_WINDOW_STATE state;
  do
  {
    GetMessage(&msg, NULL, 0, 0);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    target->GetWindowState(&state);
  } while(D3DU_WINDOW_CLOSED != state);
  return 0;
}