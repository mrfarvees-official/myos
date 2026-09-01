#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Window.hpp"
#include "../input/InputEvent.hpp"

class DisplayMetrics;

namespace myos::graphics {
    class Renderer;
}

struct WindowUpdate
{
    bool changed = false;

    std::vector<Rect> dirtyRects;

    void invalidate(
        const Rect &rect
    )
    {
        dirtyRects.push_back(
            rect
        );

        changed = true;
    }
};

class WindowManager
{
public:
    explicit WindowManager(
        const DisplayMetrics &displayMetrics
    );

    Window &createWindow(
        const std::string &title,
        const Rect &bounds
    );

    void focusWindow(
        int windowId
    );

    Window *focusedWindow();

    const Window *focusedWindow() const;

    WindowUpdate handleEvent(
        const InputEvent &event
    );

    Window *windowAt(
        Point position
    );

    const Window *windowAt(
        Point position
    ) const;

    void focusNextWindow();

    void render(
        myos::graphics::Renderer &renderer
    ) const;

private:
    const DisplayMetrics &m_displayMetrics;

    std::vector<std::unique_ptr<Window>> m_windows;

    int m_nextWindowId = 1;
    int m_focusedWindowId = -1;

    Window *m_draggedWindo = nullptr;

    Point m_dragOffset {};
};