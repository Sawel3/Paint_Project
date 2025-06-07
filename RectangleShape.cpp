/**
 * @file RectangleShape.cpp
 * @brief Implementation of RectangleShape.
 */
#include "RectangleShape.h"

// Draws the rectangle using the specified device context.
void RectangleShape::Draw(HDC hdc) const {
    HPEN hPen = CreatePen(PS_SOLID, penSize, color);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(hdc, start.x, start.y, end.x, end.y);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(hPen);
}
