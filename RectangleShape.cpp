// RectangleShape.cpp
#include "framework.h"
#include "RectangleShape.h"

void RectangleShape::Draw(HDC hdc) const {
    HPEN pen = CreatePen(PS_SOLID, penSize, color);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(hdc, start.x, start.y, end.x, end.y);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
}
