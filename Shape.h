// Shape.h
#pragma once
#include "framework.h"
#include "IShape.h"

class Shape : public IShape {
protected:
    POINT start, end;
    COLORREF color;
    int penSize;
public:
    Shape(POINT s, POINT e, COLORREF c, int pen)
        : start(s), end(e), color(c), penSize(pen) {
    }
    virtual ~Shape() {}
};
