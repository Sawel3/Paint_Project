#pragma once
#include "framework.h"
#include "IShape.h"
#include <string>

class TextShape : public IShape {
    POINT pos;
    std::wstring text;
    COLORREF color;
    LOGFONT font;
public:
    TextShape(POINT p, const std::wstring& t, COLORREF c, const LOGFONT& f);
    void Draw(HDC hdc) const override;
};
