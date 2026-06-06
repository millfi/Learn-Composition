#include <pch.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.h>
#include <Windows.h>
//*/
using namespace winrt;

using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Core;

static void LogFailure(wchar_t const* step, winrt::hresult_error const& e)
{
    wchar_t buffer[512];
    swprintf_s(
        buffer,
        L"%s failed. HRESULT=0x%08X Message=%s\n",
        step,
        static_cast<unsigned int>(static_cast<uint32_t>(e.code().value)),
        e.message().c_str()
    );
    OutputDebugStringW(buffer);
}

struct AppView : implements<AppView, IFrameworkView>
{
    CoreWindow m_window{ nullptr };
    Compositor m_compositor{ nullptr };
    CompositionTarget m_target{ nullptr };
    SpriteVisual m_box1{ nullptr };
    SpriteVisual m_box2{ nullptr };

    float3 m_box1Offset{ 40.0f, 80.0f, 0.0f };
    float3 m_box2Offset{ 40.0f, 260.0f, 0.0f };

    bool m_initialized = false;
    bool m_windowClosed = false;

    void Initialize(CoreApplicationView const& applicationView)
    {
        applicationView.Activated(
            { this, &AppView::OnActivated }
        );
    }

    void SetWindow(CoreWindow const& window)
    {
        m_window = window;

        m_window.Closed(
            { this, &AppView::OnClosed }
        );

        m_window.PointerPressed(
            { this, &AppView::OnPointerPressed }
        );
    }

    void Load(hstring const&)
    {
    }

    void Run()
    {
        while (!m_windowClosed)
        {
            m_window.Dispatcher().ProcessEvents(
                CoreProcessEventsOption::ProcessAllIfPresent
            );
        }
    }

    void Uninitialize()
    {
        m_box1 = nullptr;
        m_box2 = nullptr;
        m_target = nullptr;
        m_compositor = nullptr;
        m_window = nullptr;
    }

    void OnActivated(
        CoreApplicationView const&,
        IActivatedEventArgs const&)
    {
        CoreWindow::GetForCurrentThread().Activate();

        if (!m_initialized)
        {
            InitComposition();
            m_initialized = true;
        }
    }

    void OnClosed(
        CoreWindow const&,
        CoreWindowEventArgs const&)
    {
        m_windowClosed = true;
    }

    void OnPointerPressed(
        CoreWindow const&,
        PointerEventArgs const& args)
    {
        auto point = args.CurrentPoint().Position();
        if (HitTest(point, m_box1))
        {
            StartAnimation(m_box1, m_box1Offset);
        }
        else if (HitTest(point, m_box2))
        {
            StartAnimation(m_box2, m_box2Offset);
        }
    }

    bool HitTest(Point const& point, SpriteVisual const& box)
    {
        if (!box)
        {
            return false;
        }
        auto offset = box.Offset();
        auto size = box.Size();
        return point.X >= offset.x && point.X < offset.x + size.x &&
               point.Y >= offset.y && point.Y < offset.y + size.y;
    }

    void InitComposition()
    {
        try
        {
            m_compositor = Compositor();

            // UWP では HWND ではなく、現在の CoreWindow view に CompositionTarget を作る
            m_target = m_compositor.CreateTargetForCurrentView();
            auto root = m_compositor.CreateContainerVisual();
            root.RelativeSizeAdjustment({ 1.0f, 1.0f });
            m_box1 = m_compositor.CreateSpriteVisual();
            m_box1.Size({ 140.0f, 140.0f });
            m_box1.Offset(m_box1Offset);
            m_box1.Brush(
                m_compositor.CreateColorBrush(
                    Color{ 255, 0, 0, 215 }
                )
            );

            m_box2 = m_compositor.CreateSpriteVisual();
            m_box2.Size({ 140.0f, 140.0f });
            m_box2.Offset(m_box2Offset);
            m_box2.Brush(
                m_compositor.CreateColorBrush(
                    Color{ 255, 0, 215, 0 }
                )
            );

            root.Children().InsertAtTop(m_box1);
            root.Children().InsertAtTop(m_box2);
            m_target.Root(root);
        }
        catch (winrt::hresult_error const& e)
        {
            LogFailure(L"InitComposition", e);
            throw;
        }
    }

    void StartAnimation(SpriteVisual const& box, float3 const& baseOffset)
    {
        try
        {
            if (!box)
            {
                return;
            }

            auto offsetAnimation =
                m_compositor.CreateVector3KeyFrameAnimation();

            offsetAnimation.Duration(std::chrono::milliseconds(900));

            offsetAnimation.InsertKeyFrame(
                0.0f,
                baseOffset
            );

            offsetAnimation.InsertKeyFrame(
                0.5f,
                float3{ baseOffset.x + 320.0f, baseOffset.y, 0.0f }
            );

            offsetAnimation.InsertKeyFrame(
                1.0f,
                baseOffset
            );

            auto opacityAnimation =
                m_compositor.CreateScalarKeyFrameAnimation();

            opacityAnimation.Duration(std::chrono::milliseconds(900));

            opacityAnimation.InsertKeyFrame(0.0f, 1.0f);
            opacityAnimation.InsertKeyFrame(0.5f, 0.25f);
            opacityAnimation.InsertKeyFrame(1.0f, 1.0f);

            box.StartAnimation(L"Offset", offsetAnimation);
            box.StartAnimation(L"Opacity", opacityAnimation);
        }
        catch (winrt::hresult_error const& e)
        {
            LogFailure(L"StartAnimation", e);
            throw;
        }
    }
};

struct AppViewSource : implements<AppViewSource, IFrameworkViewSource>
{
    IFrameworkView CreateView()
    {
        return make<AppView>();
    }
};

int __stdcall wWinMain(void*, void*, wchar_t*, int)
{
    CoreApplication::Run(make<AppViewSource>());
    return 0;
}
