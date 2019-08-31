#pragma once

/// This file provides fmt::formatter specializations for all events in the engine.

#include "../io/fmt.h"

#include "event/keyboard_event.h"
#include "event/mouse_event.h"
#include "event/window_event.h"

LSTD_BEGIN_NAMESPACE

/* @TODO:
template <>
struct fmt::formatter<window_closed_event> {
    void format(const window_closed_event &value, format_context *f) const {
        f->debug_struct("window_closed_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<window_resized_event> {
    void format(const window_resized_event &value, format_context *f) const {
        f->debug_struct("window_resized_event")
            .field("Window", (void *) value.Window)
            ->field("Width", value.Width)
            ->field("Height", value.Height)
            ->finish();
    }
};

template <>
struct fmt::formatter<window_gained_focus_event> {
    void format(const window_gained_focus_event &value, format_context *f) const {
        f->debug_struct("window_gained_Focus_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<window_lost_focus_event> {
    void format(const window_lost_focus_event &value, format_context *f) const {
        f->debug_struct("window_lost_focus_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<window_moved_event> {
    void format(const window_moved_event &value, format_context *f) const {
        f->debug_struct("window_moved_event")
            .field("Window", (void *) value.Window)
            ->field("Left", value.Left)
            ->field("Top", value.Top)
            ->finish();
    }
};

template <>
struct fmt::formatter<window_files_dropped_event> {
    void format(const window_files_dropped_event &value, format_context *f) const {
        // @TODO Print paths when we have the simpler API
        f->debug_struct("window_moved_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<key_pressed_event> {
    void format(const key_pressed_event &value, format_context *f) const {
        // @TODO: We to provide an API to simplify this mess (add fmt::formatter specializations for debug_*)...

        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & Modifier_Shift), (bool) (value.Modifiers & Modifier_Control),
                    (bool) (value.Modifiers & Modifier_Alt), (bool) (value.Modifiers & Modifier_Super));

        f->debug_struct("key_pressed_event")
            .field("Window", (void *) value.Window)
            ->field("KeyCode", key_name_from_code(value.KeyCode))
            ->field("Modifiers", mods)
            ->field("Repeat", value.Repeat)
            ->finish();
    }
};

template <>
struct fmt::formatter<key_released_event> {
    void format(const key_released_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & Modifier_Shift), (bool) (value.Modifiers & Modifier_Control),
                    (bool) (value.Modifiers & Modifier_Alt), (bool) (value.Modifiers & Modifier_Super));

        f->debug_struct("key_released_event")
            .field("Window", (void *) value.Window)
            ->field("KeyCode", key_name_from_code(value.KeyCode))
            ->field("Modifiers", mods)
            ->finish();
    }
};

template <>
struct fmt::formatter<key_typed_event> {
    void format(const key_typed_event &value, format_context *f) const {
        string cp;
        fmt::sprint(&cp, "{:c}", value.CodePoint);

        f->debug_struct("key_typed_event").field("Window", (void *) value.Window)->field("CodePoint", cp)->finish();
    }
};

template <>
struct fmt::formatter<mouse_button_pressed_event> {
    void format(const mouse_button_pressed_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & Modifier_Shift), (bool) (value.Modifiers & Modifier_Control),
                    (bool) (value.Modifiers & Modifier_Alt), (bool) (value.Modifiers & Modifier_Super));

        f->debug_struct("mouse_button_pressed_event")
            .field("Window", (void *) value.Window)
            ->field("Button", mouse_button_name_from_code(value.Button))
            ->field("Modifiers", mods)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->field("DoubleClick", value.DoubleClick)
            ->finish();
    }
};

template <>
struct fmt::formatter<mouse_button_released_event> {
    void format(const mouse_button_released_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & Modifier_Shift), (bool) (value.Modifiers & Modifier_Control),
                    (bool) (value.Modifiers & Modifier_Alt), (bool) (value.Modifiers & Modifier_Super));

        f->debug_struct("mouse_button_released_event")
            .field("Window", (void *) value.Window)
            ->field("Button", mouse_button_name_from_code(value.Button))
            ->field("Modifiers", mods)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->finish();
    }
};

template <>
struct fmt::formatter<mouse_scrolled_event> {
    void format(const mouse_scrolled_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & Modifier_Shift), (bool) (value.Modifiers & Modifier_Control),
                    (bool) (value.Modifiers & Modifier_Alt), (bool) (value.Modifiers & Modifier_Super));

        string buttonsDown;
        fmt::sprint(
            &mods,
            "{{\n        Left = {},\n        Middle = {},\n        Right = {},\n        X1 = {},\n        X2 = {},\n}}",
            (bool) (value.ButtonsDown & Mouse_Button_Left), (bool) (value.ButtonsDown & Mouse_Button_Middle),
            (bool) (value.ButtonsDown & Mouse_Button_Right), (bool) (value.ButtonsDown & Mouse_Button_X1),
            (bool) (value.ButtonsDown & Mouse_Button_X2));

        f->debug_struct("mouse_scrolled_event")
            .field("Window", (void *) value.Window)
            ->field("DeltaX", value.DeltaX)
            ->field("DeltaY", value.DeltaY)
            ->field("Modifiers", mods)
            ->field("ButtonsDown", buttonsDown)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->finish();
    }
};

template <>
struct fmt::formatter<mouse_entered_event> {
    void format(const mouse_entered_event &value, format_context *f) const {
        f->debug_struct("mouse_entered_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<mouse_left_event> {
    void format(const mouse_left_event &value, format_context *f) const {
        f->debug_struct("mouse_left_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<mouse_moved_event> {
    void format(const mouse_moved_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & Modifier_Shift), (bool) (value.Modifiers & Modifier_Control),
                    (bool) (value.Modifiers & Modifier_Alt), (bool) (value.Modifiers & Modifier_Super));

        string buttonsDown;
        fmt::sprint(
            &mods,
            "{{\n        Left = {},\n        Middle = {},\n        Right = {},\n        X1 = {},\n        X2 = {},\n}}",
            (bool) (value.ButtonsDown & Mouse_Button_Left), (bool) (value.ButtonsDown & Mouse_Button_Middle),
            (bool) (value.ButtonsDown & Mouse_Button_Right), (bool) (value.ButtonsDown & Mouse_Button_X1),
            (bool) (value.ButtonsDown & Mouse_Button_X2));

        f->debug_struct("mouse_moved_event")
            .field("Window", (void *) value.Window)
            ->field("Modifiers", mods)
            ->field("ButtonsDown", buttonsDown)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->finish();
    }
};
*/

LSTD_END_NAMESPACE
