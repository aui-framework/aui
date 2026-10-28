/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dlfcn.h>
#include "PlatformAbstractionGtk.h"
#include "RenderingContextGtk.h"
#include "gtk_functions.h"
#include <AUI/AppInfo.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/ASurface.h>
#include <AUI/Platform/AStacktrace.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/ACleanup.h>
#include <AUI/Util/kAUI.h>
#include "GtkX11Interop.h"
#include <algorithm>
#include <cmath>
#include <mutex>
#include <unordered_set>

// hack: we sort of implementing g_application_run here.
// particularly:
// - command line parsing is already done by AUI_ENTRY
// - acquiring GMainContext in constructor
// - registering the application
// - windowManagerLoop handles application loop
// - releasing acquired context in destructor
// See https://github.com/aui-framework/aui/issues/468

namespace aui::gtk4_fake {
extern void* handle;
}

using namespace aui::gtk4_fake;


namespace {

/**
 * Wayland deliberately does not let clients do some things (positioning windows, warping pointer, etc). Report such
 * attempts with a stacktrace (once per feature) so the caller can be fixed or made aware.
 */
void unsupportedOnWayland(const char* feature) {
    static std::mutex mutex;
    static std::unordered_set<std::string> reported;
    {
        std::lock_guard lock(mutex);
        if (!reported.insert(feature).second) {
            return;
        }
    }
    ALogger::err("PlatformAbstractionGtk")
        << feature << " is not supported by the current display server (Wayland) and is ignored\n"
        << AStacktrace::capture(1);
}

GdkSurface* surfaceOf(GtkWindow* window) {
    if (window == nullptr) {
        return nullptr;
    }
    // surface exists only after the window is realized; realizing does not show the window.
    gtk_widget_realize(GTK_WIDGET(window));
    auto native = gtk_widget_get_native(GTK_WIDGET(window));
    return native ? gtk_native_get_surface(native) : nullptr;
}

/**
 * @brief Surface which scale applies to the window. Popups take the scale from their parent until they get their own
 * surface.
 */
GdkSurface* scaleSurfaceOf(AWindow& window) {
    auto widget = PlatformAbstractionGtk::gtkWidget(window);
    if (auto native = gtk_widget_get_native(widget)) {
        if (auto surface = gtk_native_get_surface(native)) {
            return surface;
        }
    }
    if (auto parent = PlatformAbstractionGtk::parentWindow(window)) {
        return scaleSurfaceOf(*parent);
    }
    return nullptr;
}

GdkDisplay* defaultDisplay() { return reinterpret_cast<GdkDisplay*>(gdk_display_get_default()); }

class CursorGtk : public ACursor::Custom {
public:
    explicit CursorGtk(AGlibPtr<GdkCursor> cursor) : mCursor(std::move(cursor)) {}
    [[nodiscard]] GdkCursor* handle() const noexcept { return mCursor.value(); }

private:
    AGlibPtr<GdkCursor> mCursor;
};

AGlibPtr<GdkCursor> createCursorFromImage(AImageView img, glm::ivec2 hotspot) {
    std::vector<uint8_t> pixels(size_t(img.width()) * img.height() * 4);
    static constexpr auto FORMAT = APixelFormat::RGBA | APixelFormat::BYTE;
    AFormattedImageView<FORMAT> destination(
        AByteBufferView(reinterpret_cast<const char*>(pixels.data()), pixels.size()), img.size());
    img.visit([&](const auto& source) {
        using source_image_t = std::decay_t<decltype(source)>;
        using destination_image_t = std::decay_t<decltype(destination)>;

        static constexpr auto sourceFormat = (APixelFormat::Value) source_image_t::FORMAT;
        static constexpr auto destinationFormat = (APixelFormat::Value) destination_image_t::FORMAT;

        std::transform(
            source.begin(), source.end(), const_cast<destination_image_t::Color*>(destination.begin()),
            aui::pixel_format::convert<sourceFormat, destinationFormat>);
    });
    auto bytes = g_bytes_new(pixels.data(), pixels.size());
    auto texture = AGlibPtr<GdkTexture>(
        gdk_memory_texture_new(img.width(), img.height(), GDK_MEMORY_R8G8B8A8, bytes, size_t(img.width()) * 4));
    g_bytes_unref(bytes);
    return AGlibPtr<GdkCursor>(gdk_cursor_new_from_texture(texture.value(), hotspot.x, hotspot.y, nullptr));
}

AGlibPtr<GdkCursor> createSystemCursor(ACursor::System s) {
    // https://www.w3.org/TR/css-ui-4/#cursor
    const char* name = "default";
    switch (s) {
        case ACursor::POINTER: name = "pointer"; break;
        case ACursor::TEXT: name = "text"; break;
        case ACursor::EW_RESIZE: name = "ew-resize"; break;
        case ACursor::NS_RESIZE: name = "ns-resize"; break;
        case ACursor::NWSE_RESIZE: name = "nwse-resize"; break;
        case ACursor::NESW_RESIZE: name = "nesw-resize"; break;
        case ACursor::MOVE: name = "move"; break;
        default: break;
    }
    return AGlibPtr<GdkCursor>(gdk_cursor_new_from_name(name, nullptr));
}

void setCursor(AWindow* window, GdkCursor* cursor) {
    if (auto handle = PlatformAbstractionGtk::nativeHandle(*window)) {
        gtk_widget_set_cursor(GTK_WIDGET(handle), cursor);
    }
}

}   // namespace

