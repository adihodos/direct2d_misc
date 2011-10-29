#include "pch_hdr.h"
#include "vector2.h"

template<typename D2D1Interface>
struct D2D1_Obj_Deleter {
    void operator()(D2D1Interface* iptr) {
        if (iptr)
            iptr->Release();
    }
};

class Fighter_Mig21 {
public :
    Fighter_Mig21(ID2D1PathGeometry* geometry, ID2D1SolidColorBrush* brush) 
        : geometry_(geometry, D2D1_Obj_Deleter<ID2D1Geometry>()),
          fbrush_(brush, D2D1_Obj_Deleter<ID2D1SolidColorBrush>()) {}

   ID2D1PathGeometry* GetGeometry() const {
       return geometry_.get();
   }

   ID2D1SolidColorBrush* GetBrush() const {
       return fbrush_.get();
   }
   
   void BuildFighterGeometry() {
        ID2D1GeometrySink* gsink;
        HRESULT ret_code = geometry_->Open(&gsink);
        if (FAILED(ret_code))
            return;

        std::shared_ptr<ID2D1GeometrySink> skptr(
            gsink, D2D1_Obj_Deleter<ID2D1GeometrySink>());
        skptr->BeginFigure(D2D1::Point2F(-5.0f, 0.0f), D2D1_FIGURE_BEGIN_FILLED);
        skptr->AddLine(D2D1::Point2F(-5.0f, 1.0f));
        skptr->AddBezier(
            D2D1::BezierSegment(D2D1::Point2F(-4.5f, 2.0f), 
                                D2D1::Point2F(-3.5f, 3.5f), 
                                D2D1::Point2F(-1.0f, 5.0f)));
        skptr->AddArc(D2D1::ArcSegment(
            D2D1::Point2F(1.0f, 5.0f), D2D1::SizeF(1.0f, 4.0f), 0.0f, 
            D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_LARGE));

        skptr->AddBezier(
            D2D1::BezierSegment(D2D1::Point2F(2.0f, 4.5f), 
            D2D1::Point2F(3.5f, 3.5f), 
            D2D1::Point2F(5.0f, 1.0f)));

        skptr->AddLine(D2D1::Point2F(5.0f, 0.0f));
        skptr->AddLine(D2D1::Point2F(0.50f, -1.0f));

        skptr->AddBezier(
            D2D1::BezierSegment(D2D1::Point2F(0.5f, -1.0f), 
                                D2D1::Point2F(0.0f, -4.0f), 
                                D2D1::Point2F(-0.5f, -1.0f)));

        skptr->AddLine(D2D1::Point2F(-5.0f, 0.0f));
        skptr->EndFigure(D2D1_FIGURE_END_CLOSED);
        skptr->Close();
    }

   void SetBrush(ID2D1SolidColorBrush* brsh) {
       fbrush_.reset(brsh);
   }
private :
    std::shared_ptr<ID2D1PathGeometry>      geometry_;
    std::shared_ptr<ID2D1SolidColorBrush>   fbrush_;
};

class W32Window {
public :

    W32Window(int width, int height)
        : wnd_(0), width_(width), height_(height) {}

    ~W32Window() {
        ::ClipCursor(nullptr);
    }

    static void Set_Instance(HINSTANCE inst) {
        W32Window::inst_ = inst;
    }

    static HINSTANCE Get_Instance() {
        return W32Window::inst_;
    }

