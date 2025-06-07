#pragma once
#include "framework.h"

// FileManager: Handles file operations for saving and loading bitmaps.
// Provides static methods for saving the current canvas to a BMP file
// and loading a BMP file into the application.
class FileManager {
public:
    // Saves the given HBITMAP to a BMP file.
    // Throws std::runtime_error on failure.
    static void Save(const std::wstring& filename, HBITMAP hBitmap, HDC hdc, int width, int height);

    // Loads a BMP file into an HBITMAP.
    // Updates hBitmap, width, and height. Throws std::runtime_error on failure.
    static void Load(const std::wstring& filename, HBITMAP& hBitmap, HDC hdc, int& width, int& height);
};
