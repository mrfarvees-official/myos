#pragma once

#include "Point.hpp"

struct Rect
{
    int x;
    int y;
    int width;
    int height;

    int right() const
    {
        return x + width;
    }

    int bottom() const
    {
        return y + height;
    }

    bool contains(Point point) const 
    {
        return  point.x >= x &&
                point.x < right() &&
                point.y >= y &&
                point.y < bottom();
    }
};