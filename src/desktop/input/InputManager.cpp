#include "InputManager.hpp"

#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>

InputManager::InputManager(Mouse& mouse)
    : m_mouse(mouse)
{
}

InputManager::~InputManager()
{
    stop();

    if (m_mouseFd >= 0) {
        close(m_mouseFd);
        m_mouseFd = -1;
    }

    if (m_keyboardFd >= 0) {
        close(m_keyboardFd);
        m_keyboardFd = -1;
    }
}

bool InputManager::openMouse(const char* devicePath)
{
    if (m_mouseFd >= 0) {
        close(m_mouseFd);
    }

    m_mouseFd = open(
        devicePath,
        O_RDONLY | O_NONBLOCK
    );

    return m_mouseFd >= 0;
}

bool InputManager::openKeyboard(const char* devicePath)
{
    if (m_keyboardFd >= 0) {
        close(m_keyboardFd);
    }

    m_keyboardFd = open(
        devicePath,
        O_RDONLY | O_NONBLOCK
    );

    return m_keyboardFd >= 0;
}

bool InputManager::start()
{
    if (m_running.load()) {
        return true;
    }

    if (
        m_mouseFd < 0 &&
        m_keyboardFd < 0
    ) {
        return false;
    }

    m_running.store(true);

    m_workerThread = std::thread(
        &InputManager::workerLoop,
        this
    );

    return true;
}

