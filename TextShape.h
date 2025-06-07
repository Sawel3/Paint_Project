#pragma once
#include "IShape.h"

// TextShape: Represents a text label drawn on the canvas.
// Stores position, text content, color, and font.
class TextShape : public IShape {
    POINT pos;             // Position of the text
    std::wstring text;     // Text content
    COLORREF color;        // Text color
    LOGFONT font;          // Font settings

public:
    // Constructs a text shape with position, content, color, and font.
    TextShape(POINT p, const std::wstring& t, COLORREF c, const LOGFONT& f);

    // Draws the text onto the given device context.
    void Draw(HDC hdc) const override;
};