GtkWindow* PlatformAbstractionGtk::nativeWindow(AWindow& window) {
    return isPopup(window) ? nullptr : nativeHandle(window);
}

bool PlatformAbstractionGtk::isPopup(AWindow& window) {
    auto widget = gtkWidget(window);
    return widget != nullptr && g_object_get_data(G_OBJECT(widget), POPUP_KEY) != nullptr;
}

PlatformAbstractionGtk::PlatformAbstractionGtk() : mMainContext(g_main_context_default()) {
    handle = dlopen("libgtk-4.so.1", RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        throw AException("failed to load libgtk-4.so.1: {}"_format(dlerror()));
    }
    gtk_init();

    if (auto display = gdk_display_get_default()) {
        // vulkan? no thanks
        g_object_set_data_full(G_OBJECT(display), "gsk-renderer", g_strdup("ngl"), g_free);
    }
}

void PlatformAbstractionGtk::init() {
    if (!mApplication) {
        // Use app_id from AppInfo as the application identifier for GTK
        // This will be used as the WM_CLASS on X11 and app-id on Wayland
        mApplication = G_APPLICATION(gtk_application_new(
            aui::app_info::app_id.toStdString().c_str(),
            // single instance is handled by AApplication::requestSingleInstanceLock (which is GApplication itself
            // when the session bus is available); this one must not compete for the same bus name.
            G_APPLICATION_NON_UNIQUE));
    }
    g_signal_connect(
        mApplication, "activate",
        [] {
          /* just a stub to silence warning */
        },
        nullptr);

    if (!g_main_context_acquire(mMainContext)) {
        throw AException("failed to acquire GMainContext");
    }

    if (GError* error = nullptr; !g_application_register(mApplication, nullptr, &error)) {
        AUI_DEFER { g_error_free(error); };
        throw AException("failed to register GApplication: {}"_format(error->message));
    }
    
    // Monitor gtk-xft-dpi changes
    setupDpiChangeMonitoring();
    
    // maybe we might want to delegate command line handling to GApplication
    while (g_main_context_iteration (mMainContext, false));
}

PlatformAbstractionGtk::~PlatformAbstractionGtk() {
    while (g_main_context_iteration (mMainContext, false));
    mApplication = nullptr;
    while (g_main_context_iteration (mMainContext, false));
    g_main_context_release(mMainContext);
}

_<ACursor::Custom> PlatformAbstractionGtk::createCustomCursor(AImageView image, glm::ivec2 hotspot) {
    return _new<CursorGtk>(createCursorFromImage(image, hotspot));
}

