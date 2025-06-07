/**
 * @file TextShape.h
 * @brief Text label shape implementation.
 */
#pragma once
#include "IShape.h"

 /**
  * @brief Represents a text label drawn on the canvas.
  */
class TextShape : public IShape {
    POINT pos;           ///< Position of the text.
    std::wstring text;   ///< Text content.
    COLORREF color;      ///< Text color.
    LOGFONT font;        ///< Font used for the text.

public:
    /**
     * @brief Construct a text shape with position, content, color, and font.
     * @param p Position.
     * @param t Text content.
     * @param c Text color.
     * @param f Font.
     */
    TextShape(POINT p, const std::wstring& t, COLORREF c, const LOGFONT& f);

    /**
     * @brief Draw the text onto the given device context.
     * @param hdc The device context to draw on.
     */
    void Draw(HDC hdc) const override;
};
