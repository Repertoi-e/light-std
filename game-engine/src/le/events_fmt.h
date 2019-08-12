#pragma once

/// This file provides fmt::formatter specializations for all events in the engine.

#include <lstd/io/fmt.h>

#include "event/keyboard_event.h"
#include "event/mouse_event.h"
#include "event/window_event.h"

template <>
struct fmt::formatter<le::window_closed_event> {
    void format(const le::window_closed_event &value, format_context *f) const {
        f->debug_struct("window_closed_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<le::window_resized_event> {
    void format(const le::window_resized_event &value, format_context *f) const {
        f->debug_struct("window_resized_event")
            .field("Window", (void *) value.Window)
            ->field("Width", value.Width)
            ->field("Height", value.Height)
            ->finish();
    }
};

template <>
struct fmt::formatter<le::window_gained_focus_event> {
    void format(const le::window_gained_focus_event &value, format_context *f) const {
        f->debug_struct("window_gained_Focus_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<le::window_lost_focus_event> {
    void format(const le::window_lost_focus_event &value, format_context *f) const {
        f->debug_struct("window_lost_focus_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<le::window_moved_event> {
    void format(const le::window_moved_event &value, format_context *f) const {
        f->debug_struct("window_moved_event")
            .field("Window", (void *) value.Window)
            ->field("Left", value.Left)
            ->field("Top", value.Top)
            ->finish();
    }
};

template <>
struct fmt::formatter<le::key_pressed_event> {
    void format(const le::key_pressed_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n\t\tShift = {},\n\t\tControl = {},\n\t\tAlt = {},\n\t\tSuper = {},\n}}",
                    (bool) (value.Modifiers & le::Modifier_Shift), (bool) (value.Modifiers & le::Modifier_Control),
                    (bool) (value.Modifiers & le::Modifier_Alt), (bool) (value.Modifiers & le::Modifier_Super));

        f->debug_struct("key_pressed_event")
            .field("Window", (void *) value.Window)
            ->field("KeyCode", le::key_name_from_code(value.KeyCode))
            ->field("Modifiers", mods)
            ->field("Repeat", value.Repeat)
            ->finish();
    }
};

template <>
struct fmt::formatter<le::key_released_event> {
    void format(const le::key_released_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n\t\tShift = {},\n\t\tControl = {},\n\t\tAlt = {},\n\t\tSuper = {},\n}}",
                    (bool) (value.Modifiers & le::Modifier_Shift), (bool) (value.Modifiers & le::Modifier_Control),
                    (bool) (value.Modifiers & le::Modifier_Alt), (bool) (value.Modifiers & le::Modifier_Super));

        f->debug_struct("key_released_event")
            .field("Window", (void *) value.Window)
            ->field("KeyCode", le::key_name_from_code(value.KeyCode))
            ->field("Modifiers", mods)
            ->finish();
    }
};

template <>
struct fmt::formatter<le::key_typed_event> {
    void format(const le::key_typed_event &value, format_context *f) const {
        f->debug_struct("key_typed_event")
            .field("Window", (void *) value.Window)
            ->field("CodePoint", value.CodePoint)
            ->finish();
    }
};

template <>
struct fmt::formatter<le::mouse_button_pressed_event> {
    void format(const le::mouse_button_pressed_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n\t\tShift = {},\n\t\tControl = {},\n\t\tAlt = {},\n\t\tSuper = {},\n}}",
                    (bool) (value.Modifiers & le::Modifier_Shift), (bool) (value.Modifiers & le::Modifier_Control),
                    (bool) (value.Modifiers & le::Modifier_Alt), (bool) (value.Modifiers & le::Modifier_Super));

        f->debug_struct("mouse_button_pressed_event")
            .field("Window", (void *) value.Window)
            ->field("Button", le::mouse_button_name_from_code(value.Button))
            ->field("Modifiers", mods)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->finish();
    }
};

template <>
struct fmt::formatter<le::mouse_button_released_event> {
    void format(const le::mouse_button_released_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n\t\tShift = {},\n\t\tControl = {},\n\t\tAlt = {},\n\t\tSuper = {},\n}}",
                    (bool) (value.Modifiers & le::Modifier_Shift), (bool) (value.Modifiers & le::Modifier_Control),
                    (bool) (value.Modifiers & le::Modifier_Alt), (bool) (value.Modifiers & le::Modifier_Super));

        f->debug_struct("mouse_button_released_event")
            .field("Window", (void *) value.Window)
            ->field("Button", le::mouse_button_name_from_code(value.Button))
            ->field("Modifiers", mods)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->finish();
    }
};

template <>
struct fmt::formatter<le::mouse_scrolled_event> {
    void format(const le::mouse_scrolled_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n\t\tShift = {},\n\t\tControl = {},\n\t\tAlt = {},\n\t\tSuper = {},\n}}",
                    (bool) (value.Modifiers & le::Modifier_Shift), (bool) (value.Modifiers & le::Modifier_Control),
                    (bool) (value.Modifiers & le::Modifier_Alt), (bool) (value.Modifiers & le::Modifier_Super));

        string buttonsDown;
        fmt::sprint(&mods, "{{\n\t\tLeft = {},\n\t\tMiddle = {},\n\t\tRight = {},\n\t\tX1 = {},\n\t\tX2 = {},\n}}",
                    (bool) (value.ButtonsDown & le::Mouse_Button_Left),
                    (bool) (value.ButtonsDown & le::Mouse_Button_Middle),
                    (bool) (value.ButtonsDown & le::Mouse_Button_Right),
                    (bool) (value.ButtonsDown & le::Mouse_Button_X1), (bool) (value.ButtonsDown & le::Mouse_Button_X2));

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
struct fmt::formatter<le::mouse_entered_event> {
    void format(const le::mouse_entered_event &value, format_context *f) const {
        f->debug_struct("mouse_entered_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<le::mouse_left_event> {
    void format(const le::mouse_left_event &value, format_context *f) const {
        f->debug_struct("mouse_left_event").field("Window", (void *) value.Window)->finish();
    }
};

template <>
struct fmt::formatter<le::mouse_moved_event> {
    void format(const le::mouse_moved_event &value, format_context *f) const {
        string mods;
        fmt::sprint(&mods, "{{\n\t\tShift = {},\n\t\tControl = {},\n\t\tAlt = {},\n\t\tSuper = {},\n}}",
                    (bool) (value.Modifiers & le::Modifier_Shift), (bool) (value.Modifiers & le::Modifier_Control),
                    (bool) (value.Modifiers & le::Modifier_Alt), (bool) (value.Modifiers & le::Modifier_Super));

        string buttonsDown;
        fmt::sprint(&mods, "{{\n\t\tLeft = {},\n\t\tMiddle = {},\n\t\tRight = {},\n\t\tX1 = {},\n\t\tX2 = {},\n}}",
                    (bool) (value.ButtonsDown & le::Mouse_Button_Left),
                    (bool) (value.ButtonsDown & le::Mouse_Button_Middle),
                    (bool) (value.ButtonsDown & le::Mouse_Button_Right),
                    (bool) (value.ButtonsDown & le::Mouse_Button_X1), (bool) (value.ButtonsDown & le::Mouse_Button_X2));

        f->debug_struct("mouse_moved_event")
            .field("Window", (void *) value.Window)
            ->field("Modifiers", mods)
            ->field("ButtonsDown", buttonsDown)
            ->field("MouseX", value.MouseX)
            ->field("MouseY", value.MouseY)
            ->finish();
    }
};