void PlatformAbstractionGtk::applyNativeCursor(const ACursor &cursor, AWindow *pWindow) {
    std::visit(
        aui::lambda_overloaded {
          [&](ACursor::System s) { setCursor(pWindow, createSystemCursor(s).value()); },
          [&](const _<ACursor::Custom>& custom) {
              if (auto impl = _cast<CursorGtk>(custom)) {
                  setCursor(pWindow, impl->handle());
              }
          },
          [&](const _<IDrawable>& drawable) {
              static AMap<_<IDrawable>, AMap<std::tuple<int, int, float, float>, _<CursorGtk>>> cache;
              AUI_DO_ONCE {
                  ACleanup::afterEntry([&] { cache.clear(); });
              }
              auto custom = cache[drawable].getOrInsert(
                  { int(pWindow->getDpiRatio() * 10), cursor.size(), cursor.hotspot().x, cursor.hotspot().y }, [&] {
                      auto image = drawable->rasterize(glm::ivec2(cursor.size() * pWindow->getDpiRatio()));
                      return _new<CursorGtk>(
                          createCursorFromImage(image, ACursor::hotspotInPixels(cursor.hotspot(), image.size())));
                  });
              setCursor(pWindow, custom->handle());
          },
        },
        cursor.value());
}

void PlatformAbstractionGtk::setClipboardText(const AString &text) {
    if (auto display = defaultDisplay()) {
        gdk_clipboard_set_text(gdk_display_get_clipboard(display), text.toStdString().c_str());
    }
}

AString PlatformAbstractionGtk::getClipboardText() {
    auto display = defaultDisplay();
    if (!display) {
        return {};
    }
    // gdk clipboard is asynchronous only; AUI expects a synchronous result. Spin the main loop until it's done.
    struct State {
        bool done = false;
        bool abandoned = false;
        std::string text;
    };
    auto state = new State;
    auto clipboard = gdk_display_get_clipboard(display);
    gdk_clipboard_read_text_async(
        clipboard, nullptr,
        +[](GObject* source, GAsyncResult* res, gpointer userData) {
            auto state = static_cast<State*>(userData);
            if (char* text = gdk_clipboard_read_text_finish(reinterpret_cast<GdkClipboard*>(source), res, nullptr)) {
                state->text = text;
                g_free(text);
            }
            if (state->abandoned) {
                delete state;
                return;
            }
            state->done = true;
        },
        state);

    // the clipboard owner might be unresponsive.
    struct Timeout { bool expired = false; } timeout;
    auto timeoutSource = g_timeout_add(
        2000, +[](gpointer data) -> gboolean { static_cast<Timeout*>(data)->expired = true; return false; }, &timeout);
    while (!state->done && !timeout.expired) {
        g_main_context_iteration(mMainContext, true);
    }
    if (!timeout.expired) {
        g_source_remove(timeoutSource);
    }
    if (!state->done) {
        state->abandoned = true; // callback frees the state
        ALogger::warn("PlatformAbstractionGtk") << "Timed out waiting for clipboard contents";
        return {};
    }
    AString result = AString::fromUtf8(state->text);
    delete state;
    return result;
}

glm::ivec2 PlatformAbstractionGtk::desktopGetMousePosition() {
    if (auto p = aui::gtk4_x11::pointerPosition()) {
        return *p;
    }
    unsupportedOnWayland("desktopGetMousePosition");
    return glm::ivec2(0);
}

void PlatformAbstractionGtk::desktopSetMousePosition(glm::ivec2 pos) {
    if (!aui::gtk4_x11::warpPointer(pos)) {
        unsupportedOnWayland("desktopSetMousePosition");
    }
}
void PlatformAbstractionGtk::windowSetTitle(AWindow &window, const AString& title) {
    if (auto handle = nativeWindow(window)) {
        gtk_window_set_title(handle, title.toStdString().c_str());
    }
}

void PlatformAbstractionGtk::windowSetResizable(AWindow &window, bool resizable) {
    if (auto handle = nativeWindow(window)) {
        gtk_window_set_resizable(handle, resizable);
    }
}

bool PlatformAbstractionGtk::windowIsResizable(AWindow &window) const {
    if (auto handle = nativeWindow(window)) {
        return gtk_window_get_resizable(handle);
    }
    return false;
}

