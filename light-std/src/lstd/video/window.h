#pragma once

#include "../math.h"
#include "../memory/pixel_buffer.h"
#include "cursor.h"
#include "event.h"
#include "monitor.h"

struct HWND__;
struct HICON__;

LSTD_BEGIN_NAMESPACE

struct window {
    union platform_data {
        struct {
            HWND__ *hWnd = null;
            HICON__ *BigIcon = null, *SmallIcon = null;

            bool CursorTracked = false;
            bool FrameAction = false;

            u16 Surrogate = 0;  // Used when handling text input

            // The last received cursor position, regardless of source
            vec2<s32> LastCursorPos = {no_init};
        } Win32;
    } PlatformData{};

    // Used to indicate that you don't care about the starting position of the window.
    // Also used in the forced aspect ratio.
    // .. also used in forced min/max dimensions.
    static constexpr auto DONT_CARE = 0x1FFF0000;

    // Used to indicate that the window should be centered on the screen
    static constexpr auto CENTERED = 0x2FFF0000;

    enum flags : u32 {
        // These flags are added/removed while the window is being used:
        // (read-only, use show(), hide(), minimize() etc. to change):

        SHOWN = BIT(2),      // Window is visible
        HIDDEN = BIT(3),     // Window is not visible
        MINIMIZED = BIT(4),  // Window is minimized
        MAXIMIZED = BIT(5),  // Window is maximized
        FOCUSED = BIT(6),    // Window is focused

        // These flags get set by the user when the window is initialized or manually later:

        BORDERLESS = BIT(7),  // No window decoration; Specify in _init()_ or use _set_borderless()_
        RESIZABLE = BIT(8),   // Window can be resized; Specify in _init()_ or use _set_resizable()_

        AUTO_MINIMIZE = BIT(9),   // Indicates whether a full screen window is minimized on focus loss
                                  // Specify in _init()_ or manually modify _Flags_
        ALWAYS_ON_TOP = BIT(10),  // Also called floating, topmost, etc.
                                  // Specify in _init()_ or use _set_always_on_top()_
        FOCUS_ON_SHOW = BIT(11),  // Specify in _init()_ or manually modify _Flags_
        ALPHA = BIT(12),  // Window contents with alpha value get the color of the thing behind them, can only be
                          // specified when creating the window! Can also check this in the _Flags_ after creation
                          // to see if the window was able to be created with this flag.

        VSYNC = BIT(13),
        CLOSE_ON_ALT_F4 = BIT(14),  // Specify in _init()_ or manually modify _Flags_
        MOUSE_PASS_THROUGH =
            BIT(15),  // Specifies that the window is transparent for mouse input (e.g. when testing if
                      // a background window is hovered, but this one has the flag, it treats this one
                      // as "transparent" and continues to test on the window behind it). Specify in _init()_ or
                      // manually modify _Flags_. Note that this flag is unrelated to visual transparency.

        CREATION_FLAGS = SHOWN | BORDERLESS | RESIZABLE | AUTO_MINIMIZE | ALWAYS_ON_TOP | FOCUS_ON_SHOW | ALPHA |
                         VSYNC | CLOSE_ON_ALT_F4 | MOUSE_PASS_THROUGH
    };

    enum cursor_mode : u8 {
        CURSOR_NORMAL = 0,  // Makes the cursor visible and behaving normally
        CURSOR_HIDDEN,      // Makes the cursor invisible when it is over the content area of the window but does not
                            // restrict the cursor from leaving.
        CURSOR_DISABLED     // Hides and grabs the cursor, providing virtual and unlimited cursor movement.
    };

    inline static u32 s_NextID = 0;

    static constexpr u32 INVALID_ID =
        (u32) -1;         // _ID_ is set to that when the window is not initialized or destroyed and no longer valid
    u32 ID = INVALID_ID;  // Unique ID for each window

    u32 Flags = 0;

    // The state of each key (true if pressed)
    stack_array<bool, Key_Last + 1> Keys;
    stack_array<bool, Key_Last + 1> LastFrameKeys;  // Needed internally

    // The state of each key if it got changed this frame (true if pressed), use this to check for non-repeat
    stack_array<bool, Key_Last + 1> KeysThisFrame;

