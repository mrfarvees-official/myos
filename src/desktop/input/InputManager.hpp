#pragma once

#include "InputEvent.hpp"

class InputManager
{
    public:
        InputManager(
            int screenWidth,
            int screenHeight
        );
        ~InputManager();

        bool openMouse(const char* devicePath);
        bool openKeyboard(const char* devicePath);

        bool pollEvent(InputEvent &event);

    private:
        int m_mouseFd = -1;
        int m_keyboardFd = -1;
        int m_screenWidth = 0;
        int m_screenHeight = 0;
        Point m_mousePosition {};
        bool m_altDown = false;
};