GtkWidget* PlatformAbstractionGtk::gtkTitlebar(GtkWindow* window) {
    return static_cast<GtkWidget*>(g_object_get_data(G_OBJECT(window), TITLEBAR_KEY));
}

void PlatformAbstractionGtk::gtkSetTitlebar(GtkWindow* window, GtkWidget* titlebar) {
    g_object_set_data(G_OBJECT(window), TITLEBAR_KEY, titlebar);
}

float PlatformAbstractionGtk::windowFetchDpiFromSystem(AWindow &window) {
    auto surface = scaleSurfaceOf(window);
    if (surface == nullptr) {
        return 1.0f;
    }
    return float(gdk_surface_get_scale(surface));
}
void PlatformAbstractionGtk::windowRestore(AWindow &window) {
    if (auto handle = nativeWindow(window)) {
        gtk_window_unminimize(handle);
        gtk_window_unmaximize(handle);
    }
}
void PlatformAbstractionGtk::windowMinimize(AWindow &window) {
    if (auto handle = nativeWindow(window)) {
        gtk_window_minimize(handle);
    }
}
bool PlatformAbstractionGtk::windowIsMinimized(AWindow &window) const {
    if (isPopup(window)) {
        return false;
    }
    if (!aui::gtk4_x11::isActive()) {
        // wayland does not tell clients whether they're minimized
        unsupportedOnWayland("windowIsMinimized");
        return false;
    }
    auto surface = surfaceOf(nativeHandle(window));
    if (!surface) {
        return false;
    }
    unsigned state = 0;
    g_object_get(surface, "state", &state, nullptr);
    return bool(state & GDK_TOPLEVEL_STATE_MINIMIZED);
}
bool PlatformAbstractionGtk::windowIsMaximized(AWindow &window) const {
    auto handle = nativeWindow(window);
    return handle && gtk_window_is_maximized(handle);
}
void PlatformAbstractionGtk::windowMaximize(AWindow &window) {
    if (auto handle = nativeWindow(window)) {
        gtk_window_maximize(handle);
    }
}
glm::ivec2 PlatformAbstractionGtk::windowGetPosition(AWindow &window) const {
    if (isPopup(window)) {
        // popups are positioned relative to their parent
        return glm::ivec2(0);
    }
    if (auto p = aui::gtk4_x11::windowPosition(surfaceOf(nativeHandle(window)))) {
        return *p;
    }
    unsupportedOnWayland("windowGetPosition");
    return glm::ivec2(0);
}
void PlatformAbstractionGtk::windowFlagRedraw(AWindow &window) {
    window.getThread()->enqueue([&window] {
      if (auto ctx = dynamic_cast<RenderingContextGtk*>(window.getRenderingContext().get())) {
          gtk_widget_queue_draw(GTK_WIDGET(ctx->auiWidget()));
      }
    });
}

void PlatformAbstractionGtk::windowShow(AWindow &window) {
    if (isPopup(window)) {
        gtk_popover_popup(reinterpret_cast<GtkPopover*>(gtkWidget(window)));
        return;
    }
    gtk_window_present(nativeHandle(window));
}

void PlatformAbstractionGtk::windowActivate(AWindow &window, const AString& activationToken) {
    if (isPopup(window)) {
        windowShow(window);
        return;
    }
    if (!activationToken.empty()) {
        // xdg-activation token (Wayland) / startup notification id (X11) of the launched instance
        gtk_window_set_startup_id(nativeHandle(window), activationToken.toStdString().c_str());
    }
    gtk_window_present(nativeHandle(window));
}

