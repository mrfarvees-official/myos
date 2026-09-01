#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Window.hpp"
#include "../input/InputEvent.hpp"

namespace myos::graphics {
    class Renderer;
}

class WindowManager
{
public:
    Window &createWindow(
        const std::string &title,
        const Rect &bounds
    );

    void focusWindow(int windowId);

    Window *focusedWindow();

    const Window *focusedWindow() const;

    bool handleEvent(
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
    std::vector<std::unique_ptr<Window>> m_windows;

    int m_nextWindowId = 1;
    int m_focusedWindowId = -1;

    Window *m_draggedWindo = nullptr;

    Point m_dragOffset {};
};