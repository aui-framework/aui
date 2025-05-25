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

#pragma once

#include <glib-object.h>
#include <gio/gio.h>

/**
 * @brief Provides a fake implementation of GTK4 types and constants, so we wont depend on the gtk4 dev package.
 */
namespace aui::gtk4_fake {
extern "C" {
enum GtkWidgetHelpType : int;
enum GtkWindowType : int;
enum GdkEventType : int;
enum GtkStateFlags : int;
typedef enum { GTK_ORIENTATION_HORIZONTAL, GTK_ORIENTATION_VERTICAL } GtkOrientation;
enum GdkModifierType : int;
enum GtkIconLookupFlags : int;
enum GtkPageOrientation : int;
enum GtkTextDirection : int;
enum GtkSizeRequestMode : int;
enum GtkDirectionType : int;

using GtkWidgetPath = struct _GtkWidgetPath;
using GtkContainer = struct _GtkContainer;
using GdkEventKey = struct _GdkEventKey;
using GdkWindow = struct _GdkWindow;
using GdkKeymap = struct _GdkKeymap;
using GdkGLTexture = struct _GdkGLTexture;
using GtkApplication = struct _GtkApplication;
using GtkWindow = struct _GtkWindow;
using GtkBox = struct _GtkBox;
using GtkWidget = struct _GtkWidget {
    GInitiallyUnowned parent_instance;

    /*< private >*/

    void *priv;
};
using GtkSnapshot = struct _GtkSnapshot;
using GtkIconInfo = struct _GtkIconInfo;
using GdkScreen = struct _GdkScreen;
using GdkColor = struct _GdkColor;
using GdkEvent = struct _GdkEvent;
using GdkKeymapKey = struct _GdkKeymapKey;
using GdkTexture = struct _GdkTexture;
using GdkGLTextureBuilder = struct _GdkGLTextureBuilder;
using GtkSettings = struct _GtkSettings;
using GtkStyleContext = struct _GtkStyleContext;
using GskRenderNode = struct _GskRenderNode;
using GtkIMContext = struct _GtkIMContext;
using GtkNative = struct _GtkNative;
using GdkDevice = struct _GdkDevice;
using GdkSurface = struct _GdkSurface;
using GtkFileChooser = struct _GtkFileChooser;
using GtkIconTheme = struct _GtkIconTheme;
using GdkDisplay = struct _GdkDisplay;
using GdkPixbuf = struct _GdkPixbuf;
using GtkTreeModel = struct _GtkTreeModel;
using GtkCssProvider = struct _GtkCssProvider;
using GtkStyleProvider = struct _GtkStyleProvider;
using GtkImage = struct _GtkImage;
using GtkFileFilter = struct _GtkFileFilter;
using GtkPrinter = struct _GtkPrinter;
using GtkPrintBackend = struct _GtkPrintBackend;
using GdkRectangle = struct _GdkRectangle;
using GtkIMMulticontext = struct _GtkIMMulticontext;
using GtkPageSetup = struct _GtkPageSetup;
using GtkPaperSize = struct _GtkPaperSize;
using GtkPrintJob = struct _GtkPrintJob;
using GtkPrintSettings = struct _GtkPrintSettings;
using GtkPageRange = struct _GtkPageRange;
using GtkPrintUnixDialog = struct _GtkPrintUnixDialog;
using PangoContext = struct _PangoContext;
using GtkRequisition = struct _GtkRequisition;
using GdkPaintable = struct _GdkPaintable;
using GdkSnapshot = struct _GdkSnapshot;
using GdkSeat = struct _GdkSeat;
using GdkToplevel = struct _GdkToplevel;
using GdkMonitor = struct _GdkMonitor;
using GtkTooltip = struct _GtkTooltip;
using GtkAlertDialog = struct _GtkAlertDialog;
using GtkCssStyleChange = struct _GtkCssStyleChange;
using GtkWidgetClassPrivate = struct _GtkWidgetClassPrivate;

using graphene_point_t = struct _graphene_point_t {
    float x;
    float y;
};

using graphene_size_t = struct _graphene_size_t {
    float width;
    float height;
};

using graphene_rect_t = struct _graphene_rect_t {
    graphene_point_t origin;
    graphene_size_t size;
};

using GdkGLContext = struct _GdkGLContext;
using GtkPrinterFunc = void *;
using PangoAttrList = void *;
using GtkPrintJobCompleteFunc = void *;
using GdkEventFunc = void (*)(GdkEvent *event, gpointer data);

using cairo_t = struct _cairo;
using cairo_surface_t = struct _cairo_surface;

struct _GdkEventKey {
    GdkEventType type;
    GdkWindow *window;
    gint8 send_event;
    guint32 time;
    guint state;
    guint keyval;
    gint length;
    gchar *string;
    guint16 hardware_keycode;
    guint8 group;
    guint is_modifier : 1;
};

struct _GdkColor {
    guint32 pixel;
    guint16 red;
    guint16 green;
    guint16 blue;
};

typedef enum {
    GSK_NOT_A_RENDER_NODE = 0,
    GSK_CONTAINER_NODE,
    GSK_CAIRO_NODE,
    GSK_COLOR_NODE,
    GSK_LINEAR_GRADIENT_NODE,
    GSK_REPEATING_LINEAR_GRADIENT_NODE,
    GSK_RADIAL_GRADIENT_NODE,
    GSK_REPEATING_RADIAL_GRADIENT_NODE,
    GSK_CONIC_GRADIENT_NODE,
    GSK_BORDER_NODE,
    GSK_TEXTURE_NODE,
    GSK_INSET_SHADOW_NODE,
    GSK_OUTSET_SHADOW_NODE,
    GSK_TRANSFORM_NODE,
    GSK_OPACITY_NODE,
    GSK_COLOR_MATRIX_NODE,
    GSK_REPEAT_NODE,
    GSK_CLIP_NODE,
    GSK_ROUNDED_CLIP_NODE,
    GSK_SHADOW_NODE,
    GSK_BLEND_NODE,
    GSK_CROSS_FADE_NODE,
    GSK_TEXT_NODE,
    GSK_BLUR_NODE,
    GSK_DEBUG_NODE,
    GSK_GL_SHADER_NODE,
    GSK_TEXTURE_SCALE_NODE,
    GSK_MASK_NODE,
    GSK_FILL_NODE,
    GSK_STROKE_NODE,
    GSK_SUBSURFACE_NODE,
} GskRenderNodeType;

typedef enum {
    GDK_MEMORY_B8G8R8A8_PREMULTIPLIED,
    GDK_MEMORY_A8R8G8B8_PREMULTIPLIED,
    GDK_MEMORY_R8G8B8A8_PREMULTIPLIED,
    GDK_MEMORY_B8G8R8A8,
    GDK_MEMORY_A8R8G8B8,
    GDK_MEMORY_R8G8B8A8,
    GDK_MEMORY_A8B8G8R8,
    GDK_MEMORY_R8G8B8,
    GDK_MEMORY_B8G8R8,
    GDK_MEMORY_R16G16B16,
    GDK_MEMORY_R16G16B16A16_PREMULTIPLIED,
    GDK_MEMORY_R16G16B16A16,
    GDK_MEMORY_R16G16B16_FLOAT,
    GDK_MEMORY_R16G16B16A16_FLOAT_PREMULTIPLIED,
    GDK_MEMORY_R16G16B16A16_FLOAT,
    GDK_MEMORY_R32G32B32_FLOAT,
    GDK_MEMORY_R32G32B32A32_FLOAT_PREMULTIPLIED,
    GDK_MEMORY_R32G32B32A32_FLOAT,
    GDK_MEMORY_G8A8_PREMULTIPLIED,
    GDK_MEMORY_G8A8,
    GDK_MEMORY_G8,
    GDK_MEMORY_G16A16_PREMULTIPLIED,
    GDK_MEMORY_G16A16,
    GDK_MEMORY_G16,
    GDK_MEMORY_A8,
    GDK_MEMORY_A16,
    GDK_MEMORY_A16_FLOAT,
    GDK_MEMORY_A32_FLOAT,
    GDK_MEMORY_A8B8G8R8_PREMULTIPLIED,
    GDK_MEMORY_B8G8R8X8,
    GDK_MEMORY_X8R8G8B8,
    GDK_MEMORY_R8G8B8X8,
    GDK_MEMORY_X8B8G8R8,

    GDK_MEMORY_N_FORMATS
} GdkMemoryFormat;

typedef enum { GTK_UNIT_NONE, GTK_UNIT_POINTS, GTK_UNIT_INCH, GTK_UNIT_MM } GtkUnit;

typedef enum {
    GTK_PRINT_PAGES_ALL,
    GTK_PRINT_PAGES_CURRENT,
    GTK_PRINT_PAGES_RANGES,
    GTK_PRINT_PAGES_SELECTION
} GtkPrintPages;

typedef enum { GDK_GL_API_GL = 1 << 0, GDK_GL_API_GLES = 1 << 1 } GdkGLAPI;

typedef enum {
    GTK_PRINT_CAPABILITY_PAGE_SET = 1 << 0,
    GTK_PRINT_CAPABILITY_COPIES = 1 << 1,
    GTK_PRINT_CAPABILITY_COLLATE = 1 << 2,
    GTK_PRINT_CAPABILITY_REVERSE = 1 << 3,
    GTK_PRINT_CAPABILITY_SCALE = 1 << 4,
    GTK_PRINT_CAPABILITY_GENERATE_PDF = 1 << 5,
    GTK_PRINT_CAPABILITY_GENERATE_PS = 1 << 6,
    GTK_PRINT_CAPABILITY_PREVIEW = 1 << 7,
    GTK_PRINT_CAPABILITY_NUMBER_UP = 1 << 8,
    GTK_PRINT_CAPABILITY_NUMBER_UP_LAYOUT = 1 << 9
} GtkPrintCapabilities;

typedef enum {
    GTK_SYSTEM_SETTING_DPI,
    GTK_SYSTEM_SETTING_FONT_NAME,
    GTK_SYSTEM_SETTING_FONT_CONFIG,
    GTK_SYSTEM_SETTING_DISPLAY,
    GTK_SYSTEM_SETTING_ICON_THEME
} GtkSystemSetting;

typedef enum { GSK_SCALING_FILTER_LINEAR, GSK_SCALING_FILTER_NEAREST, GSK_SCALING_FILTER_TRILINEAR } GskScalingFilter;

// Not defined in GTK.
constexpr int GSK_RENDER_NODE_MAX_VALUE = GSK_SUBSURFACE_NODE;

constexpr int GTK_ICON_LOOKUP_USE_BUILTIN = 1 << 2;
constexpr int GTK_ICON_LOOKUP_GENERIC_FALLBACK = 1 << 3;
constexpr int GTK_ICON_LOOKUP_FORCE_SIZE = 1 << 4;

constexpr auto GTK_WINDOW_TOPLEVEL = static_cast<GtkWindowType>(0);

using GtkWidgetClass = struct _GtkWidgetClass {
    GInitiallyUnownedClass parent_class;

    /*< public >*/

    /* basics */
    void (*show)(GtkWidget *widget);
    void (*hide)(GtkWidget *widget);
    void (*map)(GtkWidget *widget);
    void (*unmap)(GtkWidget *widget);
    void (*realize)(GtkWidget *widget);
    void (*unrealize)(GtkWidget *widget);
    void (*root)(GtkWidget *widget);
    void (*unroot)(GtkWidget *widget);
    void (*size_allocate)(GtkWidget *widget, int width, int height, int baseline);
    void (*state_flags_changed)(GtkWidget *widget, GtkStateFlags previous_state_flags);
    void (*direction_changed)(GtkWidget *widget, GtkTextDirection previous_direction);

    /* size requests */
    GtkSizeRequestMode (*get_request_mode)(GtkWidget *widget);
    void (*measure)(
        GtkWidget *widget, GtkOrientation orientation, int for_size, int *minimum, int *natural, int *minimum_baseline,
        int *natural_baseline);

    /* Mnemonics */
    gboolean (*mnemonic_activate)(GtkWidget *widget, gboolean group_cycling);

    /* explicit focus */
    gboolean (*grab_focus)(GtkWidget *widget);
    gboolean (*focus)(GtkWidget *widget, GtkDirectionType direction);
    void (*set_focus_child)(GtkWidget *widget, GtkWidget *child);

    /* keyboard navigation */
    void (*move_focus)(GtkWidget *widget, GtkDirectionType direction);
    gboolean (*keynav_failed)(GtkWidget *widget, GtkDirectionType direction);

    gboolean (*query_tooltip)(GtkWidget *widget, int x, int y, gboolean keyboard_tooltip, GtkTooltip *tooltip);

    void (*compute_expand)(GtkWidget *widget, gboolean *hexpand_p, gboolean *vexpand_p);

    void (*css_changed)(GtkWidget *widget, GtkCssStyleChange *change);

    void (*system_setting_changed)(GtkWidget *widget, GtkSystemSetting settings);

    void (*snapshot)(GtkWidget *widget, GtkSnapshot *snapshot);

    gboolean (*contains)(GtkWidget *widget, double x, double y);

    /*< private >*/

    GtkWidgetClassPrivate *priv;

    gpointer padding[8];
};

#ifndef GTK_TYPE_WIDGET
#define GTK_TYPE_WIDGET            (gtk_widget_get_type())
#define GTK_WIDGET(widget)         (G_TYPE_CHECK_INSTANCE_CAST((widget), GTK_TYPE_WIDGET, GtkWidget))
#define GTK_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_WIDGET, GtkWidgetClass))
#define GTK_IS_WIDGET(widget)      (G_TYPE_CHECK_INSTANCE_TYPE((widget), GTK_TYPE_WIDGET))
#define GTK_IS_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_WIDGET))
#define GTK_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_WIDGET, GtkWidgetClass))

#define GDK_TYPE_GL_TEXTURE    (gdk_gl_texture_get_type())
#define GDK_GL_TEXTURE(obj)    (G_TYPE_CHECK_INSTANCE_CAST((obj), GDK_TYPE_GL_TEXTURE, GdkGLTexture))
#define GDK_IS_GL_TEXTURE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GDK_TYPE_GL_TEXTURE))

#define GTK_TYPE_APPLICATION            (gtk_application_get_type())
#define GTK_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_APPLICATION, GtkApplication))
#define GTK_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_APPLICATION, GtkApplicationClass))
#define GTK_IS_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_APPLICATION))
#define GTK_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_APPLICATION))
#define GTK_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_APPLICATION, GtkApplicationClass))

#define GTK_TYPE_WINDOW            (gtk_window_get_type())
#define GTK_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_WINDOW, GtkWindow))
#define GTK_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_WINDOW, GtkWindowClass))
#define GTK_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_WINDOW))
#define GTK_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_WINDOW))
#define GTK_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_WINDOW, GtkWindowClass))

#define GTK_TYPE_BOX            (gtk_box_get_type())
#define GTK_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_BOX, GtkBox))
#define GTK_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_BOX, GtkBoxClass))
#define GTK_IS_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_BOX))
#define GTK_IS_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_BOX))
#define GTK_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_BOX, GtkBoxClass))

#endif
}
}   // namespace aui::gtk4_fake
