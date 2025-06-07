#pragma once
#include "IShape.h"

// Shape: Abstract base class for geometric shapes.
// Stores common properties such as start/end points, color, and pen size.
class Shape : public IShape {
protected:
    POINT start, end;      // Start and end points of the shape
    COLORREF color;        // Drawing color
    int penSize;           // Pen thickness

public:
    // Constructs a shape with given coordinates, color, and pen size.
    Shape(POINT s, POINT e, COLORREF c, int pen)
        : start(s), end(e), color(c), penSize(pen) {
    }

    // Virtual destructor for safe inheritance.
    virtual ~Shape() {}
};
