/**
 * @file RectangleShape.h
 * @brief Rectangle shape implementation.
 */

#pragma once
#include "Shape.h"

 /**
  * @brief Represents a rectangle shape.
  */
class RectangleShape : public Shape {
public:
    /**
     * @brief Construct a rectangle with given coordinates, color, and pen size.
     * @param s Start point.
     * @param e End point.
     * @param c Drawing color.
     * @param penSize Pen size.
     */
    RectangleShape(POINT s, POINT e, COLORREF c, int penSize)
        : Shape(s, e, c, penSize) {
    }

    /**
     * @brief Draw the rectangle onto the given device context.
     * @param hdc The device context to draw on.
     */
    void Draw(HDC hdc) const override;
};
