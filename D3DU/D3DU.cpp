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

#include "StdAfx.h"
#include "D3DU.h"

class D3DU_NOVTABLE CWindowTarget :
  public ID3DUWindowTarget
{
public:

  BEGIN_INTERFACE_MAP
    INTERFACE_MAP_ENTRY(ID3DUTarget)
    INTERFACE_MAP_ENTRY(ID3DUWindowTarget)
  END_INTERFACE_MAP

  CWindowTarget() { }

  STDMETHOD(Construct)(
    UINT x,
    UINT y,
    UINT width,
    UINT height,
    D3D_FEATURE_LEVEL fl,
    BOOL acceptSw,
    HWND parent)
  {
    HRESULT hr;
    _wState = D3DU_WINDOW_CLOSED;
    hr = InitWindow(x, y, width, height, parent);
    if(FAILED(hr))
      return hr;
    hr = InitDevice(width, height, fl, acceptSw);
    if(FAILED(hr))
      return hr;
    hr = InitTargets(width, height, fl);
    if(FAILED(hr))
      return hr;
    _wState = D3DU_WINDOW_NORMALIZED;
    return S_OK;
  }
  virtual ~CWindowTarget()
  {
    if(_hwnd)
    {
      DestroyWindow(_hwnd);
      _hwnd = NULL;
    }
  }
  STDMETHOD(Render)()
  {
    switch(_wState)
    {
    case D3DU_WINDOW_CLOSED:
      {
#ifdef D3DU_DEBUG
        OutputDebugString(L"Window is closed! Unable to render!\n");
#endif
        return E_FAIL;
      }
    case D3DU_WINDOW_MINIMIZED:
    case D3DU_WINDOW_HIDDEN:
      break;
    case D3DU_WINDOW_RESIZING:
      _swapChain->Present(0, 0);
      break;
    default:
      if(_frameSink)
        _frameSink->RenderFrame(this);
      _swapChain->Present(0, 0);
    }
    return S_OK;
  }
  STDMETHOD(GetFrameSink)(ID3DUFrameSink **oSink)
  {
    if(!oSink)
      return E_POINTER;
    _frameSink.AddRef();
    *oSink = _frameSink;
    return S_OK;
  }
  STDMETHOD(SetFrameSink)(ID3DUFrameSink *sink)
  {
    if(_frameSink)
      _frameSink->Detach(this);
    _frameSink = sink;
    if(sink)
      sink->Attach(this);
    return S_OK;
  }  
  STDMETHOD(GetDevice)(ID3D11Device **oDevice)
  {
    if(!oDevice)
      return E_POINTER;
    _device.AddRef();
    *oDevice = _device;
    return S_OK;
  }
  STDMETHOD(GetDevice10)(ID3D10Device1 **oDevice)
  {
    if(!oDevice)
      return E_POINTER;
    _device10.AddRef();
    *oDevice = _device10;
    return S_OK;
  }
  STDMETHOD(GetDC)(ID3D11DeviceContext **oDC)
  {
    if(!oDC)
      return E_POINTER;
    _dc.AddRef();
    *oDC = _dc;
    return S_OK;
  }
  STDMETHOD(GetFrameRTV)(ID3D11RenderTargetView **oRTV)
  {
    if(!oRTV)
      return E_POINTER;
    _rtv.AddRef();
    *oRTV = _rtv;
    return S_OK;
  }
  STDMETHOD(GetFrameDSV)(ID3D11DepthStencilView **oDSV)
  {
    if(!oDSV)
      return E_POINTER;
    _dsv.AddRef();
    *oDSV = _dsv;
    return S_OK;
  }
  STDMETHOD(GetKeySink)(ID3DUKeySink **oSink)
  {
    if(!oSink)
      return E_POINTER;
    _keySink.AddRef();
    *oSink = _keySink;
    return S_OK;
  }
  STDMETHOD(SetKeySink)(ID3DUKeySink *sink)
  {
    if(_keySink)
      _keySink->Detach(this);
    _keySink = sink;
    if(sink)
      sink->Attach(this);
    return S_OK;
  }
  STDMETHOD(GetMouseSink)(ID3DUMouseSink **oSink)
  {
    if(!oSink)
      return E_POINTER;
    _mouseSink.AddRef();
    *oSink = _mouseSink;
    return S_OK;
  }
  STDMETHOD(SetMouseSink)(ID3DUMouseSink *sink)
  {
    if(_mouseSink)
      _mouseSink->Detach(this);
    _mouseSink = sink;
    if(sink)
      sink->Attach(this);
    return S_OK;
  }
  STDMETHOD(GetWindowState)(D3DU_WINDOW_STATE *oState)
  {
    if(!oState)
      return E_POINTER;
    *oState = _wState;
    return S_OK;
  }
  STDMETHOD(SetWindowState)(D3DU_WINDOW_STATE state)
  {
    if(_wState == state)
      return S_OK;
    if(!_hwnd)
    {
#ifdef D3DU_DEBUG
      OutputDebugString(L"Window is closed.\n");
#endif
      return E_FAIL;
    }
    switch(state)
    {
    case D3DU_WINDOW_CLOSED:
      _swapChain->SetFullscreenState(FALSE, NULL);
      DestroyWindow(_hwnd);
      return S_OK;
    case D3DU_WINDOW_NORMALIZED:
      _swapChain->SetFullscreenState(FALSE, NULL);
      ShowWindow(_hwnd, SW_SHOWNORMAL);
      return S_OK;
    case D3DU_WINDOW_HIDDEN:
      _swapChain->SetFullscreenState(FALSE, NULL);
      ShowWindow(_hwnd, SW_HIDE);
      return S_OK;
    case D3DU_WINDOW_MAXIMIZED:
      _swapChain->SetFullscreenState(FALSE, NULL);
      ShowWindow(_hwnd, SW_SHOWMAXIMIZED);
      return S_OK;
    case D3DU_WINDOW_MINIMIZED:
      _swapChain->SetFullscreenState(FALSE, NULL);
      ShowWindow(_hwnd, SW_SHOWMINNOACTIVE);
      return S_OK;
    case D3DU_WINDOW_RESIZING:
      return E_INVALIDARG;
    case D3DU_WINDOW_FULLSCREEN:
      {
        HRESULT hr = _swapChain->SetFullscreenState(TRUE, _output);
        if(SUCCEEDED(hr))
          _wState = D3DU_WINDOW_FULLSCREEN;
        return hr;
      }
    default:
      return E_INVALIDARG;
    }
  }
private:
  static LPCWSTR className;
  ULONG _ref;
  HWND _hwnd;
  D3DU_WINDOW_STATE _wState;
  ComPtr<ID3DUFrameSink> _frameSink;
  ComPtr<ID3DUKeySink> _keySink;
  ComPtr<ID3DUMouseSink> _mouseSink;
  ComPtr<IDXGIFactory> _factory;
  ComPtr<IDXGIDevice> _dxgiDevice;
  ComPtr<IDXGIAdapter> _adapter;
  ComPtr<IDXGIOutput> _output;
  ComPtr<IDXGISwapChain> _swapChain;
  ComPtr<ID3D11Device> _device;
  ComPtr<ID3D11DeviceContext> _dc;
  ComPtr<ID3D10Device1> _device10;
  ComPtr<ID3D11RenderTargetView> _rtv;
  ComPtr<ID3D11DepthStencilView> _dsv;
  ComPtr<ID3D11Texture2D> _ds;

  static LPCWSTR STDMETHODCALLTYPE InitClass()
  {
    WNDCLASSEX wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = L"CWindowTarget";
    wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    if(INVALID_ATOM == RegisterClassEx(&wc))
      throw HRESULT_FROM_WIN32(GetLastError());
    return wc.lpszClassName;
  }

  static LRESULT COM_DECLSPEC_NOTHROW CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    if(WM_CREATE == msg)
    {
      CWindowTarget *target = (CWindowTarget*)((LPCREATESTRUCT)lParam)->lpCreateParams;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)target);
      return TRUE;
    }
    CWindowTarget* target = (CWindowTarget*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(!target)
    {      
      return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    switch(msg)
    {
    case WM_DESTROY:
      {
        if(target->_frameSink)
          target->_frameSink->Detach(target);
        if(target->_keySink)
          target->_keySink->Detach(target);
        if(target->_mouseSink)
          target->_mouseSink->Detach(target);
        target->_hwnd = NULL;
        target->_wState = D3DU_WINDOW_CLOSED;
      }
      break;
    case WM_ERASEBKGND:
      break;
    case WM_PAINT:
      {

        target->Render();
      }
      break;
    case WM_ENTERSIZEMOVE:
      {
        target->_wState = D3DU_WINDOW_RESIZING;
      }
      break;
    case WM_EXITSIZEMOVE:
      {
        target->_wState = D3DU_WINDOW_NORMALIZED;
        target->ResizeTargets();
      }
      break;
    case WM_SHOWWINDOW:
      if(!wParam)
        target->_wState = D3DU_WINDOW_HIDDEN;
      break;
    case WM_SIZE:
      switch(target->_wState)
      {
      case D3DU_WINDOW_CLOSED:
      case D3DU_WINDOW_RESIZING:
        break;
      default:
        switch(wParam)
        {
        case SIZE_RESTORED:
          target->_wState = D3DU_WINDOW_NORMALIZED;
          target->ResizeTargets();
          break;
        case SIZE_MINIMIZED:
          target->_wState = D3DU_WINDOW_HIDDEN;
          break;
        case SIZE_MAXIMIZED:
          target->_wState = D3DU_WINDOW_MAXIMIZED;
          target->ResizeTargets();
          break;        
        }
        break;
      }
      break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      {
        if(target->_keySink)
        {
          DWORD sks = 0;
          if(0x80 & GetKeyState(VK_CONTROL))
            sks |= D3DU_SKS_CTRL;
          if(0x80 & GetKeyState(VK_MENU))
            sks |= D3DU_SKS_ALT;
          if(0x80 & GetKeyState(VK_SHIFT))
            sks |= D3DU_SKS_SHIFT;
          target->_keySink->KeyDown(target, (DWORD)wParam, sks);
        }
      }      
      break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
      {
        if(target->_keySink)
        {
          DWORD sks = 0;
          if(0x80 & GetKeyState(VK_CONTROL))
            sks |= D3DU_SKS_CTRL;
          if(0x80 & GetKeyState(VK_MENU))
            sks |= D3DU_SKS_ALT;
          if(0x80 & GetKeyState(VK_SHIFT))
            sks |= D3DU_SKS_SHIFT;
          target->_keySink->KeyUp(target, (DWORD)wParam, sks);
        }
      }
      break;
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
  }

  HRESULT COM_DECLSPEC_NOTHROW STDMETHODCALLTYPE InitWindow(UINT x, UINT y, UINT width, UINT height, HWND parent)
  {    
    HRESULT hr = S_OK;
    DWORD exStyle = 0;
    DWORD style = WS_VISIBLE;
    if(parent)
    {
      style |= WS_CHILD;
    }
    else
    {
      exStyle |= WS_EX_COMPOSITED;
      style |= WS_OVERLAPPEDWINDOW;
    }
    _hwnd = CreateWindowEx(
      exStyle,
      className,
      L"",
      style,
      x,
      y,
      width,
      height,
      parent,
      NULL,
      GetModuleHandle(NULL),
      this);
    if(!_hwnd)
      return HRESULT_FROM_WIN32(GetLastError());
    if(!parent)
    {
      MARGINS m = {-1};
      hr = DwmExtendFrameIntoClientArea(_hwnd, &m);
    }
    return hr;
  }

  HRESULT COM_DECLSPEC_NOTHROW STDMETHODCALLTYPE InitDevice(UINT width, UINT height, D3D_FEATURE_LEVEL fl, BOOL acceptSw)
  {
    HRESULT hr;
    D3D_DRIVER_TYPE dTypes[] =
    {
      D3D_DRIVER_TYPE_HARDWARE,
      acceptSw ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE,
      acceptSw ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_HARDWARE,
    };
    D3D_FEATURE_LEVEL fLevels[] = 
    {
      fl,
    };
    DWORD flags = 0;
#ifdef D3DU_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
#endif
    for(int i = 0; i < ARRAYSIZE(dTypes); ++i)
    {
      hr = D3D11CreateDevice(
        NULL,
        dTypes[i],
        NULL,
        flags,
        fLevels,
        ARRAYSIZE(fLevels),
        D3D11_SDK_VERSION,
        &_device,
        &fl,
        &_dc);
      if(SUCCEEDED(hr))
        break;
    }
    if(FAILED(hr))
    {
#ifdef D3DU_DEBUG
      OutputDebugString(L"Unable to create device.\n");
#endif
      return hr;
    }
    hr = _device->QueryInterface(
      __uuidof(*_dxgiDevice),
      (void**)(IDXGIDevice**)&_dxgiDevice);
    if(FAILED(hr))
      return hr;
    hr = _dxgiDevice->GetAdapter(&_adapter);
    if(FAILED(hr))
      return hr;
    hr = _adapter->GetParent(
      __uuidof(*_factory),
      (void**)(IDXGIFactory**)&_factory);
    if(FAILED(hr))
      return hr;
    D3D10_DRIVER_TYPE dTypes10[] =
    {
      D3D10_DRIVER_TYPE_HARDWARE,
      acceptSw ? D3D10_DRIVER_TYPE_WARP : D3D10_DRIVER_TYPE_HARDWARE,
      acceptSw ? D3D10_DRIVER_TYPE_REFERENCE : D3D10_DRIVER_TYPE_REFERENCE,
    };
    DWORD flags10 = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef D3DU_DEBUG
    flags10 |= D3D10_CREATE_DEVICE_DEBUG | D3D10_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
#endif
    for(int i = 0; i < ARRAYSIZE(dTypes10); ++i)
    {
      hr = D3D10CreateDevice1(
        _adapter,
        dTypes10[i],
        NULL,
        flags10,
        D3D10_FEATURE_LEVEL_10_0,
        D3D10_1_SDK_VERSION,
        &_device10);
      if(SUCCEEDED(hr))
        break;
    }
    if(FAILED(hr))
    {
#ifdef D3DU_DEBUG
      OutputDebugString(L"Unable to create 10.1 device.\n");
#endif
      return hr;
    }
    hr = _adapter->EnumOutputs(0, &_output);

    if(FAILED(hr))
      return hr;    
    DXGI_SWAP_CHAIN_DESC sd;
    memset(&sd, 0, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.Windowed = TRUE;
    sd.OutputWindow = _hwnd;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = fl > D3D_FEATURE_LEVEL_10_0 ? 4 : 1 ;
    sd.SampleDesc.Quality = fl > D3D_FEATURE_LEVEL_10_0 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
    DEVMODE dm;
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = sizeof(dm);
    if(!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
    {
#ifdef D3DU_DEBUG
      OutputDebugString(L"Unable to obtain display settings.\n");
#endif
      return HRESULT_FROM_WIN32(GetLastError());
    }
    sd.BufferDesc.RefreshRate.Numerator = dm.dmDisplayFrequency;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    hr = _factory->CreateSwapChain(_dxgiDevice, &sd, &_swapChain);
    if(FAILED(hr))
    {
#ifdef D3DU_DEBUG
      OutputDebugString(L"Unable to create swap chain.\n");
#endif
      return hr;
    }
    return S_OK;
  }

  HRESULT COM_DECLSPEC_NOTHROW STDMETHODCALLTYPE InitTargets(UINT width, UINT height, D3D_FEATURE_LEVEL fl)
  {
    HRESULT hr;
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = _swapChain->GetBuffer(0, __uuidof(*backBuffer),(void**)(ID3D11Texture2D**)&backBuffer);
    if(FAILED(hr))
      return hr;
    hr = _device->CreateRenderTargetView(backBuffer, NULL, &_rtv);
    if(FAILED(hr))
      return hr;
    D3D11_TEXTURE2D_DESC td;
    memset(&td, 0, sizeof(td));
    td.ArraySize = 1;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    td.Format = DXGI_FORMAT_D32_FLOAT;
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.SampleDesc.Quality = fl > D3D_FEATURE_LEVEL_10_0 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
    td.SampleDesc.Count = fl > D3D_FEATURE_LEVEL_10_0 ? 4 : 1;
    hr = _device->CreateTexture2D(&td, NULL, &_ds);
    if(FAILED(hr))
      return hr;
    hr = _device->CreateDepthStencilView(_ds, NULL, &_dsv);
    if(FAILED(hr))
      return hr;
    return S_OK;
  }

  void COM_DECLSPEC_NOTHROW STDMETHODCALLTYPE ResizeTargets()
  {
    switch(_wState)
    {
      case D3DU_WINDOW_CLOSED:
      case D3DU_WINDOW_HIDDEN:
      case D3DU_WINDOW_MINIMIZED:
      case D3DU_WINDOW_RESIZING:
        return;
    }
    RECT rc;
    GetClientRect(_hwnd, &rc);
    if((rc.right - rc.left) <= 0
      || (rc.bottom - rc.top) <= 0)
      return;
    DXGI_SWAP_CHAIN_DESC sd;
    D3D11_TEXTURE2D_DESC td;
    _swapChain->GetDesc(&sd);
    _ds->GetDesc(&td);
    td.Width = rc.right - rc.left;
    td.Height = rc.bottom - rc.top;
    _dc->ClearState();
    _rtv.Release();
    _dsv.Release();
    _ds.Release();
    _swapChain->ResizeBuffers(sd.BufferCount, td.Width, td.Height, sd.BufferDesc.Format, sd.Flags);
    ComPtr<ID3D11Texture2D> backBuffer;
    _swapChain->GetBuffer(0, __uuidof(*backBuffer), (void**)(ID3D11Texture2D**)&backBuffer);
    _device->CreateRenderTargetView(backBuffer, NULL, &_rtv);
    _device->CreateTexture2D(&td, NULL, &_ds);
    _device->CreateDepthStencilView(_ds, NULL, &_dsv);
    if(_frameSink)
      _frameSink->Resize(this, td.Width, td.Height);
  }
};

LPCWSTR CWindowTarget::className = CWindowTarget::InitClass();

D3DU_EXTERN HRESULT D3DU_API D3DUCreateWindowTarget(
  UINT x,
  UINT y,
  UINT width,
  UINT height,
  HWND parent,
  D3D_FEATURE_LEVEL featureLevel,
  BOOL acceptSoftwareDriver,
  ID3DUWindowTarget **oTarget)
{
  *oTarget = NULL;
  HRESULT hr;
  ComObject<CWindowTarget> *target = new ComObject<CWindowTarget>();
  hr = target->Construct(x, y, width, height, featureLevel, acceptSoftwareDriver, parent);
  if(FAILED(hr))
  {
    delete target;
    return hr;
  }
  *oTarget = target;
  return S_OK;
}