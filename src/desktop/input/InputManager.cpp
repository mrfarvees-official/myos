#include "InputManager.hpp"

#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>

InputManager::InputManager(
    int screenWidth,
    int screenHeight
)
    : m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
}

InputManager::~InputManager()
{
    if (m_mouseFd >= 0) {
        close(m_mouseFd);
    }

    if (m_keyboardFd >= 0) {
        close(m_keyboardFd);
    }
}

bool InputManager::openMouse(const char* devicePath)
{
    m_mouseFd = open(
        devicePath,
        O_RDONLY | O_NONBLOCK
    );

    return m_mouseFd >= 0;
}

bool InputManager::openKeyboard(const char* devicePath)
{
    m_keyboardFd = open(
        devicePath,
        O_RDONLY | O_NONBLOCK
    );

    return m_keyboardFd >= 0;
}

bool InputManager::pollEvent(InputEvent &event)
{
    struct input_event linuxEvent;
    // ---------------------------
    // Mouse Events
    // ---------------------------
    if (m_mouseFd >= 0) {
        ssize_t bytesRead = read(
            m_mouseFd,
            &linuxEvent,
            sizeof(linuxEvent)
        );

        if (bytesRead == sizeof(linuxEvent)) {
            if (linuxEvent.type == EV_REL) {
                if (linuxEvent.code == REL_X) {
                    m_mousePosition.x += linuxEvent.value;
                }

                if (linuxEvent.code == REL_Y) {
                    m_mousePosition.y += linuxEvent.value;
                }

                m_mousePosition.x = std::clamp(
                    m_mousePosition.x,
                    0,
                    m_screenWidth - 1
                );

                m_mousePosition.y = std::clamp(
                    m_mousePosition.y,
                    0,
                    m_screenHeight - 1
                );

                event = InputEvent{};
                event.type = InputEventType::MouseMove;
                event.mousePosition = m_mousePosition;

                return true;
            }

            if (linuxEvent.type == EV_KEY) {
                if (linuxEvent.code == BTN_LEFT) {
                    event = InputEvent{};
                    event.mousePosition = m_mousePosition;
                    event.mouseButton = MouseButton::Left;

                    if (linuxEvent.value == 1) {
                        event.type = InputEventType::MouseButtonDown;
                        return true;
                    }

                    if (linuxEvent.value == 0) {
                        event.type = InputEventType::MouseButtonUp;
                        return true;
                    }
                }

                if (linuxEvent.code == BTN_RIGHT) {
                    event = InputEvent{};
                    event.mousePosition = m_mousePosition;
                    event.mouseButton = MouseButton::Right;

                    if (linuxEvent.value == 1) {
                        event.type = InputEventType::MouseButtonDown;
                        return true;
                    }

                    if (linuxEvent.value == 0) {
                        event.type = InputEventType::MouseButtonUp;
                        return true;
                    }
                }

                if (linuxEvent.code == BTN_BACK) {
                    event = InputEvent{};
                    event.mousePosition = m_mousePosition;
                    event.mouseButton = MouseButton::Back;

                    if (linuxEvent.value == 1) {
                        event.type = InputEventType::MouseButtonDown;
                        return true;
                    }

                    if (linuxEvent.value == 0) {
                        event.type = InputEventType::MouseButtonUp;
                        return true;
                    }
                }
                
                if (linuxEvent.code == BTN_FORWARD) {
                    event = InputEvent{};
                    event.mousePosition = m_mousePosition;
                    event.mouseButton = MouseButton::Forward;

                    if (linuxEvent.value == 1) {
                        event.type = InputEventType::MouseButtonDown;
                        return true;
                    }

                    if (linuxEvent.value == 0) {
                        event.type = InputEventType::MouseButtonUp;
                        return true;
                    }
                }
            }
        }
    }
    // ---------------------------
    // Keyboard Events
    // ---------------------------
    if (m_keyboardFd >= 0) {
        ssize_t bytesRead = read(
            m_keyboardFd,
            &linuxEvent,
            sizeof(linuxEvent)
        );

        if (bytesRead == sizeof(linuxEvent)) {
            if (linuxEvent.type == EV_KEY) {
                // Track Alt State
                if (linuxEvent.code == KEY_LEFTALT || linuxEvent.code == KEY_RIGHTALT) {
                    if (linuxEvent.value == 1 ) {
                        m_altDown = true;
                    }

                    if (linuxEvent.value == 0) {
                        m_altDown = false;
                    }
                }

                event = InputEvent{};
                event.keyCode = linuxEvent.code;
                event.alt = m_altDown;

                if (linuxEvent.value == 1) {
                    event.type = InputEventType::KeyDown;
                    return true;
                }

                if (linuxEvent.value == 0) {
                    event.type = InputEventType::KeyUp;
                    return true;
                }
            }
        }
    }

    return false;
}