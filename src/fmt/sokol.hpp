#pragma once

#include <sokol_app.h>

#include "../log.hpp"

// sokol
template <>
struct fmt::formatter<sapp_event_type> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const sapp_event_type obj, FormatContext &ctx) const {
        string_view name = "unknown";
        switch(obj) {
        case SAPP_EVENTTYPE_INVALID: name = "INVALID"; break;
        case SAPP_EVENTTYPE_KEY_DOWN: name = "KEY_DOWN"; break;
        case SAPP_EVENTTYPE_KEY_UP: name = "KEY_UP"; break;
        case SAPP_EVENTTYPE_CHAR: name = "CHAR"; break;
        case SAPP_EVENTTYPE_MOUSE_DOWN: name = "MOUSE_DOWN"; break;
        case SAPP_EVENTTYPE_MOUSE_UP: name = "MOUSE_UP"; break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL: name = "MOUSE_SCROLL"; break;
        case SAPP_EVENTTYPE_MOUSE_MOVE: name = "MOUSE_MOVE"; break;
        case SAPP_EVENTTYPE_MOUSE_ENTER: name = "MOUSE_ENTER"; break;
        case SAPP_EVENTTYPE_MOUSE_LEAVE: name = "MOUSE_LEAVE"; break;
        case SAPP_EVENTTYPE_TOUCHES_BEGAN: name = "TOUCHES_BEGAN"; break;
        case SAPP_EVENTTYPE_TOUCHES_MOVED: name = "TOUCHES_MOVED"; break;
        case SAPP_EVENTTYPE_TOUCHES_ENDED: name = "TOUCHES_ENDED"; break;
        case SAPP_EVENTTYPE_TOUCHES_CANCELLED: name = "TOUCHES_CANCELLED"; break;
        case SAPP_EVENTTYPE_RESIZED: name = "RESIZED"; break;
        case SAPP_EVENTTYPE_ICONIFIED: name = "ICONIFIED"; break;
        case SAPP_EVENTTYPE_RESTORED: name = "RESTORED"; break;
        case SAPP_EVENTTYPE_FOCUSED: name = "FOCUSED"; break;
        case SAPP_EVENTTYPE_UNFOCUSED: name = "UNFOCUSED"; break;
        case SAPP_EVENTTYPE_SUSPENDED: name = "SUSPENDED"; break;
        case SAPP_EVENTTYPE_RESUMED: name = "RESUMED"; break;
        case SAPP_EVENTTYPE_QUIT_REQUESTED: name = "QUIT_REQUESTED"; break;
        case SAPP_EVENTTYPE_CLIPBOARD_PASTED: name = "CLIPBOARD_PASTED"; break;
        case SAPP_EVENTTYPE_FILES_DROPPED: name = "FILES_DROPPED"; break;
        default: break;
        }
        return fmt::format_to(ctx.out(), name);
    }
};

template <>
struct fmt::formatter<sapp_mousebutton> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const sapp_mousebutton obj, FormatContext &ctx) const {
        string_view name = "unknown";
        switch(obj) {
        case SAPP_MOUSEBUTTON_LEFT: name = "LEFT"; break;
        case SAPP_MOUSEBUTTON_RIGHT: name = "RIGHT"; break;
        case SAPP_MOUSEBUTTON_MIDDLE: name = "MIDDLE"; break;
        case SAPP_MOUSEBUTTON_INVALID: name = "INVALID"; break;
        }
        return fmt::format_to(ctx.out(), name);
    }
};

template <>
struct fmt::formatter<sapp_event> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const sapp_event &obj, FormatContext &ctx) const {
        fmt::format_to(ctx.out(), "{}", obj.type);

        switch (obj.type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
        case SAPP_EVENTTYPE_KEY_UP:
            fmt::format_to(ctx.out(), ", key_code {}, repeat {}, mods {:x}",
                    (int)obj.key_code, obj.key_repeat, obj.modifiers);
            break;
        case SAPP_EVENTTYPE_CHAR:
            fmt::format_to(ctx.out(), ", char_code {}, repeat {}, mods {:x}",
                    obj.char_code, obj.key_repeat, obj.modifiers);
            break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        case SAPP_EVENTTYPE_MOUSE_UP:
            fmt::format_to(ctx.out(), ", mouse_button {}, mods {:x}",
                    obj.mouse_button, obj.modifiers);
            break;
        default:
            break;
        }

        return fmt::format_to(ctx.out(), ","
                " mouse @ ({}, {}), mouse delta ({}, {}), scroll ({}, {}),"
                " window ({}, {}), framebuffer ({}, {})",
                obj.mouse_x, obj.mouse_y,
                obj.mouse_dx, obj.mouse_dy,
                obj.scroll_x, obj.scroll_y,
                obj.window_width, obj.window_height,
                obj.framebuffer_width, obj.framebuffer_height);
    }
};
