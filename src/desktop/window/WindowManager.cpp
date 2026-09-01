#include "WindowManager.hpp"

#include <algorithm>
#include <iostream>
#include "../graphics/Renderer.hpp"

Window &WindowManager::createWindow(
    const std::string &title,
    const Rect &bounds
)
{
    const int id = m_nextWindowId++;

    auto window = std::make_unique<Window>(
        id,
        title,
        bounds
    );

    Window &reference = *window;

    m_windows.push_back(std::move(window));

    focusWindow(id);

    return reference;
}

void WindowManager::focusWindow(int windowId)
{
    auto it = std::find_if(
        m_windows.begin(),
        m_windows.end(),
        [windowId](const std::unique_ptr<Window> &window) {
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

    m_focusedWindowId = windowId;

    auto window = std::move(*it);

    m_windows.erase(it);

    m_windows.push_back(std::move(window));
}

Window *WindowManager::focusedWindow()
{
    for (auto &window : m_windows) {
        if (window->id() == m_focusedWindowId) {
            return window.get();
        }
    }

    return nullptr;
}

const Window *WindowManager::focusedWindow() const {
    for (const auto &window : m_windows) {
        if (window->id() == m_focusedWindowId) {
            return window.get();
        }
    }

    return nullptr;
}

void WindowManager::handleEvent(const InputEvent &event)
{
    // -----------------------------------
    // Keyboard shortcuts
    // -----------------------------------

    if (
        event.type == InputEventType::KeyDown &&
        event.ctrl &&
        event.key == Key::Enter
    ) {
        std::cout << "[wm] Ctrl+Enter detected\n";

        focusNextWindow();
        return;
    }

    // -----------------------------------
    // Left mouse button pressed
    // -----------------------------------

    if (event.type == InputEventType::MouseButtonDown &&
        event.mouseButton == MouseButton::Left)
    {
        Window *window = windowAt(event.mousePosition);

        if (!window) {
            return;
        }

        focusWindow(window->id());

        window = focusedWindow();

        if (!window) {
            return;
        }

        if (window->titleBarContains(event.mousePosition))
        {
            m_draggedWindo = window;

            const Rect &bounds = window->bounds();

            m_dragOffset = Point {
                event.mousePosition.x - bounds.x,
                event.mousePosition.y - bounds.y
            };
        }

        return;
    }

    // -----------------------------------
    // Mouse moving during drag
    // -----------------------------------

    if (event.type == InputEventType::MouseMove &&
        m_draggedWindo != nullptr)
    {
        m_draggedWindo->setPosition(
            event.mousePosition.x - m_dragOffset.x,
            event.mousePosition.y - m_dragOffset.y
        );

        return;
    }

    // -----------------------------------
    // Left mouse released
    // -----------------------------------

    if (event.type == InputEventType::MouseButtonUp &&
        event.mouseButton == MouseButton::Left)
    {
        m_draggedWindo = nullptr;
        return;
    }
}

Window *WindowManager::windowAt(Point position)
{
    for (auto it = m_windows.rbegin(); it != m_windows.rend(); ++it) {
        if ((*it)->contains(position)) {
            return it->get();
        }
    }

    return nullptr;
}

const Window *WindowManager::windowAt(Point position) const
{
    for (auto it = m_windows.rbegin(); it != m_windows.rend(); ++it) {
        if ((*it)->contains(position)) {
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

    // The window at the end is currently on top/focused.
    // Move it to the bottom of the z-order.
    auto currentWindow = std::move(m_windows.back());

    m_windows.pop_back();

    m_windows.insert(
        m_windows.begin(),
        std::move(currentWindow)
    );

    // The new last window becomes focused.
    for (auto& window : m_windows) {
        window->setFocused(false);
    }

    Window* nextWindow = m_windows.back().get();

    nextWindow->setFocused(true);

    m_focusedWindowId = nextWindow->id();
}

void WindowManager::render(myos::graphics::Renderer &renderer) const {
    for (const auto &window : m_windows) {
        window->render(renderer);
    }
}
