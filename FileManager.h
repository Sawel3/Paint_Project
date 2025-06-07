#pragma once
#include "framework.h"

// Handles file operations and errors
class FileManager {
public:
    static void Save(const std::wstring& filename, HBITMAP hBitmap, HDC hdc, int width, int height);
    static void Load(const std::wstring& filename, HBITMAP& hBitmap, HDC hdc, int& width, int& height);
};