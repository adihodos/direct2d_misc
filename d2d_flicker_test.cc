#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <vector>

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define NTDDI_VERSION NTDDI_WIN7

#include <d2d1.h>
#include <d2d1helper.h>
#include <Windows.h>

#ifndef WIDEN_STR
#define WIDEN_STR(str) L#str
#endif

#ifndef __WFILE__
#define __WFILE__ WIDEN_STR(__FILE__)
#endif

namespace {

void 
OutputFormattedDebugString(
  const wchar_t* file,
  int line,
  const wchar_t* fmt,
  ...
  )
{
  wchar_t buff_msg[2048];
  _snwprintf_s(buff_msg, _countof(buff_msg) - 1, L"\n[%s, %d]\n", file, line);
  ::OutputDebugStringW(buff_msg);

  va_list args_ptr;
  va_start(args_ptr, fmt);
  _vsnwprintf_s(buff_msg, _countof(buff_msg) - 1, fmt, args_ptr);
  va_end(args_ptr);

  ::OutputDebugStringW(buff_msg);
}

#ifndef ERR_WINAPI
#define ERR_WINAPI(func)  \
  do {                    \
    OutputFormattedDebugString(__WFILE__, __LINE__, \
                               L"Function %s failed, error %d", \
                               WIDEN_STR(func), ::GetLastError()); \
  } while (0)
#endif

#ifndef TRACE_D2DCALL
#define TRACE_D2DCALL(ret_code_ptr, call_and_args)  \
  do {                                              \
    *(ret_code_ptr) = (call_and_args);              \
    if (FAILED(*(ret_code_ptr))) {                  \
      OutputFormattedDebugString(__WFILE__, __LINE__,\
        L"Call %s failed, HRESULT %#08x",            \
        WIDEN_STR(call_and_args),                    \
        *(ret_code_ptr));                            \
    }                                                \
  } while (0)
#endif

struct COM_Deleter {
  void operator()(IUnknown* ptr) {
    if (ptr)
      ptr->Release();
  }
};

inline
bool
PointInRectangle(
  const D2D1_POINT_2F& pt,
  const D2D1_RECT_F& rect
  )
{
  return pt.x >= rect.left && pt.x <= rect.right &&
    pt.y >= rect.top && pt.y <= rect.bottom;
}

class MovingRectangle {
public :
  MovingRectangle() {}

  ~MovingRectangle() {}

  void SetVelocity(const D2D1_POINT_2F& v) {
    velocity_ = v;
  }

  void SetPosition(const D2D1_POINT_2F& pos) {
    pos_ = pos;
  }

  void SetGeometry(const D2D1_SIZE_F& geometry) {
    geometry_ = geometry;
  }

  void SetBrush(ID2D1SolidColorBrush* brush) {
    brush_ = brush;
  }

  void Draw(ID2D1RenderTarget* target) {
    assert(brush_);
    target->FillRectangle(D2D1::RectF(pos_.x - geometry_.width / 2, 
                                      pos_.y - geometry_.height / 2,
                                      pos_.x + geometry_.width / 2,
                                      pos_.y + geometry_.height / 2),
                          brush_);
  }

  void Move(float direction) {
    D2D1_POINT_2F new_pos(D2D1::Point2F(
      pos_.x + direction * (velocity_.x + geometry_.width / 2), pos_.y));
    if (PointInRectangle(new_pos, D2D1::RectF(0.0f, 0.0f, 1280, 1024)))
      pos_.x += velocity_.x * direction;
  }
  
private :
  D2D1_POINT_2F           pos_;
  D2D1_POINT_2F           velocity_;
  D2D1_SIZE_F             geometry_;
  ID2D1SolidColorBrush*   brush_;
};

class Direct2DWindow {
public :
  Direct2DWindow() : app_window_(nullptr) {}

  ~Direct2DWindow() {}

  static bool RegisterWindowClass(HINSTANCE);

  static void PumpMessagesUntilQuit();

  bool Create(int width, int height);

private :
  static const wchar_t* const C_WindowClassName;
  static Direct2DWindow*      mainwindow_;
  static HINSTANCE            app_instance_;

  static LRESULT CALLBACK WindowProcStub(HWND, UINT, WPARAM, LPARAM);

  LRESULT WindowProcedureHandler(UINT, WPARAM, LPARAM);

