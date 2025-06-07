#include "TextShape.h"

// Constructs a text shape with the specified properties.
TextShape::TextShape(POINT p, const std::wstring& t, COLORREF c, const LOGFONT& f)
    : pos(p), text(t), color(c), font(f) {
}

// Draws the text using the specified device context.
void TextShape::Draw(HDC hdc) const {
    HFONT hFont = CreateFontIndirect(&font);
    HGDIOBJ oldFont = SelectObject(hdc, hFont);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, pos.x, pos.y, text.c_str(), static_cast<int>(text.length()));
    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}
