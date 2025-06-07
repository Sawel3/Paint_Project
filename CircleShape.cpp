/**
 * @file CircleShape.cpp
 * @brief Implementation of CircleShape.
 */
#include "CircleShape.h"

// Draws the ellipse using the specified device context.
void CircleShape::Draw(HDC hdc) const {
    HPEN hPen = CreatePen(PS_SOLID, penSize, color);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Ellipse(hdc, start.x, start.y, end.x, end.y);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(hPen);
}