void PlatformAbstractionGtk::windowSetSize(AWindow &window, glm::ivec2 size) {
    if (isPopup(window)) {
        const auto position = window.getPosition();
        popupSetGeometry(window, position.x, position.y, size.x, size.y);
        return;
    }
    gtk_window_set_default_size(nativeHandle(window), size.x, size.y);
}
void PlatformAbstractionGtk::popupSetGeometry(AWindow &window, int x, int y, int width, int height) {
    auto popover = gtkWidget(window);
    auto context = dynamic_cast<RenderingContextGtk*>(window.getRenderingContext().get());
    auto surface = scaleSurfaceOf(window);
    // AUI works in physical pixels, GTK in logical ones.
    const float scale = surface ? float(gdk_surface_get_scale(surface)) : 1.f;
    auto logical = [&](int i) { return int(std::ceil(float(i) / scale)); };
    if (context && context->auiWidget()) {
        gtk_widget_set_size_request(context->auiWidget(), std::max(logical(width), 1), std::max(logical(height), 1));
    }
    // the popover is placed right below the zero-height rectangle, horizontally centered on it; so the rectangle
    // is as wide as the popup and its top-left corner is the desired position.
    GdkRectangle rect { .x = int(std::floor(float(x) / scale)), .y = int(std::floor(float(y) / scale)),
                        .width = logical(width), .height = 0 };
    gtk_popover_set_pointing_to(reinterpret_cast<GtkPopover*>(popover), &rect);
    if (gtk_widget_get_mapped(popover)) {
        gtk_popover_present(reinterpret_cast<GtkPopover*>(popover));
    }
}

void PlatformAbstractionGtk::windowSetGeometry(AWindow &window, int x, int y, int width, int height) {
    if (isPopup(window)) {
        popupSetGeometry(window, x, y, width, height);
        return;
    }
    auto handle = nativeHandle(window);
    gtk_window_set_default_size(handle, width, height);
    if (!aui::gtk4_x11::windowMoveResize(
            surfaceOf(handle), x, y, width, height, window.isPopup() && !gtk_widget_get_mapped(GTK_WIDGET(handle)))) {
        unsupportedOnWayland("windowSetGeometry (position)");
    }
}

void PlatformAbstractionGtk::windowSetIcon(AWindow &window, const AImage &image) {
    if (isPopup(window)) {
        return;
    }
    // wayland takes the icon from the .desktop file matching the application id only.
    if (!aui::gtk4_x11::windowSetIcon(surfaceOf(nativeHandle(window)), image)) {
        unsupportedOnWayland("windowSetIcon");
    }
}
void PlatformAbstractionGtk::windowHide(AWindow &window) {
    if (isPopup(window)) {
        gtk_popover_popdown(reinterpret_cast<GtkPopover*>(gtkWidget(window)));
        return;
    }
    gtk_widget_set_visible(GTK_WIDGET(nativeHandle(window)), false);
}
void PlatformAbstractionGtk::windowBlockUserInput(AWindow &window, bool blockUserInput) {
    gtk_widget_set_sensitive(GTK_WIDGET(nativeHandle(window)), !blockUserInput);
}
void PlatformAbstractionGtk::windowAllowDragNDrop(AWindow &window) {}
void PlatformAbstractionGtk::windowShowTouchscreenKeyboardImpl(AWindow &window) {}
void PlatformAbstractionGtk::windowHideTouchscreenKeyboardImpl(AWindow &window) {}
void PlatformAbstractionGtk::windowMoveToCenter(AWindow &window) {
    if (isPopup(window)) {
        return;
    }
    auto parent = parentWindow(window);
    if (!aui::gtk4_x11::windowMoveToCenter(
            surfaceOf(nativeHandle(window)), parent ? surfaceOf(nativeHandle(*parent)) : nullptr)) {
        unsupportedOnWayland("windowMoveToCenter");
    }
}
void PlatformAbstractionGtk::windowAnnounceMinMaxSize(AWindow &window) {
    if (isPopup(window)) {
        return;
    }
    auto handle = nativeHandle(window);
    const auto minSize = window.getMinimumSize();
    // AUI works in physical pixels, GTK in logical ones. The minimum size is the size of the AUI contents, so it is
    // requested from the AUI widget rather than from the GtkWindow: the latter would not account for the header bar
    // and the client-side decorations around the contents, cutting off the bottom and right edges of the contents.
    auto context = dynamic_cast<RenderingContextGtk*>(window.getRenderingContext().get());
    if (context && context->auiWidget()) {
        auto surface = scaleSurfaceOf(window);
        const float scale = surface ? float(gdk_surface_get_scale(surface)) : 1.f;
        const auto logical = [&](int i) { return i > 0 ? int(std::ceil(float(i) / scale)) : -1; };
        gtk_widget_set_size_request(context->auiWidget(), logical(minSize.x), logical(minSize.y));
    }
    const auto maxSize = window.getMaxSize();
    if (maxSize.x == -1 && maxSize.y == -1) {
        return;
    }
    if (!aui::gtk4_x11::windowSetMaxSize(surfaceOf(handle), maxSize)) {
        unsupportedOnWayland("windowAnnounceMinMaxSize (maximum size)");
    }
}

