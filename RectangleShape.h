// RectangleShape.h
#pragma once
#include "framework.h"
#include "Shape.h"

class RectangleShape : public Shape {
public:
    RectangleShape(POINT s, POINT e, COLORREF c, int penSize)
        : Shape(s, e, c, penSize) {}
    void Draw(HDC hdc) const override;
};
