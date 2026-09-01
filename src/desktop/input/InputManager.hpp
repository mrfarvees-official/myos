#pragma once

#include "InputEvent.hpp"
#include "Mouse.hpp"

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

class InputManager
{
public:
    explicit InputManager(Mouse& mouse);

    ~InputManager();

    bool openMouse(const char* devicePath);
    bool openKeyboard(const char* devicePath);

    bool start();
    void stop();

    bool popEvent(InputEvent& event);

private:
    void workerLoop();

    void handleMouseEvent(
        unsigned short type,
        unsigned short code,
        int value
    );

    void handleKeyboardEvent(
        unsigned short type,
        unsigned short code,
        int value
    );

    void pushEvent(const InputEvent& event);

    Key translateKey(
        unsigned short linuxKeyCode
    );

private:
    Mouse& m_mouse;

    int m_mouseFd = -1;
    int m_keyboardFd = -1;

    bool m_altDown = false;
    bool m_ctrlDown = false;
    bool m_shiftDown = false;

    std::atomic<bool> m_running { false };

    std::thread m_workerThread;

    std::queue<InputEvent> m_eventQueue;
    std::mutex m_queueMutex;
};