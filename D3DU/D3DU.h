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

#ifndef __D3DU_H__
#define __D3DU_H__

#if !defined(UNICODE) || !defined(_UNICODE)
#error D3DU requires unicode build
#endif

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#if defined(DEBUG) || defined(_DEBUG)
#define D3DU_DEBUG
#endif

#define D3DU_NOVTABLE __declspec(novtable)
#ifdef D3DU_INTERNALS
#define D3DU_EXTERN extern "C" __declspec(dllexport)
#else
#define D3DU_EXTERN extern "C" __declspec(dllimport)
#endif
#define D3DU_API __stdcall

typedef interface ID3DUFloatAnimation ID3DUFloatAnimation;
typedef interface ID3DUTarget ID3DUTarget;
typedef interface ID3DUWindowTarget ID3DUWindowTarget;
typedef interface ID3DUSink ID3DUSink;
typedef interface ID3DUFrameSink ID3DUFrameSink;
typedef interface ID3DUKeySink ID3DUKeySink;
typedef interface ID3DUMouseSink ID3DUMouseSink;

/// Well, function and argument names are self-explanatory.

D3DU_EXTERN HRESULT D3DU_API D3DUCreateFloatAnimation(
  FLOAT begin,
  FLOAT end,
  FLOAT interval,
  BOOL repeat,
  BOOL autoReverse,
  /* [out] */ ID3DUFloatAnimation **oFloatAnimation);

D3DU_EXTERN HRESULT D3DU_API D3DUCreateWindowTarget(
  UINT x,
  UINT y,
  UINT width,
  UINT height,
  HWND parent,  
  D3D_FEATURE_LEVEL featureLevel,
  BOOL acceptSoftwareDriver,
  /* [out] */ ID3DUWindowTarget **oTarget);

D3DU_EXTERN HRESULT D3DU_API D3DUCompileFromMemory(
  LPCSTR code,
  SIZE_T size,
  LPCSTR entry,  
  LPCSTR target,
  DWORD shaderFlags,
  /* [out] */ ID3DBlob **oCodeBlob);

D3DU_EXTERN HRESULT D3DU_API D3DUCompileFromResource(
  HMODULE module,  
  LPCWSTR resourceName,
  LPCWSTR resourceType,
  LPCSTR entry,  
  LPCSTR target,
  DWORD shaderFlags,
  /* [out] */ ID3DBlob **oCodeBlob);

D3DU_EXTERN HRESULT D3DU_API D3DUCompileFromFile(
  LPCWSTR filename,
  LPCSTR entry,  
  LPCSTR target,
  DWORD shaderFlags,
  /* [out] */ ID3DBlob **oCodeBlob);

// Animation
MIDL_INTERFACE("9D1DA4B4-1DDE-479C-BE3C-A652CAA71540")
ID3DUFloatAnimation : public IUnknown
{
public:
  STDMETHOD(Start)() = 0;
  STDMETHOD(Stop)() = 0;
  STDMETHOD(GetStatus)(/* [out] */ BOOL *oStarted)= 0;
  STDMETHOD(Query)(/* [out] */ FLOAT *oCurrent) = 0;
  STDMETHOD(GetRange)(/* [out] */ FLOAT *oBegin, /* [out] */ FLOAT *oEnd) = 0;
  STDMETHOD(SetRange)(FLOAT begin, FLOAT end) = 0;
  STDMETHOD(GetInterval)(/* [out] */ FLOAT *oSeconds) = 0;
  STDMETHOD(SetInterval)(FLOAT seconds) = 0;
  STDMETHOD(GetRepeat)(/* [out] */ BOOL *oRepeat) = 0;
  STDMETHOD(SetRepeat)(BOOL repeat) = 0;
  STDMETHOD(GetAutoReverse)(/* [out] */ BOOL *oAutoReverse) = 0;
  STDMETHOD(SetAutoReverse)(BOOL autoReverse) = 0;
};

