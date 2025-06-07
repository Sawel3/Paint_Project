/**
 * @file FileManager.h
 * @brief File operations for saving and loading BMP images.
 */
#pragma once
#include "framework.h"

 /**
  * @brief Provides static methods for saving and loading BMP files.
  */
class FileManager {
public:
    /**
     * @brief Save the current canvas to a BMP file.
     * @param filename Path to save the file.
     * @param hBitmap Bitmap handle.
     * @param hDC Device context.
     * @param width Canvas width.
     * @param height Canvas height.
     * @throws std::exception on failure.
     */
    static void Save(const std::wstring& filename, HBITMAP hBitmap, HDC hdc, int width, int height);
    
    /**
         * @brief Load a BMP file into the canvas.
         * @param filename Path to load the file from.
         * @param hBitmap Bitmap handle (output).
         * @param hDC Device context.
         * @param width Canvas width (output).
         * @param height Canvas height (output).
         * @throws std::exception on failure.
         */
    static void Load(const std::wstring& filename, HBITMAP& hBitmap, HDC hdc, int& width, int& height);
};
