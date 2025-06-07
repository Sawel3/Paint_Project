#pragma once
#include "Shape.h"

// RectangleShape: Represents a rectangle shape.
// Inherits common properties and behavior from Shape.
class RectangleShape : public Shape {
public:
    // Constructs a rectangle with given coordinates, color, and pen size.
    RectangleShape(POINT s, POINT e, COLORREF c, int penSize)
        : Shape(s, e, c, penSize) {
    }

    // Draws the rectangle onto the given device context.
    void Draw(HDC hdc) const override;
};
