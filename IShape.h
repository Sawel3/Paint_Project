#pragma once

// Interface for drawable shapes
class IShape {
public:
    virtual void Draw(HDC hdc) const = 0;
    virtual ~IShape() {}
};