void PlatformAbstractionGtk::windowBeginMove(AWindow &window) {
    if (isPopup(window)) {
        return;
    }
    auto surface = surfaceOf(nativeHandle(window));
    if (!surface || !lastButtonPress.device) {
        return;
    }
    // works on both X11 and Wayland: gdk finds the matching serial by device and timestamp.
    gdk_toplevel_begin_move(
        reinterpret_cast<GdkToplevel*>(surface), lastButtonPress.device, int(lastButtonPress.button),
        lastButtonPress.x, lastButtonPress.y, lastButtonPress.time);
}

void PlatformAbstractionGtk::windowBeginResize(AWindow &window, ResizeEdge edge) {
    if (isPopup(window)) {
        return;
    }
    auto surface = surfaceOf(nativeHandle(window));
    if (!surface || !lastButtonPress.device) {
        return;
    }
    const auto gdkEdge = [&] {
        switch (edge) {
            case ResizeEdge::TOP_LEFT: return GDK_SURFACE_EDGE_NORTH_WEST;
            case ResizeEdge::TOP: return GDK_SURFACE_EDGE_NORTH;
            case ResizeEdge::TOP_RIGHT: return GDK_SURFACE_EDGE_NORTH_EAST;
            case ResizeEdge::RIGHT: return GDK_SURFACE_EDGE_EAST;
            case ResizeEdge::BOTTOM_RIGHT: return GDK_SURFACE_EDGE_SOUTH_EAST;
            case ResizeEdge::BOTTOM: return GDK_SURFACE_EDGE_SOUTH;
            case ResizeEdge::BOTTOM_LEFT: return GDK_SURFACE_EDGE_SOUTH_WEST;
            case ResizeEdge::LEFT: return GDK_SURFACE_EDGE_WEST;
        }
        return GDK_SURFACE_EDGE_SOUTH_EAST;
    }();
    gdk_toplevel_begin_resize(
        reinterpret_cast<GdkToplevel*>(surface), gdkEdge, lastButtonPress.device, int(lastButtonPress.button),
        lastButtonPress.x, lastButtonPress.y, lastButtonPress.time);
}

bool PlatformAbstractionGtk::windowHasNativeResizeBorders(AWindow &window) {
    auto handle = nativeWindow(window);
    return handle && gtk_window_get_decorated(handle);
}

float PlatformAbstractionGtk::windowGetDpiRatio(AWindow &window) {
    float scale = 1.0f;
    
    // Try to get surface scale first
    auto surface = scaleSurfaceOf(window);
    if (surface != nullptr) {
        scale = gdk_surface_get_scale(surface);
    }
    
    // Also apply GTK text scale settings
    auto settings = gtk_settings_get_default();
    if (settings != nullptr) {
        gint xft_dpi = 0;
        g_object_get(settings, "gtk-xft-dpi", &xft_dpi, nullptr);
        if (xft_dpi > 0) {
            // gtk-xft-dpi is in units of 1/1024 of an inch, normalize to 96 dpi base
            float text_scale = xft_dpi / 1024.0f / 96.0f;
            scale *= text_scale;
        }
    }
    
    return scale;
}

void PlatformAbstractionGtk::setupDpiChangeMonitoring() {
    auto settings = gtk_settings_get_default();
    if (settings == nullptr) {
        return;
    }
    
    // Listen to changes in gtk-xft-dpi setting
    g_signal_connect(
        settings, "notify::gtk-xft-dpi",
        G_CALLBACK(+[](GObject* object, GParamSpec* pspec, gpointer user_data) {
            // Trigger updateDpi() on all open windows to handle system-wide DPI changes
            try {
                auto& windowManager = ASurface::getWindowManager();
                for (auto& window : windowManager.getWindows()) {
                    if (window) {
                        window->getThread()->enqueue([window]() {
                            window->updateDpi();
                        });
                    }
                }
            } catch (...) {
                // Window manager may not be initialized yet
            }
        }),
        nullptr);
}