/// Generic renderer interface.
MIDL_INTERFACE("A368DF08-C45B-4FA2-9188-EA5482BF5DC1")
ID3DUTarget : public IUnknown
{
public:  
  STDMETHOD(Render)() = 0;
  STDMETHOD(GetSize)(UINT *oWidth, UINT *oHeight) = 0;
  STDMETHOD(SetSize)(UINT width, UINT height) = 0;
  STDMETHOD(GetFrameSink)(/* [out] */ ID3DUFrameSink **oSink) = 0;
  STDMETHOD(SetFrameSink)(ID3DUFrameSink *sink) = 0;
  STDMETHOD(GetDevice)(/* [out] */ ID3D11Device **oDevice) = 0;
  STDMETHOD(GetDevice10)(/* [out] */ ID3D10Device1 **oDevice) = 0;
  STDMETHOD(GetDC)(ID3D11DeviceContext **oDC) = 0;
  STDMETHOD(GetFrameRTV)(/* [out] */ ID3D11RenderTargetView **oRTV) = 0;
  STDMETHOD(GetFrameDSV)(/* [out] */ ID3D11DepthStencilView **oDSV) = 0;
};

/// ID3DUWindowTarget window state.
typedef enum
{
  D3DU_WINDOW_CLOSED,
  D3DU_WINDOW_HIDDEN,
  D3DU_WINDOW_NORMALIZED,
  D3DU_WINDOW_MINIMIZED,
  D3DU_WINDOW_MAXIMIZED,
  D3DU_WINDOW_RESIZING,
  D3DU_WINDOW_FULLSCREEN,
} D3DU_WINDOW_STATE;

/// Renders into HWND.
MIDL_INTERFACE("5A55CF7E-C7C1-432D-9CDE-E10C8E8E9D59")
ID3DUWindowTarget : public ID3DUTarget
{
public:
  STDMETHOD(GetKeySink)(/* [out] */ ID3DUKeySink **oSink) = 0;
  STDMETHOD(SetKeySink)(ID3DUKeySink *sink) = 0;
  STDMETHOD(GetMouseSink)(/* [out] */ ID3DUMouseSink **oSink) = 0;
  STDMETHOD(SetMouseSink)(ID3DUMouseSink *sink) = 0;  
  STDMETHOD(GetWindowState)(/* [out] */ D3DU_WINDOW_STATE *oState) = 0;
  STDMETHOD(SetWindowState)(D3DU_WINDOW_STATE state) = 0;
};

/// `Sink' interfaces are actually callbacks.
/// Their methods are not intended to be called directly by library user.
MIDL_INTERFACE("C872AC15-0814-45A5-95EA-C6E6E5D0A4C2")
ID3DUSink : public IUnknown
{
public:
  STDMETHOD_(void, Attach)(ID3DUTarget *target) = 0;
  STDMETHOD_(void, Detach)(ID3DUTarget *target) = 0;
};

/// Handles frame rendering events
MIDL_INTERFACE("8808F25A-F8FE-4401-BC04-DB6AD94519A2")
ID3DUFrameSink : public ID3DUSink
{
public:  
  STDMETHOD_(void, RenderFrame)(ID3DUTarget *target) = 0;
  STDMETHOD_(void, Resize)(ID3DUTarget *target, UINT width, UINT height) = 0;
};

/// Describes system keys that were pressed.
typedef enum
{
  D3DU_SKS_CTRL = 1,
  D3DU_SKS_ALT = 2,
  D3DU_SKS_SHIFT = 4,
} D3DU_SYSTEM_KEY_STATE;

/// Handles keyboard events.
MIDL_INTERFACE("5BA81E41-014B-4238-81AB-F3763DBDF071")
ID3DUKeySink : public ID3DUSink
{
  STDMETHOD_(void, KeyDown)(ID3DUWindowTarget *target, DWORD key, DWORD sks) = 0;
  STDMETHOD_(void, KeyUp)(ID3DUWindowTarget *target, DWORD key, DWORD sks) = 0;
};

/// Handles mouse events.
MIDL_INTERFACE("B4BD8521-7E41-488F-8296-8EBC34E69C8E")
ID3DUMouseSink : public ID3DUSink
{
};

#endif // __D3DU_H__