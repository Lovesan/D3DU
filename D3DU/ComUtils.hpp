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

#ifndef __COM_UTILS_HPP__
#define __COM_UTILS_HPP__

template<typename T>
class ComPtr
{
public:
  ComPtr()
  {
    _p = NULL;
  }
  ComPtr(T *p)
  {
    _p = p;
  }
  ComPtr(const ComPtr<T>& ptr)
  {
    _p=ptr._p;
    if(_p)
      _p->AddRef();
  }
  ~ComPtr()
  {
    if(_p)
    {
      _p->Release();
      _p = NULL;      
    }
  }
  inline T& operator*()
  {
    return *_p;
  }
  inline T* operator->()
  {
    return _p;
  }
  inline T** operator&()
  {
    return &_p;
  }
  inline operator T*()
  {
    return _p;
  }
  inline T* operator=(T* p)
  {
    if(_p)
      _p->Release();
    if(p)
      p->AddRef();
    _p = p;
    return _p;
  }
  inline T* operator=(ComPtr p)
  {
    if(_p)
      _p->Release();
    if(p)
      p->AddRef();
    _p = p._p;
    return _p;
  }
  inline bool operator==(T* p) const
  {
    return _p == p;
  }
  inline bool operator!=(T* p) const
  {
    return !operator==(p);
  }
  inline bool operator!() const
  {
    return NULL == _p;
  }
  inline void AddRef()
  {
    if(_p)
      _p->AddRef();
  }
  inline void Release()
  {
    if(_p)
    {
      _p->Release();
      _p = NULL;
    }
  }
  inline void Attach(T* p)
  {
    if(_p)
      _p->Release();
    _p = p;
  }
  inline void Detach()
  {
    _p = NULL;
  }
  bool IsEqualObject(IUnknown* other)
  {
    if(NULL == _p && NULL == other)
      return true;
    if(NULL == _p || NULL == other)
      return false;
    CComPtr<T> unk1;
    CComPtr<T> unk2;
    _p->QueryInterface(__uuidof(IUnknown), (void**)&unk1);
    other->QueryInterface(__uuidof(IUnknown), (void**)&unk2);
    return unk1 == unk2;
  }
private:
  T *_p;
};

template<class Base>
class ComObject : public Base
{
public:
  ComObject() : _ref(1) { }
  virtual ~ComObject() { }
  STDMETHOD_(ULONG, AddRef)()
  {
    InterlockedIncrement(&_ref);
    return _ref;
  }
  STDMETHOD_(ULONG, Release)()
  {
    InterlockedDecrement(&_ref);
    if(0 == _ref)
      delete this;
    return _ref;
  }
private:
  ULONG _ref;
};

#define BEGIN_INTERFACE_MAP STDMETHOD(QueryInterface)(REFIID riid, LPVOID *oObject){ \
                              if(__uuidof(IUnknown) == riid) {*oObject = (IUnknown*)this;return S_OK;}
#define INTERFACE_MAP_ENTRY(name) if(__uuidof(name) == riid) {*oObject = (name*)this; return S_OK;}
#define END_INTERFACE_MAP *oObject = NULL; return E_NOINTERFACE; }

#endif // __COM_UTILS_HPP__