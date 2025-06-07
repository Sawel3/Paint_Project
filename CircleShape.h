#pragma once
#include "framework.h"
#include "Shape.h"

class CircleShape : public Shape {
public:
    CircleShape(POINT s, POINT e, COLORREF c, int penSize)
        : Shape(s, e, c, penSize) {
    }
    void Draw(HDC hdc) const override;
};
