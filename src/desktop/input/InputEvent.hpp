#pragma once

#include "../geometry/Point.hpp"

enum class InputEventType
{
    MouseMove,
    MouseButtonDown,
    MouseButtonUp,
    KeyDown,
    KeyUp
};

enum class MouseButton 
{
    None,
    Left,
    Right,
    Middle,
    Back,
    Forward
};

enum class Key 
{
    Unknown,

    Tab,
    Enter,
    Escape,
    Space,

    Left,
    Right,
    Up,
    Down,

    LeftAlt,
    RightAlt,

    LeftCtrl,
    RightCtrl,

    LeftShift,
    RightShift
};

struct InputEvent
{
    InputEventType type;
    
    Point mousePosition {};

    MouseButton mouseButton = MouseButton::None;

    Key key = Key::Unknown;

    bool alt = false;
    bool ctrl = false;
    bool shift = false;
};