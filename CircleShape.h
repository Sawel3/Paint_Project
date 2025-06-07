/**
 * @file CircleShape.h
 * @brief Circle/ellipse shape implementation.
 */
#pragma once
#include "Shape.h"

 /**
  * @brief Represents an ellipse (circle/oval) shape.
  */
class CircleShape : public Shape {
public:
    /**
     * @brief Construct a circle/ellipse with given coordinates, color, and pen size.
     * @param s Start point.
     * @param e End point.
     * @param c Drawing color.
     * @param penSize Pen size.
     */
    CircleShape(POINT s, POINT e, COLORREF c, int penSize)
        : Shape(s, e, c, penSize) {
    }

    /**
     * @brief Draw the ellipse onto the given device context.
     * @param hdc The device context to draw on.
     */
    void Draw(HDC hdc) const override;
};
