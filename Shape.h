/**
 * @file Shape.h
 * @brief Abstract base class for geometric shapes.
 */
#pragma once
#include "IShape.h"

 /**
  * @brief Base class for geometric shapes, storing common properties.
  */
class Shape : public IShape {
protected:
    POINT start;   ///< Start point of the shape.
    POINT end;     ///< End point of the shape.
    COLORREF color;///< Drawing color.
    int penSize;   ///< Pen size

public:
    /**
     * @brief Construct a shape with given coordinates, color, and pen size.
     * @param s Start point.
     * @param e End point.
     * @param c Drawing color.
     * @param pen Pen size.
     */
    Shape(POINT s, POINT e, COLORREF c, int pen)
        : start(s), end(e), color(c), penSize(pen) {
    }

    /**
     * @brief Virtual destructor for safe inheritance.
     */
    virtual ~Shape() {}
};