    static bool RegisterWndClass() {
        WNDCLASSW cls_info;
        cls_info.cbClsExtra = cls_info.cbWndExtra = 0;
        cls_info.hbrBackground = static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));
        cls_info.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
        cls_info.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
        cls_info.hInstance = W32Window::inst_;
        cls_info.lpfnWndProc = W32Window::WindowProcedureStub;
        cls_info.lpszClassName = W32Window::Class_Name;
        cls_info.lpszMenuName = 0;
        cls_info.style = CS_HREDRAW | CS_VREDRAW;

        return ::RegisterClassW(&cls_info) != 0;
    }

    static void PumpMessagesUntilDone() {
        assert(W32Window::instance_ptr_);
        MSG msg_data;

        for (;;) {
            if (::PeekMessageW(&msg_data, nullptr, 0, 0, PM_REMOVE)) {
                if (msg_data.message == WM_QUIT)
                    break;

                ::TranslateMessage(&msg_data);
                ::DispatchMessageW(&msg_data);
            }

            instance_ptr_->DrawFrame();
        }
    }

    void DrawFrame() {
        rtarget_->BeginDraw();
        rtarget_->Clear(D2D1::ColorF(D2D1::ColorF::White));
        rtarget_->SetTransform(D2D1::Matrix3x2F::Identity());
        rtarget_->FillRectangle(D2D1::RectF(0.0f, 0.0f, width_, height_), brushes_[Brush_DeepSkyBlue].get());
        rtarget_->DrawLine(
            D2D1::Point2F(width_ / 2, 0.0f),
            D2D1::Point2F(width_ / 2, height_),
            brushes_[Brush_Black].get(),
            1.0f);
        rtarget_->DrawLine(
            D2D1::Point2F(0.0f, height_ / 2),
            D2D1::Point2F(width_, height_ / 2),
            brushes_[Brush_Black].get(),
            1.0f);

        rtarget_->SetTransform(
            D2D1::Matrix3x2F::Rotation(180.0f) * 
            D2D1::Matrix3x2F::Scale(25.0f, 25.0f) * 
            D2D1::Matrix3x2F::Translation(world_origin_)
            );
        rtarget_->FillGeometry(fmig21_->GetGeometry(), fmig21_->GetBrush());
        if (rtarget_->EndDraw() == D2DERR_RECREATE_TARGET)
            DiscardResources();
    }

    bool Create() {
        assert(!wnd_);
        assert(!W32Window::instance_ptr_);

        instance_ptr_ = this;

        DWORD style = WS_POPUP;
        RECT wnd_geometry;
        wnd_geometry.left = wnd_geometry.top = 0;
        wnd_geometry.right = width_;
        wnd_geometry.bottom = height_;

        ::AdjustWindowRect(&wnd_geometry, style, false);
        wnd_ = ::CreateWindow(
            W32Window::Class_Name, L"D2D1 Window", style, 0, 0,
            wnd_geometry.right, wnd_geometry.bottom, 0, 0, inst_, this);

        if (!wnd_)
            return false;

        world_origin_.x_ = wnd_geometry.right / 2;
        world_origin_.y_ = wnd_geometry.bottom / 2;
        ::ShowWindow(wnd_, SW_SHOWNORMAL);
        ::UpdateWindow(wnd_);
        //::ClipCursor(&wnd_geometry);
        
        return InitializeRenderer();
    }
    
