#include <pch.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Core.h>
#include <Windows.h>

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
    SpriteVisual m_box{ nullptr };

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
        m_box = nullptr;
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
        PointerEventArgs const&)
    {
        StartAnimation();
    }

    void InitComposition()
    {
        try
        {
            OutputDebugStringW(L"InitComposition: Compositor\n");
            m_compositor = Compositor();

            // UWP では HWND ではなく、現在の CoreWindow view に CompositionTarget を作る
            OutputDebugStringW(L"InitComposition: CreateTargetForCurrentView\n");
            m_target = m_compositor.CreateTargetForCurrentView();

            OutputDebugStringW(L"InitComposition: CreateContainerVisual\n");
            auto root = m_compositor.CreateContainerVisual();
            root.RelativeSizeAdjustment({ 1.0f, 1.0f });

            OutputDebugStringW(L"InitComposition: CreateSpriteVisual\n");
            m_box = m_compositor.CreateSpriteVisual();
            m_box.Size({ 140.0f, 140.0f });
            m_box.Offset({ 40.0f, 80.0f, 0.0f });
            m_box.Brush(
                m_compositor.CreateColorBrush(
                    Color{ 255, 0, 120, 215 }
                )
            );

            OutputDebugStringW(L"InitComposition: Root\n");
            root.Children().InsertAtTop(m_box);
            m_target.Root(root);
            OutputDebugStringW(L"InitComposition: done\n");
        }
        catch (winrt::hresult_error const& e)
        {
            LogFailure(L"InitComposition", e);
            throw;
        }
    }

    void StartAnimation()
    {
        try
        {
            if (!m_box)
            {
                return;
            }

            auto offsetAnimation =
                m_compositor.CreateVector3KeyFrameAnimation();

            offsetAnimation.Duration(std::chrono::milliseconds(900));

            offsetAnimation.InsertKeyFrame(
                0.0f,
                float3{ 40.0f, 80.0f, 0.0f }
            );

            offsetAnimation.InsertKeyFrame(
                0.5f,
                float3{ 360.0f, 80.0f, 0.0f }
            );

            offsetAnimation.InsertKeyFrame(
                1.0f,
                float3{ 40.0f, 80.0f, 0.0f }
            );

            auto opacityAnimation =
                m_compositor.CreateScalarKeyFrameAnimation();

            opacityAnimation.Duration(std::chrono::milliseconds(900));

            opacityAnimation.InsertKeyFrame(0.0f, 1.0f);
            opacityAnimation.InsertKeyFrame(0.5f, 0.25f);
            opacityAnimation.InsertKeyFrame(1.0f, 1.0f);

            m_box.StartAnimation(L"Offset", offsetAnimation);
            m_box.StartAnimation(L"Opacity", opacityAnimation);
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
