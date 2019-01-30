#pragma once

// This file provides fmt::Formatter specializations for all events in the engine.

#include <lstd/fmt.hpp>

#include "events.hpp"

template <>
struct fmt::Formatter<le::Window_Closed_Event> {
    void format(const le::Window_Closed_Event &value, Format_Context &f) {
        f.write_fmt("Window_Closed_Event {{ WindowPtr = {} }", (void *) value.WindowPtr);
    }
};

template <>
struct fmt::Formatter<le::Window_Resized_Event> {
    void format(const le::Window_Resized_Event &value, Format_Context &f) {
        f.write_fmt("Window_Resized_Event {{ WindowPtr = {}, Width = {}, Height = {} }", (void *) value.WindowPtr,
                    value.Width, value.Height);
    }
};

template <>
struct fmt::Formatter<le::Window_Gained_Focus_Event> {
    void format(const le::Window_Gained_Focus_Event &value, Format_Context &f) {
        f.write_fmt("Window_Gained_Focus_Event {{ WindowPtr = {} }", (void *) value.WindowPtr);
    }
};

template <>
struct fmt::Formatter<le::Window_Lost_Focus_Event> {
    void format(const le::Window_Lost_Focus_Event &value, Format_Context &f) {
        f.write_fmt("Window_Lost_Focus_Event {{ WindowPtr = {} }", (void *) value.WindowPtr);
    }
};

template <>
struct fmt::Formatter<le::Window_Moved_Event> {
    void format(const le::Window_Moved_Event &value, Format_Context &f) {
        f.write_fmt("Window_Moved_Event {{ WindowPtr = {}, Left = {}, Top = {} }", (void *) value.WindowPtr, value.Left,
                    value.Top);
    }
};

template <>
struct fmt::Formatter<le::Key_Pressed_Event> {
    void format(const le::Key_Pressed_Event &value, Format_Context &f) {
        f.write_fmt("Key_Pressed_Event {{\n\tWindowPtr = {}, KeyCode = {},\n\tModifiers = {{\n",
                    (void *) value.WindowPtr, le::key_name_from_code(value.KeyCode));
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write_fmt("\t},\n\tRepeat = {}\n}\n", value.Repeat);
    }
};

template <>
struct fmt::Formatter<le::Key_Released_Event> {
    void format(const le::Key_Released_Event &value, Format_Context &f) {
        f.write_fmt("Key_Released_Event {{\n\tWindowPtr = {}, KeyCode = {},\n\tModifiers = {{\n",
                    (void *) value.WindowPtr, le::key_name_from_code(value.KeyCode));
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write("\t}\n}\n");
    }
};

template <>
struct fmt::Formatter<le::Key_Typed_Event> {
    void format(const le::Key_Typed_Event &value, Format_Context &f) {
        f.write_fmt("Key_Typed_Event {{ WindowPtr = {}, CodePoint: {} }", (void *) value.WindowPtr, value.CodePoint);
    }
};

template <>
struct fmt::Formatter<le::Mouse_Button_Pressed_Event> {
    void format(const le::Mouse_Button_Pressed_Event &value, Format_Context &f) {
        f.write_fmt("Mouse_Button_Pressed_Event {{\n\tWindowPtr = {}, Button = {},\n\tModifiers = {{\n",
                    (void *) value.WindowPtr, le::mouse_button_name_from_code(value.Button));
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write_fmt("\t},\n\tMouseX = {}, MouseY = {}\n}\n", value.MouseX, value.MouseY);
    }
};

template <>
struct fmt::Formatter<le::Mouse_Button_Released_Event> {
    void format(const le::Mouse_Button_Released_Event &value, Format_Context &f) {
        f.write_fmt("Mouse_Button_Released_Event {{\n\tWindowPtr = {}, Button = {},\n\tModifiers = {{\n",
                    (void *) value.WindowPtr, le::mouse_button_name_from_code(value.Button));
        f.write_fmt("\t\tShift = {},\n", (bool) (value.Modifiers & le::Modifier_Shift));
        f.write_fmt("\t\tControl = {},\n", (bool) (value.Modifiers & le::Modifier_Control));
        f.write_fmt("\t\tAlt = {},\n", (bool) (value.Modifiers & le::Modifier_Alt));
        f.write_fmt("\t\tSuper = {},\n", (bool) (value.Modifiers & le::Modifier_Super));
        f.write_fmt("\t},\n\tMouseX = {}, MouseY = {}\n}\n", value.MouseX, value.MouseY);
    }
};

template <>
struct fmt::Formatter<le::Mouse_Scrolled_Event> {
    void format(const le::Mouse_Scrolled_Event &value, Format_Context &f) {
        f.write_fmt("Mouse_Scrolled_Event {{\n\tWindowPtr = {}, DeltaX = {}, DeltaY = {}, \n\tModifiers = {{\n",
                    (void *) value.WindowPtr, value.DeltaX, value.DeltaY);
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
struct fmt::Formatter<le::Mouse_Entered_Event> {
    void format(const le::Mouse_Entered_Event &value, Format_Context &f) {
        f.write_fmt("Mouse_Entered_Event {{ WindowPtr = {} }", (void *) value.WindowPtr);
    }
};

template <>
struct fmt::Formatter<le::Mouse_Left_Event> {
    void format(const le::Mouse_Left_Event &value, Format_Context &f) {
        f.write_fmt("Mouse_Left_Event {{ WindowPtr = {} }", (void *) value.WindowPtr);
    }
};

template <>
struct fmt::Formatter<le::Mouse_Moved_Event> {
    void format(const le::Mouse_Moved_Event &value, Format_Context &f) {
        f.write_fmt("Mouse_Moved_Event {{\n\tWindowPtr = {}, \n\tModifiers = {{\n", (void *) value.WindowPtr);
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