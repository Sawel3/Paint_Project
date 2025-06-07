#include "FileManager.h"

// Helper: Save HBITMAP to BMP file
void FileManager::Save(const std::wstring& filename, HBITMAP hBitmap, HDC hdc, int width, int height) {
    BITMAP bmp;
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp))
        throw std::runtime_error("GetObject failed.");

    BITMAPFILEHEADER bmfHeader = { 0 };
    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    int lineBytes = ((width * 3 + 3) & ~3);
    int imageSize = lineBytes * height;
    std::vector<BYTE> bits(imageSize);

    BITMAPINFO biInfo = { 0 };
    biInfo.bmiHeader = bi;

    // Get the bitmap bits
    if (!GetDIBits(hdc, hBitmap, 0, height, bits.data(), &biInfo, DIB_RGB_COLORS))
        throw std::runtime_error("GetDIBits failed.");

    bmfHeader.bfType = 0x4D42; // 'BM'
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = bmfHeader.bfOffBits + imageSize;

    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        throw std::runtime_error("CreateFile failed.");

    DWORD dwWritten = 0;
    BOOL ok = WriteFile(hFile, &bmfHeader, sizeof(bmfHeader), &dwWritten, NULL)
        && WriteFile(hFile, &bi, sizeof(bi), &dwWritten, NULL)
        && WriteFile(hFile, bits.data(), imageSize, &dwWritten, NULL);
    CloseHandle(hFile);

    if (!ok)
        throw std::runtime_error("WriteFile failed.");
}

// Helper: Load BMP file into HBITMAP
void FileManager::Load(const std::wstring& filename, HBITMAP& hBitmap, HDC hdc, int& width, int& height) {
    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        throw std::runtime_error("CreateFile failed.");

    BITMAPFILEHEADER bmfHeader;
    DWORD dwRead = 0;
    if (!ReadFile(hFile, &bmfHeader, sizeof(bmfHeader), &dwRead, NULL) || bmfHeader.bfType != 0x4D42) {
        CloseHandle(hFile);
        throw std::runtime_error("Invalid BMP file.");
    }

    BITMAPINFOHEADER bi;
    if (!ReadFile(hFile, &bi, sizeof(bi), &dwRead, NULL)) {
        CloseHandle(hFile);
        throw std::runtime_error("ReadFile failed.");
    }

    int imageSize = bi.biSizeImage;
    if (imageSize == 0) {
        int lineBytes = ((bi.biWidth * 3 + 3) & ~3);
        imageSize = lineBytes * abs(bi.biHeight);
    }
    std::vector<BYTE> bits(imageSize);
    SetFilePointer(hFile, bmfHeader.bfOffBits, NULL, FILE_BEGIN);
    if (!ReadFile(hFile, bits.data(), imageSize, &dwRead, NULL)) {
        CloseHandle(hFile);
        throw std::runtime_error("ReadFile failed.");
    }
    CloseHandle(hFile);

    HBITMAP hNewBitmap = CreateCompatibleBitmap(hdc, bi.biWidth, abs(bi.biHeight));
    if (!hNewBitmap)
        throw std::runtime_error("CreateCompatibleBitmap failed.");

    HDC hMemDC = CreateCompatibleDC(hdc);
    HGDIOBJ oldBmp = SelectObject(hMemDC, hNewBitmap);

    BITMAPINFO biInfo = { 0 };
    biInfo.bmiHeader = bi;
    if (!SetDIBits(hMemDC, hNewBitmap, 0, abs(bi.biHeight), bits.data(), &biInfo, DIB_RGB_COLORS)) {
        SelectObject(hMemDC, oldBmp);
        DeleteDC(hMemDC);
        DeleteObject(hNewBitmap);
        throw std::runtime_error("SetDIBits failed.");
    }

    SelectObject(hMemDC, oldBmp);
    DeleteDC(hMemDC);

    if (hBitmap) DeleteObject(hBitmap);
    hBitmap = hNewBitmap;
    width = bi.biWidth;
    height = abs(bi.biHeight);
}