  void RenderFrame();

  bool CreateDeviceIndependentResources();

  bool CreateDeviceDependentResources();

  void DiscardResources() {
    brush_cache_.clear();
    rendertarget_.reset();
    bitmaptarget_.reset();
  }

  void Handle_KeyDown(UINT code) {
    switch (code) {
    case VK_LEFT :
      block_.Move(0.5f);
      break;

    case VK_RIGHT :
      block_.Move(-0.5f);
      break;

    case VK_ESCAPE :
      if (::MessageBoxW(app_window_, L"Quit app ?", L"", 
                        MB_ICONQUESTION | MB_YESNO) == IDYES)
                        ::DestroyWindow(app_window_);
      break;

    default :
      break;
    }
  }

  enum Brushes {
    Brush_Black,
    Brush_White,
    Brush_Orange
  };

  HWND                                        app_window_;
  int                                         width_;
  int                                         height_;
  std::shared_ptr<ID2D1Factory>               factory_;
  std::shared_ptr<ID2D1HwndRenderTarget>      rendertarget_;
  std::shared_ptr<ID2D1BitmapRenderTarget>    bitmaptarget_;
  std::vector<std::shared_ptr<ID2D1SolidColorBrush>> brush_cache_;
  MovingRectangle                             block_;
};

const wchar_t* const Direct2DWindow::C_WindowClassName = L"Direct2DWindowClass@@##";

Direct2DWindow* Direct2DWindow::mainwindow_;

HINSTANCE Direct2DWindow::app_instance_;

bool
Direct2DWindow::RegisterWindowClass(
  HINSTANCE inst
  )
{
  WNDCLASSEXW cls_info = {
    sizeof(cls_info),
    CS_HREDRAW | CS_VREDRAW,
    Direct2DWindow::WindowProcStub,
    0, 0, inst,
    ::LoadIconW(nullptr, IDI_APPLICATION),
    ::LoadCursorW(nullptr, IDC_ARROW),
    reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
    nullptr,
    Direct2DWindow::C_WindowClassName,
    nullptr
  };

  if (!::RegisterClassExW(&cls_info)) {
    ERR_WINAPI(RegisterClassExW);
    return false;
  }

  Direct2DWindow::app_instance_ = inst;
  return true;
}

void
Direct2DWindow::PumpMessagesUntilQuit() {
  MSG msg_info;
  for (;;) {
    if (::PeekMessageW(&msg_info, nullptr, 0, 0, PM_REMOVE)) {
      if (msg_info.message == WM_QUIT)
        break;

      ::TranslateMessage(&msg_info);
      ::DispatchMessageW(&msg_info);
    }

    Direct2DWindow::mainwindow_->RenderFrame();
  }
}

LRESULT
CALLBACK
Direct2DWindow::WindowProcStub(
  HWND wnd,
  UINT msg,
  WPARAM wparam,
  LPARAM lparam
  )
{
  if (msg == WM_NCCREATE) {
    ::SetWindowLongPtrW(
        wnd, GWLP_USERDATA, 
        reinterpret_cast<LONG_PTR>(
          reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams));
    return true;
  }

  Direct2DWindow* wptr(reinterpret_cast<Direct2DWindow*>(
    ::GetWindowLongPtrW(wnd, GWLP_USERDATA)));
  return wptr ? wptr->WindowProcedureHandler(msg, wparam, lparam) :
    ::DefWindowProcW(wnd, msg, wparam, lparam);
}

bool
Direct2DWindow::Create(
  int width,
  int heigth
  )
{
  assert(!app_window_);
  assert(Direct2DWindow::app_instance_);

  width_ = width;
  height_ = heigth;

  RECT window_geometry = { 0, 0, width, heigth };
  DWORD extended_style = WS_EX_APPWINDOW;
  DWORD style = WS_POPUP;
  ::AdjustWindowRectEx(&window_geometry, style, false, extended_style);

  app_window_ = ::CreateWindowExW(extended_style,
                                  Direct2DWindow::C_WindowClassName,
                                  L"DrawTestWindow",
                                  style,
                                  window_geometry.left,
                                  window_geometry.top,
                                  window_geometry.right,
                                  window_geometry.bottom,
                                  nullptr, nullptr,
                                  Direct2DWindow::app_instance_,
                                  this);
  if (!app_window_) {
    ERR_WINAPI(CreateWindowEx);
    return false;
  }

  Direct2DWindow::mainwindow_ = this;

  ::ShowWindow(app_window_, SW_SHOWNORMAL);
  ::UpdateWindow(app_window_);
  return CreateDeviceIndependentResources();
}

LRESULT
Direct2DWindow::WindowProcedureHandler(
  UINT msg,
  WPARAM wparam,
  LPARAM lparam
  )
{
  switch (msg) {
  case WM_CLOSE :
    ::DestroyWindow(app_window_);
    return 0L;
    break;

  case WM_DESTROY :
    ::PostQuitMessage(0);
    return 0L;
    break;

  case WM_KEYDOWN :
    Handle_KeyDown(wparam);
    return 0L;
    break;

  default :
    break;
  }

  return ::DefWindowProcW(app_window_, msg, wparam, lparam);
}

void
Direct2DWindow::RenderFrame() {
  CreateDeviceDependentResources();
  rendertarget_->BeginDraw();
  rendertarget_->Clear(D2D1::ColorF(D2D1::ColorF::White));
  block_.Draw(rendertarget_.get());
  if (rendertarget_->EndDraw() == D2DERR_RECREATE_TARGET) {
    DiscardResources();
  }
}

bool
Direct2DWindow::CreateDeviceIndependentResources() {
  assert(!factory_);

  ID2D1Factory* tmp;
  HRESULT ret_code;
  TRACE_D2DCALL(&ret_code,
                ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &tmp));
  if (FAILED(ret_code))
    return false;

