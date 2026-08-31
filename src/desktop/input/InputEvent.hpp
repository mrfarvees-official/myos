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

struct InputEvent
{
    InputEventType type;
    
    Point mousePosition {};

    MouseButton mouseButton = MouseButton::None;

    int keyCode = 0;

    bool alt = false;
    bool ctrl = false;
    bool shift = false;
};