    // The state of the mouse buttons (true if clicked)
    stack_array<bool, Mouse_Button_Last + 1> MouseButtons;
    stack_array<bool, Mouse_Button_Last + 1> LastFrameMouseButtons;  // Needed internally

    // The state of each mouse button if it got changed this frame (true if clicked), use this to check for non-repeat
    stack_array<bool, Mouse_Button_Last + 1> MouseButtonsThisFrame;

    // _true_ when the window is closing
    bool IsDestroying = false;

    display_mode DisplayMode;
    monitor *Monitor = null;  // Non-null if we are fullscreen!
    cursor *Cursor = null;
    cursor_mode CursorMode = CURSOR_NORMAL;

    s32 AspectRatioNumerator = DONT_CARE, AspectRatioDenominator = DONT_CARE;

    // Min, max dimensions
    s32 MinW = DONT_CARE, MinH = DONT_CARE, MaxW = DONT_CARE, MaxH = DONT_CARE;

    // Virtual cursor position when cursor is disabled
    vec2<s32> VirtualCursorPos = {no_init};

    // Enable raw (unscaled and unaccelerated) mouse motion when the cursor is disabled.
    // May not be supported on some platforms.
    bool RawMouseMotion = false;

    // We keep track of created windows in a linked list
    window *Next = null;

    window() = default;
    ~window() { release(); }

    // You can use _DONT_CARE_ or _CENTERED_ for _x_ and _y_.
    // Returns _this_.
    window *init(string title, s32 x, s32 y, s32 width, s32 height, u32 flags);

    // Destroys the window
    void release();

    static void update();

    signal<bool(const event &), collector_while0<bool>> Event;

    string get_title();  // Gets temporarily allocated. Invalidated the next time _window::update_ is called..
    void set_title(string title);

    void set_fullscreen(monitor *mon, s32 width, s32 height, s32 refreshRate = DONT_CARE);
    bool is_fullscreen() { return Monitor; }

    // We choose the icons with the closest sizes we need.
    // The image data is 32-bit, little-endian, non-premultiplied RGBA,
    // i.e. eight bits per channel with the red channel first.
    // The pixels are arranged canonically as sequential rows, starting from the top-left corner.
    void set_icon(array<pixel_buffer> icons);

    void set_cursor(cursor *curs);

    vec2<s32> get_cursor_pos();
    void set_cursor_pos(vec2<s32> pos);
    void set_cursor_pos(s32 x, s32 y) { set_pos({x, y}); }

    vec2<s32> get_pos();
    void set_pos(vec2<s32> pos);
    void set_pos(s32 x, s32 y) { set_pos({x, y}); }

    vec2<s32> get_size();
    void set_size(vec2<s32> size);
    void set_size(s32 width, s32 height) { set_size({width, height}); }

    // May not map 1:1 with window size (e.g. Retina display on Mac)
    vec2<s32> get_framebuffer_size();

    rect get_adjusted_bounds();  // Gets the full area the window occupies (including title bar, borders, etc.),
                                 // relative to window's position

    // You can call this with DONT_CARE
    void set_size_limits(vec2<s32> minDimension, vec2<s32> maxDimension);
    void set_size_limits(s32 minWidth, s32 minHeight, s32 maxWidth, s32 maxHeight) {
        set_size_limits({minWidth, minHeight}, {maxWidth, maxHeight});
    }

    // Call with DONT_CARE to reset
    void set_forced_aspect_ratio(s32 numerator, s32 denominator);

    void set_raw_mouse(bool enabled);
    void set_cursor_mode(cursor_mode mode);

    // 0.0f - 1.0f
    f32 get_opacity();
    // 0.0f - 1.0f
    void set_opacity(f32 opacity);

    void set_borderless(bool enabled);
    void set_resizable(bool enabled);
    void set_always_on_top(bool enabled);

    // Returns true if the cursor is inside the content area of the window
    bool is_hovered();

    bool is_visible();

    void show();
    void hide();
    void minimize();
    void restore();
    void maximize();
    void focus();

    // Flashes the window
    void request_attention();
};

LSTD_END_NAMESPACE
