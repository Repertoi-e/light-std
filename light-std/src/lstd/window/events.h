#pragma once

/// This file provides fmt::formatter specializations for all events in the engine.

#include "../io/fmt.h"

#include "event/keyboard_event.h"
#include "event/mouse_event.h"
#include "event/window_event.h"

LSTD_BEGIN_NAMESPACE

template <>
struct fmt::formatter<window::window_closed_event> {
    void format(const window::window_closed_event &value, format_context *f) const {
        f->debug_struct("window_closed_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<window::window_resized_event> {
    void format(const window::window_resized_event &value, format_context *f) const {
        f->debug_struct("window_resized_event")
            .field("Window", (void *) value.Window)
            ->field("Width", value.Width)
            ->field("Height", value.Height)
            ->finish();
    }
};

template <>
struct fmt::formatter<window::window_gained_focus_event> {
    void format(const window::window_gained_focus_event &value, format_context *f) const {
        f->debug_struct("window_gained_Focus_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<window::window_lost_focus_event> {
    void format(const window::window_lost_focus_event &value, format_context *f) const {
        f->debug_struct("window_lost_focus_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<window::window_moved_event> {
    void format(const window::window_moved_event &value, format_context *f) const {
        f->debug_struct("window_moved_event")
            .field("Window", (void *) value.Window)
            ->field("Left", value.Left)
            ->field("Top", value.Top)
            ->finish();
    }
};

template <>
struct fmt::formatter<window::key_pressed_event> {
    void format(const window::key_pressed_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & window::Modifier_Shift),
                    (bool) (value.Modifiers & window::Modifier_Control),
                    (bool) (value.Modifiers & window::Modifier_Alt), (bool) (value.Modifiers & window::Modifier_Super));

        f->debug_struct("key_pressed_event")
            .field("Window", (void *) value.Window)
            ->field("KeyCode", window::key_name_from_code(value.KeyCode))
            ->field("Modifiers", mods)
            ->field("Repeat", value.Repeat)
            ->finish();
    }
};

template <>
struct fmt::formatter<window::key_released_event> {
    void format(const window::key_released_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & window::Modifier_Shift),
                    (bool) (value.Modifiers & window::Modifier_Control),
                    (bool) (value.Modifiers & window::Modifier_Alt), (bool) (value.Modifiers & window::Modifier_Super));

        f->debug_struct("key_released_event")
            .field("Window", (void *) value.Window)
            ->field("KeyCode", window::key_name_from_code(value.KeyCode))
            ->field("Modifiers", mods)
            ->finish();
    }
};

template <>
struct fmt::formatter<window::key_typed_event> {
    void format(const window::key_typed_event &value, format_context *f) const {
        string cp;
        fmt::sprint(&cp, "{:c}", value.CodePoint);

        f->debug_struct("key_typed_event").field("Window", (void *) value.Window)->field("CodePoint", cp)->finish();
    }
};

template <>
struct fmt::formatter<window::mouse_button_pressed_event> {
    void format(const window::mouse_button_pressed_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & window::Modifier_Shift),
                    (bool) (value.Modifiers & window::Modifier_Control),
                    (bool) (value.Modifiers & window::Modifier_Alt), (bool) (value.Modifiers & window::Modifier_Super));

        f->debug_struct("mouse_button_pressed_event")
            .field("Window", (void *) value.Window)
            ->field("Button", window::mouse_button_name_from_code(value.Button))
            ->field("Modifiers", mods)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->finish();
    }
};

template <>
struct fmt::formatter<window::mouse_button_released_event> {
    void format(const window::mouse_button_released_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & window::Modifier_Shift),
                    (bool) (value.Modifiers & window::Modifier_Control),
                    (bool) (value.Modifiers & window::Modifier_Alt), (bool) (value.Modifiers & window::Modifier_Super));

        f->debug_struct("mouse_button_released_event")
            .field("Window", (void *) value.Window)
            ->field("Button", window::mouse_button_name_from_code(value.Button))
            ->field("Modifiers", mods)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->finish();
    }
};

template <>
struct fmt::formatter<window::mouse_scrolled_event> {
    void format(const window::mouse_scrolled_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & window::Modifier_Shift),
                    (bool) (value.Modifiers & window::Modifier_Control),
                    (bool) (value.Modifiers & window::Modifier_Alt), (bool) (value.Modifiers & window::Modifier_Super));

        string buttonsDown;
        fmt::sprint(
            &mods,
            "{{\n        Left = {},\n        Middle = {},\n        Right = {},\n        X1 = {},\n        X2 = {},\n}}",
            (bool) (value.ButtonsDown & window::Mouse_Button_Left),
            (bool) (value.ButtonsDown & window::Mouse_Button_Middle),
            (bool) (value.ButtonsDown & window::Mouse_Button_Right),
            (bool) (value.ButtonsDown & window::Mouse_Button_X1), (bool) (value.ButtonsDown & window::Mouse_Button_X2));

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
struct fmt::formatter<window::mouse_entered_event> {
    void format(const window::mouse_entered_event &value, format_context *f) const {
        f->debug_struct("mouse_entered_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<window::mouse_left_event> {
    void format(const window::mouse_left_event &value, format_context *f) const {
        f->debug_struct("mouse_left_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<window::mouse_moved_event> {
    void format(const window::mouse_moved_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n        Shift = {},\n        Control = {},\n        Alt = {},\n        Super = {},\n}}",
                    (bool) (value.Modifiers & window::Modifier_Shift),
                    (bool) (value.Modifiers & window::Modifier_Control),
                    (bool) (value.Modifiers & window::Modifier_Alt), (bool) (value.Modifiers & window::Modifier_Super));

        string buttonsDown;
        fmt::sprint(
            &mods,
            "{{\n        Left = {},\n        Middle = {},\n        Right = {},\n        X1 = {},\n        X2 = {},\n}}",
            (bool) (value.ButtonsDown & window::Mouse_Button_Left),
            (bool) (value.ButtonsDown & window::Mouse_Button_Middle),
            (bool) (value.ButtonsDown & window::Mouse_Button_Right),
            (bool) (value.ButtonsDown & window::Mouse_Button_X1), (bool) (value.ButtonsDown & window::Mouse_Button_X2));

        f->debug_struct("mouse_moved_event")
            .field("Window", (void *) value.Window)
            ->field("Modifiers", mods)
            ->field("ButtonsDown", buttonsDown)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->finish();
    }
};

LSTD_END_NAMESPACE
