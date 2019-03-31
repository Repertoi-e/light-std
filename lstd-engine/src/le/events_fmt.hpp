#pragma once

// This file provides fmt::Formatter specializations for all events in the engine.

#include <lstd/fmt.hpp>

#include "events.hpp"

template <>
struct fmt::formatter<le::window_closed_event> {
    void format(const le::window_closed_event &value, format_context &f) const {
        f.write_fmt("window_closed_event {{ Window = {} }", (void *) value.Window);
    }
};

template <>
struct fmt::formatter<le::window_resized_event> {
    void format(const le::window_resized_event &value, format_context &f) const {
        f.write_fmt("window_resized_event {{ Window = {}, Width = {}, Height = {} }", (void *) value.Window,
                    value.Width, value.Height);
    }
};

template <>
struct fmt::formatter<le::window_gained_focus_event> {
    void format(const le::window_gained_focus_event &value, format_context &f) const {
        f.write_fmt("window_gained_Focus_event {{ Window = {} }", (void *) value.Window);
    }
};

template <>
struct fmt::formatter<le::window_lost_focus_event> {
    void format(const le::window_lost_focus_event &value, format_context &f) const {
        f.write_fmt("window_lost_focus_event {{ Window = {} }", (void *) value.Window);
    }
};

template <>
struct fmt::formatter<le::window_moved_event> {
    void format(const le::window_moved_event &value, format_context &f) const {
        f.write_fmt("window_moved_event {{ Window = {}, Left = {}, Top = {} }", (void *) value.Window, value.Left,
                    value.Top);
    }
};

template <>
struct fmt::formatter<le::key_pressed_event> {
    void format(const le::key_pressed_event &value, format_context &f) const {
        f.write_fmt("key_pressed_event {{\n\tWindowPtr = {}, KeyCode = {},\n\tModifiers = {{\n", (void *) value.Window,
                    le::key_name_from_code(value.KeyCode));
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write_fmt("\t},\n\tRepeat = {}\n}\n", value.Repeat);
    }
};

template <>
struct fmt::formatter<le::key_released_event> {
    void format(const le::key_released_event &value, format_context &f) const {
        f.write_fmt("key_released_event {{\n\tWindowPtr = {}, KeyCode = {},\n\tModifiers = {{\n", (void *) value.Window,
                    le::key_name_from_code(value.KeyCode));
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write("\t}\n}\n");
    }
};

template <>
struct fmt::formatter<le::key_typed_event> {
    void format(const le::key_typed_event &value, format_context &f) const {
        f.write_fmt("key_typed_event {{ Window = {}, CodePoint: {} }", (void *) value.Window, value.CodePoint);
    }
};

template <>
struct fmt::formatter<le::mouse_button_pressed_event> {
    void format(const le::mouse_button_pressed_event &value, format_context &f) const {
        f.write_fmt("mouse_button_pressed_event {{\n\tWindowPtr = {}, Button = {},\n\tModifiers = {{\n",
                    (void *) value.Window, le::mouse_button_name_from_code(value.Button));
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write_fmt("\t},\n\tMouseX = {}, MouseY = {}\n}\n", value.MouseX, value.MouseY);
    }
};

template <>
struct fmt::formatter<le::mouse_button_released_event> {
    void format(const le::mouse_button_released_event &value, format_context &f) const {
        f.write_fmt("mouse_button_released_event {{\n\tWindowPtr = {}, Button = {},\n\tModifiers = {{\n",
                    (void *) value.Window, le::mouse_button_name_from_code(value.Button));
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write_fmt("\t},\n\tMouseX = {}, MouseY = {}\n}\n", value.MouseX, value.MouseY);
    }
};

template <>
struct fmt::formatter<le::mouse_scrolled_event> {
    void format(const le::mouse_scrolled_event &value, format_context &f) const {
        f.write_fmt("Mouse_scrolled_event {{\n\tWindowPtr = {}, DeltaX = {}, DeltaY = {}, \n\tModifiers = {{\n",
                    (void *) value.Window, value.DeltaX, value.DeltaY);
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write_fmt("\t},\n\tButtonsDown = {{\n");
        f.write_fmt("\t\tLeft = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_Left));
        f.write_fmt("\t\tMiddle = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_Middle));
        f.write_fmt("\t\tRight = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_Right));
        f.write_fmt("\t\tX1 = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_X1));
        f.write_fmt("\t\tX2 = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_X2));
        f.write_fmt("\t},\n\tMouseX = {}, MouseY = {}\n}\n", value.MouseX, value.MouseY);
    }
};

template <>
struct fmt::formatter<le::mouse_entered_event> {
    void format(const le::mouse_entered_event &value, format_context &f) const {
        f.write_fmt("Mouse_Entered_event {{ Window = {} }", (void *) value.Window);
    }
};

template <>
struct fmt::formatter<le::mouse_left_event> {
    void format(const le::mouse_left_event &value, format_context &f) const {
        f.write_fmt("Mouse_Left_event {{ Window = {} }", (void *) value.Window);
    }
};

template <>
struct fmt::formatter<le::mouse_moved_event> {
    void format(const le::mouse_moved_event &value, format_context &f) const {
        f.write_fmt("Mouse_Moved_event {{\n\tWindowPtr = {}, \n\tModifiers = {{\n", (void *) value.Window);
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write_fmt("\t},\n\tButtonsDown = {{\n");
        f.write_fmt("\t\tLeft = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_Left));
        f.write_fmt("\t\tMiddle = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_Middle));
        f.write_fmt("\t\tRight = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_Right));
        f.write_fmt("\t\tX1 = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_X1));
        f.write_fmt("\t\tX2 = {},\n", (bool) (value.ButtonsDown & le::Mouse_Button_X2));
        f.write_fmt("\t},\n\tMouseX = {}, MouseY = {}\n}\n", value.MouseX, value.MouseY);
    }
};