  factory_.reset(tmp, COM_Deleter());
  return true;
}

bool
Direct2DWindow::CreateDeviceDependentResources() {
  if (rendertarget_.get())
    return true;
        
  D2D1_RENDER_TARGET_PROPERTIES rtarget_props;
  rtarget_props.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;
  rtarget_props.pixelFormat = ::D2D1::PixelFormat(
    DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
  rtarget_props.dpiX = rtarget_props.dpiY = 0.0f;
  rtarget_props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
  rtarget_props.minLevel = D2D1_FEATURE_LEVEL_10;

  D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_rprops;
  hwnd_rprops.hwnd = app_window_;
  hwnd_rprops.pixelSize = ::D2D1::SizeU(width_, height_);
  hwnd_rprops.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

  HRESULT ret_code;
  ID2D1HwndRenderTarget* ptarget;
  TRACE_D2DCALL(&ret_code, factory_->CreateHwndRenderTarget(rtarget_props, 
                                                            hwnd_rprops,
                                                            &ptarget));
  if (FAILED(ret_code))
    return false;

  rendertarget_.reset(ptarget, COM_Deleter());

  D2D1_COLOR_F colors[] = {
    D2D1::ColorF(D2D1::ColorF::Black),
    D2D1::ColorF(D2D1::ColorF::White),
    D2D1::ColorF(D2D1::ColorF::Orange)
  };

  std::transform(
    std::begin(colors),
    std::end(colors),
    std::back_inserter(brush_cache_),
    [ptarget](const D2D1_COLOR_F& colour) -> std::shared_ptr<ID2D1SolidColorBrush> {
      ID2D1SolidColorBrush* brush = nullptr;
      HRESULT ret_code;
      TRACE_D2DCALL(&ret_code, ptarget->CreateSolidColorBrush(colour, &brush));
      return std::shared_ptr<ID2D1SolidColorBrush>(brush, COM_Deleter());
  });

  block_.SetBrush(brush_cache_[Direct2DWindow::Brush_Orange].get());
  block_.SetPosition(D2D1::Point2F(width_ / 2, height_ / 2));
  block_.SetGeometry(D2D1::SizeF(200.0f, 200.0f));
  block_.SetVelocity(D2D1::Point2F(10.0f, 0.0f));
  return true;
}

}

int 
WINAPI 
wWinMain(
  HINSTANCE instance,
  HINSTANCE,
  LPWSTR,
  int
  )
{
  Direct2DWindow app_window;
  if (Direct2DWindow::RegisterWindowClass(instance) && 
      app_window.Create(1280, 1024))
      Direct2DWindow::PumpMessagesUntilQuit();

  return 0;
}