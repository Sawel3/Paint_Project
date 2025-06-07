#include "framework.h"
#include "CircleShape.h"

void CircleShape::Draw(HDC hdc) const {
    HPEN pen = CreatePen(PS_SOLID, penSize, color);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Ellipse(hdc, start.x, start.y, end.x, end.y);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
}
