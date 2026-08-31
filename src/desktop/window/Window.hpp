#pragma once

#include <string>

#include "../geometry/Rect.hpp"

namespace myos::graphics {
    class Renderer;
}

class Window
{
    public: 
        Window(
            int id,
            const std::string &title,
            const Rect &bounds
        );

        int id() const;
        
        const std::string  &title() const;
        
        const Rect &bounds() const;
        
        void setBounds(const Rect &bounds);

        void setPosition(int x, int y);

        void moveBy(int dx, int dy);

        bool isFocused() const;

        void setFocused(bool focused);

        Rect titleBarBounds() const;

        bool contains(Point point) const;

        bool titleBarContains(Point point) const;

        void render(myos::graphics::Renderer &renderer) const;

    private:
        int m_id;
        std::string m_title;
        Rect m_bounds;
        bool m_focused = false;
        static constexpr int TitleBarHeight = 28;
};