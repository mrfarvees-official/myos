#include "WindowManager.hpp"

#include <algorithm>

#include "../graphics/Renderer.hpp"

Window &WindowManager::createWindow(
    const std::string &title,
    const Rect &bounds
)
{
    const int id =
        m_nextWindowId++;

    auto window =
        std::make_unique<Window>(
            id,
            title,
            bounds
        );

    Window &reference =
        *window;

    m_windows.push_back(
        std::move(window)
    );

    focusWindow(id);

    return reference;
}

void WindowManager::focusWindow(
    int windowId
)
{
    auto it = std::find_if(
        m_windows.begin(),
        m_windows.end(),
        [windowId](
            const std::unique_ptr<Window> &window
        ) {
            return window->id() == windowId;
        }
    );

    if (it == m_windows.end()) {
        return;
    }

    for (auto &window : m_windows) {
        window->setFocused(false);
    }

    (*it)->setFocused(true);

    m_focusedWindowId =
        windowId;

    auto window =
        std::move(*it);

    m_windows.erase(it);

    m_windows.push_back(
        std::move(window)
    );
}

Window *WindowManager::focusedWindow()
{
    for (auto &window : m_windows) {
        if (
            window->id() ==
            m_focusedWindowId
        ) {
            return window.get();
        }
    }

    return nullptr;
}

const Window *WindowManager::focusedWindow() const
{
    for (const auto &window : m_windows) {
        if (
            window->id() ==
            m_focusedWindowId
        ) {
            return window.get();
        }
    }

    return nullptr;
}

bool WindowManager::handleEvent(
    const InputEvent &event
)
{
    // -----------------------------------
    // Keyboard shortcut
    // -----------------------------------

    if (
        event.type ==
            InputEventType::KeyDown &&
        event.alt &&
        event.key == Key::Tab
    ) {
        if (m_windows.size() < 2) {
            return false;
        }

        focusNextWindow();

        return true;
    }

    // -----------------------------------
    // Left mouse button pressed
    // -----------------------------------

    if (
        event.type ==
            InputEventType::MouseButtonDown &&
        event.mouseButton ==
            MouseButton::Left
    ) {
        Window *window =
            windowAt(
                event.mousePosition
            );

        if (!window) {
            return false;
        }

        const int previousFocusedWindowId =
            m_focusedWindowId;

        const int selectedWindowId =
            window->id();

        focusWindow(
            selectedWindowId
        );

        const bool focusChanged =
            previousFocusedWindowId !=
            m_focusedWindowId;

        window =
            focusedWindow();

        if (!window) {
            return focusChanged;
        }

        if (
            window->titleBarContains(
                event.mousePosition
            )
        ) {
            m_draggedWindo =
                window;

            const Rect &bounds =
                window->bounds();

            m_dragOffset = Point {
                event.mousePosition.x -
                    bounds.x,

                event.mousePosition.y -
                    bounds.y
            };
        }

        return focusChanged;
    }

    // -----------------------------------
    // Mouse moving while dragging
    // -----------------------------------

    if (
        event.type ==
            InputEventType::MouseMove &&
        m_draggedWindo != nullptr
    ) {
        const int newX =
            event.mousePosition.x -
            m_dragOffset.x;

        const int newY =
            event.mousePosition.y -
            m_dragOffset.y;

        const Rect &bounds =
            m_draggedWindo->bounds();

        if (
            bounds.x == newX &&
            bounds.y == newY
        ) {
            return false;
        }

        m_draggedWindo->setPosition(
            newX,
            newY
        );

        return true;
    }

    // -----------------------------------
    // Left mouse button released
    // -----------------------------------

    if (
        event.type ==
            InputEventType::MouseButtonUp &&
        event.mouseButton ==
            MouseButton::Left
    ) {
        m_draggedWindo =
            nullptr;

        return false;
    }

    return false;
}

Window *WindowManager::windowAt(
    Point position
)
{
    for (
        auto it = m_windows.rbegin();
        it != m_windows.rend();
        ++it
    ) {
        if (
            (*it)->contains(position)
        ) {
            return it->get();
        }
    }

    return nullptr;
}

const Window *WindowManager::windowAt(
    Point position
) const
{
    for (
        auto it = m_windows.rbegin();
        it != m_windows.rend();
        ++it
    ) {
        if (
            (*it)->contains(position)
        ) {
            return it->get();
        }
    }

    return nullptr;
}

void WindowManager::focusNextWindow()
{
    if (m_windows.size() < 2) {
        return;
    }

    auto currentWindow =
        std::move(
            m_windows.back()
        );

    m_windows.pop_back();

    m_windows.insert(
        m_windows.begin(),
        std::move(currentWindow)
    );

    for (auto &window : m_windows) {
        window->setFocused(false);
    }

    Window *nextWindow =
        m_windows.back().get();

    nextWindow->setFocused(true);

    m_focusedWindowId =
        nextWindow->id();
}

void WindowManager::render(
    myos::graphics::Renderer &renderer
) const
{
    for (const auto &window : m_windows) {
        window->render(renderer);
    }
}