void InputManager::stop()
{
    if (!m_running.load()) {
        return;
    }

    m_running.store(false);

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

bool InputManager::popEvent(InputEvent& event)
{
    std::lock_guard<std::mutex> lock(
        m_queueMutex
    );

    if (m_eventQueue.empty()) {
        return false;
    }

    event = m_eventQueue.front();

    m_eventQueue.pop();

    return true;
}

void InputManager::pushEvent(
    const InputEvent& event
)
{
    std::lock_guard<std::mutex> lock(
        m_queueMutex
    );

    m_eventQueue.push(event);
}

void InputManager::workerLoop()
{
    pollfd fds[2] {};

    fds[0].fd = m_keyboardFd;
    fds[0].events = POLLIN;

    fds[1].fd = m_mouseFd;
    fds[1].events = POLLIN;

    while (m_running.load()) {

        int result = poll(
            fds,
            2,
            100
        );

        if (!m_running.load()) {
            break;
        }

        if (result < 0) {

            if (errno == EINTR) {
                continue;
            }

            continue;
        }

        if (result == 0) {
            continue;
        }

        // -------------------------
        // Keyboard
        // -------------------------

        if (
            m_keyboardFd >= 0 &&
            (fds[0].revents & POLLIN)
        ) {
            while (true) {

                input_event linuxEvent {};

                ssize_t bytesRead = read(
                    m_keyboardFd,
                    &linuxEvent,
                    sizeof(linuxEvent)
                );

                if (
                    bytesRead ==
                    sizeof(linuxEvent)
                ) {
                    handleKeyboardEvent(
                        linuxEvent.type,
                        linuxEvent.code,
                        linuxEvent.value
                    );

                    continue;
                }

                if (
                    bytesRead < 0 &&
                    (
                        errno == EAGAIN ||
                        errno == EWOULDBLOCK
                    )
                ) {
                    break;
                }

                break;
            }
        }

        // -------------------------
        // Mouse
        // -------------------------

        if (
            m_mouseFd >= 0 &&
            (fds[1].revents & POLLIN)
        ) {
            while (true) {

                input_event linuxEvent {};

                ssize_t bytesRead = read(
                    m_mouseFd,
                    &linuxEvent,
                    sizeof(linuxEvent)
                );

                if (
                    bytesRead ==
                    sizeof(linuxEvent)
                ) {
                    handleMouseEvent(
                        linuxEvent.type,
                        linuxEvent.code,
                        linuxEvent.value
                    );

                    continue;
                }

                if (
                    bytesRead < 0 &&
                    (
                        errno == EAGAIN ||
                        errno == EWOULDBLOCK
                    )
                ) {
                    break;
                }

                break;
            }
        }
    }
}

void InputManager::handleMouseEvent(
    unsigned short type,
    unsigned short code,
    int value
)
{
    // --------------------------------
    // Accumulate relative movement
    // --------------------------------

    if (type == EV_REL)
    {
        if (code == REL_X)
        {
            m_pendingMouseDeltaX += value;
            m_mouseMoved = true;
        }
        else if (code == REL_Y)
        {
            m_pendingMouseDeltaY += value;
            m_mouseMoved = true;
        }

        return;
    }

    // --------------------------------
    // Finish one Linux mouse report
    // --------------------------------
    if (type == EV_SYN && code == SYN_REPORT)
    {
        if (!m_mouseMoved) {
            return;
        }

        m_mouse.moveRelative(
            m_pendingMouseDeltaX,
            m_pendingMouseDeltaY
        );

        m_pendingMouseDeltaX = 0;
        m_pendingMouseDeltaY = 0;
        m_mouseMoved = false;

        InputEvent event {};

        event.type =
            InputEventType::MouseMove;

        event.mousePosition =
            m_mouse.position();

        pushEvent(event);

        return;
    }

    // -------------------------
    // Mouse Buttons
    // -------------------------

    if (type != EV_KEY) {
        return;
    }

    MouseButton button =
        MouseButton::None;

    switch (code) {

        case BTN_LEFT:
            button = MouseButton::Left;
            break;

        case BTN_RIGHT:
            button = MouseButton::Right;
            break;

        case BTN_MIDDLE:
            button = MouseButton::Middle;
            break;

        case BTN_BACK:
            button = MouseButton::Back;
            break;

        case BTN_FORWARD:
            button = MouseButton::Forward;
            break;

        default:
            return;
    }

    InputEvent event {};

    event.mousePosition =
        m_mouse.position();

    event.mouseButton = button;

    if (value == 1) {

        event.type =
            InputEventType::MouseButtonDown;

        pushEvent(event);
    }
    else if (value == 0) {

        event.type =
            InputEventType::MouseButtonUp;

        pushEvent(event);
    }
}

void InputManager::handleKeyboardEvent(
    unsigned short type,
    unsigned short code,
    int value
)
{
    if (type != EV_KEY) {
        return;
    }

    bool pressed = value == 1;
    bool repeated = value == 2;
    bool released = value == 0;

    if (repeated) {
        return;
    }

    // -------------------------
    // Alt
    // -------------------------

    if (
        code == KEY_LEFTALT ||
        code == KEY_RIGHTALT
    ) {
        if (pressed) {
            m_altDown = true;
        }

        if (released) {
            m_altDown = false;
        }
    }

    // -------------------------
    // Ctrl
    // -------------------------

    if (
        code == KEY_LEFTCTRL ||
        code == KEY_RIGHTCTRL
    ) {
        if (pressed) {
            m_ctrlDown = true;
        }

        if (released) {
            m_ctrlDown = false;
        }
    }

    // -------------------------
    // Shift
    // -------------------------

    if (
        code == KEY_LEFTSHIFT ||
        code == KEY_RIGHTSHIFT
    ) {
        if (pressed) {
            m_shiftDown = true;
        }

        if (released) {
            m_shiftDown = false;
        }
    }

    Key key = translateKey(code);

    if (key == Key::Unknown) {
        return;
    }

    InputEvent event {};

    event.key = key;

    event.alt = m_altDown;
    event.ctrl = m_ctrlDown;
    event.shift = m_shiftDown;

    if (pressed) {

        event.type =
            InputEventType::KeyDown;

        pushEvent(event);
    }
    else if (released) {

        event.type =
            InputEventType::KeyUp;

        pushEvent(event);
    }
}

Key InputManager::translateKey(
    unsigned short linuxKeyCode
)
{
    switch (linuxKeyCode) {

        case KEY_TAB:
            return Key::Tab;

        case KEY_ENTER:
            return Key::Enter;

        case KEY_ESC:
            return Key::Escape;

        case KEY_SPACE:
            return Key::Space;

        case KEY_LEFT:
            return Key::Left;

        case KEY_RIGHT:
            return Key::Right;

        case KEY_UP:
            return Key::Up;

        case KEY_DOWN:
            return Key::Down;

        case KEY_LEFTALT:
            return Key::LeftAlt;

        case KEY_RIGHTALT:
            return Key::RightAlt;

        case KEY_LEFTCTRL:
            return Key::LeftCtrl;

        case KEY_RIGHTCTRL:
            return Key::RightCtrl;

        case KEY_LEFTSHIFT:
            return Key::LeftShift;

        case KEY_RIGHTSHIFT:
            return Key::RightShift;

        default:
            return Key::Unknown;
    }
}