private :
    static LRESULT WINAPI WindowProcedureStub(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        if (msg == WM_NCCREATE) {
            ::SetWindowLongPtrW(
                wnd, GWLP_USERDATA, 
                reinterpret_cast<LONG_PTR>(
                    reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams));
            return true;
        }

        W32Window* wptr(reinterpret_cast<W32Window*>(::GetWindowLongPtrW(wnd, GWLP_USERDATA)));
        if (wptr && wptr->WindowProcedure(msg, wparam, lparam))
            return 0L;

        return ::DefWindowProcW(wnd, msg, wparam, lparam);
    }

    bool InitializeRenderer() {
        ID2D1Factory* factory;
        HRESULT ret_code = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
        if (FAILED(ret_code))
            return false;

        factory_.reset(factory, D2D1_Obj_Deleter<ID2D1Factory>());
        return CreateDeviceDependentResources();
    }

    bool CreateDeviceDependentResources() {
        if (rtarget_.get())
            return true;

        ID2D1HwndRenderTarget* ptarget;
        
        D2D1_RENDER_TARGET_PROPERTIES rtarget_props;
        rtarget_props.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;
        rtarget_props.pixelFormat = ::D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
        rtarget_props.dpiX = rtarget_props.dpiY = 0.0f;
        rtarget_props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
        rtarget_props.minLevel = D2D1_FEATURE_LEVEL_10;

        D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_rprops;
        hwnd_rprops.hwnd = wnd_;
        hwnd_rprops.pixelSize = ::D2D1::SizeU(width_, height_);
        hwnd_rprops.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

        HRESULT ret_code = factory_->CreateHwndRenderTarget(
            rtarget_props, hwnd_rprops, &ptarget);
        if (FAILED(ret_code))
            return false;

        rtarget_.reset(ptarget, D2D1_Obj_Deleter<ID2D1HwndRenderTarget>());

        D2D1_COLOR_F brush_colors[] = {
            D2D1::ColorF(D2D1::ColorF::DeepSkyBlue),
            D2D1::ColorF(D2D1::ColorF::Black),
            D2D1::ColorF(D2D1::ColorF::Crimson)
        };

        std::transform(
            std::begin(brush_colors), std::end(brush_colors),
            std::back_inserter(brushes_), 
            [ptarget](const D2D1_COLOR_F& brush_color) -> std::shared_ptr<ID2D1SolidColorBrush> {
                ID2D1SolidColorBrush* pbrush = nullptr;
                HRESULT ret = ptarget->CreateSolidColorBrush(brush_color, &pbrush); 
                return std::shared_ptr<ID2D1SolidColorBrush>(pbrush, D2D1_Obj_Deleter<ID2D1SolidColorBrush>());
        });

        return InitializeObjects();
    }

    void DiscardResources() {
        rtarget_.reset();
        brushes_.clear();
    }

    bool InitializeObjects() {
        ID2D1PathGeometry* geometry;
        HRESULT ret_code = factory_->CreatePathGeometry(&geometry);
        if (FAILED(ret_code))
            return false;

        ID2D1SolidColorBrush* fbrush;
        ret_code = rtarget_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LawnGreen), &fbrush);
        if (FAILED(ret_code))
            return false;

        fmig21_.reset(new Fighter_Mig21(geometry, fbrush));
        fmig21_->BuildFighterGeometry();
        return true;
    }

    bool WindowProcedure(UINT msg, WPARAM wparam, LPARAM lparam) {
        switch (msg) {
        case WM_KEYDOWN :
            ::DestroyWindow(wnd_);
            return true;
            break;

        case WM_DESTROY :
            ::PostQuitMessage(0);
            return true;
            break;

        default :
            break;
        }

        return false;
    }

    enum {
        Brush_DeepSkyBlue,
        Brush_Black,
        Brush_Red
    };

    static HINSTANCE    inst_;
    static W32Window*   instance_ptr_;
    static const wchar_t* Class_Name;

    HWND        wnd_;
    int         width_;
    int         height_;
    std::shared_ptr<ID2D1Factory>           factory_;
    std::shared_ptr<ID2D1HwndRenderTarget>  rtarget_;
    std::vector<std::shared_ptr<ID2D1SolidColorBrush>> brushes_;
    gfx::vector2    world_origin_;
    std::shared_ptr<Fighter_Mig21>  fmig21_;
};

const wchar_t* W32Window::Class_Name = L"D2D1_Window_Class";

HINSTANCE W32Window::inst_;

W32Window* W32Window::instance_ptr_;

bool GetScreenResolution(std::pair<DWORD, DWORD>* res) {
    DEVMODEW dev_settings;
    if (::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dev_settings)) {
        res->first = dev_settings.dmPelsWidth;
        res->second = dev_settings.dmPelsHeight;
        return true;
    }
    return false;
}

int WINAPI wWinMain( 
    __in HINSTANCE hinst, 
    __in_opt HINSTANCE, 
    __in LPWSTR, 
    __in int
    )
{
    W32Window::Set_Instance(hinst);

    std::pair<DWORD, DWORD> screen_res;
    GetScreenResolution(&screen_res);
    W32Window main_wnd(
        static_cast<int>(screen_res.first), static_cast<int>(screen_res.second));

    if (!W32Window::RegisterWndClass())
        return -1;

    if (main_wnd.Create())
        W32Window::PumpMessagesUntilDone();

    return 0;
}