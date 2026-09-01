#pragma once

#include "../geometry/Point.hpp"

#include <mutex>

class Mouse 
{
    public:
        Mouse(
            int screenWidth,
            int screenHeight
        );

        void moveRelative(
            int deltaX,
            int deltaY
        );

        Point position() const;

    private:
        int m_screenWidth = 0;
        int m_screenHeight = 0;
        Point m_position {};
        mutable std::mutex m_mutex;
};
