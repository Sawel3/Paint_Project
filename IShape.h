/**
 * @file IShape.h
 * @brief Abstract interface for all drawable shapes.
 */
#pragma once
#include "framework.h"
 /**
  * @brief Interface for drawable shapes.
  */
class IShape {
public:
    /**
     * @brief Draw the shape onto the given device context.
     * @param hdc The device context to draw on.
     */
    virtual void Draw(HDC hdc) const = 0;

    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     */
    virtual ~IShape() {}
};
