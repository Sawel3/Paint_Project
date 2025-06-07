#pragma once
#include "framework.h"
// IShape: Abstract interface for all drawable shapes.
// Provides a common interface for polymorphic shape management.
class IShape {
public:
    // Draws the shape onto the given device context.
    virtual void Draw(HDC hdc) const = 0;

    // Virtual destructor for safe polymorphic deletion.
    virtual ~IShape() {}
};