bool PlatformAbstractionGtk::windowHasNativeWindowControls(AWindow &window) {
    return !window.isPopup() && std::holds_alternative<AWindow::Init::DecorationExtended>(window.init().decoration);
}

bool PlatformAbstractionGtk::windowHasNativeWindowTitle(AWindow &window) {
    return windowHasNativeWindowControls(window);
}

void PlatformAbstractionGtk::windowSetTitlebarRect(
    AWindow &window, AOptional<ARect<int>> titlebar, AOptional<ARect<int>> title) {
    auto handle = nativeWindow(window);
    if (!handle) {
        return;
    }
    auto surface = scaleSurfaceOf(window);
    const float scale = surface ? float(gdk_surface_get_scale(surface)) : 1.f;
    const auto measure = [](GtkWidget* w, GtkOrientation orientation) {
        int minimum = 0, natural = 0;
        gtk_widget_measure(w, orientation, -1, &minimum, &natural, nullptr, nullptr);
        return natural;
    };

    if (auto native = static_cast<GtkWidget*>(g_object_get_data(G_OBJECT(handle), TITLE_KEY))) {
        gtk_widget_set_visible(native, titlebar && title);
        if (titlebar && title) {
            // AUI works in physical pixels, GTK in logical ones. The header bar covers ATitlebar horizontally, and the
            // spacers cover everything but AWindowTitle.
            const auto toLogical = [&](int v) { return int(std::round(float(v) / scale)); };
            const int windowWidth = toLogical(window.getSize().x);
            gtk_widget_set_margin_start(native, toLogical(titlebar->p1.x));
            gtk_widget_set_margin_end(native, std::max(0, windowWidth - toLogical(titlebar->p2.x)));
            const auto setSpacer = [&](const char* key, int width) {
                if (auto spacer = static_cast<GtkWidget*>(g_object_get_data(G_OBJECT(handle), key))) {
                    gtk_widget_set_size_request(spacer, std::max(0, width), -1);
                }
            };
            setSpacer(TITLE_SPACER_START_KEY, toLogical(title->p1.x - titlebar->p1.x));
            setSpacer(TITLE_SPACER_END_KEY, toLogical(titlebar->p2.x - title->p2.x));
            const float center = float(title->p1.y + title->p2.y) / 2.f / scale;
            const int height = measure(native, GTK_ORIENTATION_VERTICAL);
            gtk_widget_set_margin_top(native, std::max(0, int(std::round(center - float(height) / 2.f))));
        }
    }

    if (!titlebar) {
        return;
    }
    const auto& rect = *titlebar;
    auto start = static_cast<GtkWidget*>(g_object_get_data(G_OBJECT(handle), CONTROLS_START_KEY));
    auto end = static_cast<GtkWidget*>(g_object_get_data(G_OBJECT(handle), CONTROLS_END_KEY));
    if (!start || !end) {
        return;
    }
    const int startWidth = measure(start, GTK_ORIENTATION_HORIZONTAL);
    const int endWidth = measure(end, GTK_ORIENTATION_HORIZONTAL);
    const int height = std::max(measure(start, GTK_ORIENTATION_VERTICAL), measure(end, GTK_ORIENTATION_VERTICAL));

    // center the controls vertically in the titlebar. AUI works in physical pixels, GTK in logical ones.
    const float titlebarCenter = float(rect.p1.y + rect.p2.y) / 2.f / scale;
    const int marginTop = std::max(0, int(std::round(titlebarCenter - float(height) / 2.f)));
    gtk_widget_set_margin_top(start, marginTop);
    gtk_widget_set_margin_top(end, marginTop);

    window.windowControlsInsets = AWindowControlsInsets {
        .left = int(std::ceil(float(startWidth) * scale)),
        .right = int(std::ceil(float(endWidth) * scale)),
        .height = int(std::ceil(float(height) * scale)),
    };
}
