#pragma once
#include "Shape.h"

// CircleShape: Represents an ellipse (circle/oval) shape.
// Inherits common properties and behavior from Shape.
class CircleShape : public Shape {
public:
    // Constructs a circle/ellipse with given coordinates, color, and pen size.
    CircleShape(POINT s, POINT e, COLORREF c, int penSize)
        : Shape(s, e, c, penSize) {
    }

    // Draws the ellipse onto the given device context.
    void Draw(HDC hdc) const override;
};
